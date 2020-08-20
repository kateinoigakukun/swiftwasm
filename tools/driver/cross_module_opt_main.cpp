#define DEBUG_TYPE "lto-cross-module-opt"

#include "swift/AST/DiagnosticsFrontend.h"
#include "swift/Basic/LLVMInitialize.h"
#include "swift/Frontend/Frontend.h"
#include "swift/Frontend/PrintingDiagnosticConsumer.h"
#include "swift/Option/Options.h"
#include "swift/SIL/SILModule.h"
#include "swift/SIL/TypeLowering.h"
#include "swift/Serialization/ModuleSummary.h"
#include "swift/Serialization/Validation.h"
#include "swift/Subsystems.h"
#include "llvm/ADT/ArrayRef.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/Bitstream/BitstreamReader.h"
#include "llvm/Option/ArgList.h"
#include "llvm/Option/Option.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/MemoryBuffer.h"
#include "llvm/Support/Path.h"
#include "llvm/Support/TargetSelect.h"

using namespace llvm::opt;
using namespace swift;
using namespace modulesummary;

static llvm::cl::opt<std::string>
    LTOPrintLiveTrace("lto-print-live-trace", llvm::cl::init(""),
                      llvm::cl::desc("Print liveness trace for the symbol"));

static llvm::cl::list<std::string>
    InputFilenames(llvm::cl::Positional, llvm::cl::desc("[input files...]"));
static llvm::cl::opt<std::string>
    OutputFilename("o", llvm::cl::desc("output filename"));

static llvm::DenseSet<GUID> computePreservedGUIDs(ModuleSummaryIndex *summary) {
  llvm::DenseSet<GUID> Set(1);
  for (auto FI = summary->functions_begin(), FE = summary->functions_end();
       FI != FE; ++FI) {
    auto summary = FI->second.get();
    if (summary->isPreserved()) {
      Set.insert(FI->first);
    }
  }
  return Set;
}

class LivenessTrace {
public:
  enum ReasonTy { Preserved, StaticRef, IndirectRef };
  std::shared_ptr<LivenessTrace> markedBy;
  std::string symbol;
  GUID guid;
  ReasonTy reason;

  LivenessTrace(std::shared_ptr<LivenessTrace> markedBy, GUID guid,
                ReasonTy reason)
      : markedBy(markedBy), guid(guid), reason(reason) {}

  void setName(std::string name) { this->symbol = name; }

  void dump() { dump(llvm::errs()); }
  void dump(llvm::raw_ostream &os) {
    if (!symbol.empty()) {
      os << symbol;
    } else {
      os << "**missing name**"
         << " (" << guid << ")";
    }
    os << "is referenced by:\n";

    auto target = markedBy;
    while (target) {
      os << " - ";
      if (!target->symbol.empty()) {
        os << target->symbol;
      } else {
        os << "**missing name**";
      }
      os << " (" << target->guid << ")";
      os << "\n";
      target = target->markedBy;
    }
  }
};

namespace CallGraph {
struct Node {
  FunctionSummary *FS;
  std::set<GUID> RetainedTypeRefs;
  bool Dirty = true;

  void retain(Node *Parent) {
    auto beforeSize = RetainedTypeRefs.size();
    auto &parentTypeRefs = Parent->RetainedTypeRefs;
    RetainedTypeRefs.insert(parentTypeRefs.begin(), parentTypeRefs.end());
    Dirty |= beforeSize != RetainedTypeRefs.size();
  }
};
struct Work {
  GUID Target;
  std::shared_ptr<LivenessTrace> Trace;
};

void computeDeadSymbols(ModuleSummaryIndex &M);
}; // namespace CallGraph

VFuncSlot createVFuncSlot(FunctionSummary::Call call) {
  VFuncSlot::KindTy slotKind;
  switch (call.getKind()) {
    case FunctionSummary::Call::Witness: {
      slotKind = VFuncSlot::Witness;
      break;
    }
    case FunctionSummary::Call::VTable: {
      slotKind = VFuncSlot::VTable;
      break;
    }
    case FunctionSummary::Call::Direct: {
      llvm_unreachable("Can't get slot for static call");
    }
    case FunctionSummary::Call::kindCount: {
      llvm_unreachable("impossible");
    }
  }
  return VFuncSlot(slotKind, call.getCallee());
}

void markDeadSymbols(ModuleSummaryIndex &M,
                     llvm::DenseSet<GUID> &PreservedGUIDs) {
  using namespace CallGraph;

  std::vector<Node> Nodes;
  Nodes.resize(M.functions_size());
  std::map<GUID, Node *> NodeMap;
  SmallVector<Work, 8> Worklist;

  int idx = 0;
  for (auto FI = M.functions_begin(), FE = M.functions_end(); FI != FE; ++FI) {
    Node *node = &Nodes[idx++];
    node->FS = FI->second.get();
    auto typeRefs = FI->second->typeRefs();
    for (auto ref : typeRefs) {
      node->RetainedTypeRefs.insert(ref.Guid);
    }

    NodeMap[FI->first] = node;
    if (FI->second->isPreserved()) {
      auto trace = std::make_shared<LivenessTrace>(nullptr, FI->first,
                                                   LivenessTrace::Preserved);
      if (!FI->second->getName().empty()) {
        trace->setName(FI->second->getName());
      }
      Worklist.push_back({FI->first, trace});
    }
  }

  unsigned LiveSymbols = 0;

  std::set<std::shared_ptr<LivenessTrace>> dumpTargets;
  while (!Worklist.empty()) {
    Work work = Worklist.pop_back_val();
    auto trace = work.Trace;

    auto targetNodePair = NodeMap.find(work.Target);
    if (targetNodePair == NodeMap.end()) {
      llvm_unreachable("Bad GUID");
    }
    auto targetNode = targetNodePair->second;

    if (!targetNode->Dirty) {
      continue;
    }
    targetNode->Dirty = false;
    if (targetNode->FS->getGUID() == 5323152069524157394) {
      llvm::dbgs() << "[katei debug] Hit breakpoint\n";
    }

    auto &selfTypeRefs = targetNode->RetainedTypeRefs;
    auto FS = targetNode->FS;

    if (!FS->getName().empty()) {
      LLVM_DEBUG(llvm::dbgs() << "Mark " << FS->getName() << " as live\n");
    } else {
      LLVM_DEBUG(llvm::dbgs() << "Mark (" << FS->getGUID() << ") as live\n");
    }
    FS->setLive(true);
    LiveSymbols++;

    auto queueWorklist = [&](std::shared_ptr<LivenessTrace> trace) {
      auto nextNode = NodeMap[trace->guid];
      nextNode->retain(targetNode);
      auto maybeCallee = M.getFunctionSummary(trace->guid);
      if (!maybeCallee) {
        llvm_unreachable("Bad GUID");
      }
      auto Callee = maybeCallee;
      if (!Callee->getName().empty()) {
        trace->setName(Callee->getName());
        if (LTOPrintLiveTrace == Callee->getName()) {
          dumpTargets.insert(trace);
        }
      }
      Worklist.push_back({trace->guid, trace});
    };

    for (auto Call : FS->calls()) {
      switch (Call.getKind()) {
      case FunctionSummary::Call::Direct: {
        queueWorklist(std::make_shared<LivenessTrace>(
            trace, Call.getCallee(), LivenessTrace::StaticRef));
        continue;
      }
      case FunctionSummary::Call::Witness:
      case FunctionSummary::Call::VTable: {
        VFuncSlot slot = createVFuncSlot(Call);
        auto Impls = M.getImplementations(slot);
        for (auto Impl : Impls) {
          if (selfTypeRefs.find(Impl.TypeGuid) == selfTypeRefs.end()) {
            continue;
          }
          queueWorklist(std::make_shared<LivenessTrace>(
              trace, Impl.Guid, LivenessTrace::IndirectRef));
        }
        break;
      }
      case FunctionSummary::Call::kindCount:
        llvm_unreachable("impossible");
      }
    }
  }
  for (auto dumpTarget : dumpTargets) {
    dumpTarget->dump();
  }
}

int cross_module_opt_main(ArrayRef<const char *> Args, const char *Argv0,
                          void *MainAddr) {
  INITIALIZE_LLVM();

  llvm::cl::ParseCommandLineOptions(Args.size(), Args.data(), "Swift LTO\n");

  CompilerInstance Instance;
  PrintingDiagnosticConsumer PDC;
  Instance.addDiagnosticConsumer(&PDC);

  if (InputFilenames.empty()) {
    Instance.getDiags().diagnose(SourceLoc(),
                                 diag::error_mode_requires_an_input_file);
    return 1;
  }

  auto TheSummary = std::make_unique<ModuleSummaryIndex>();

  for (auto Filename : InputFilenames) {
    LLVM_DEBUG(llvm::dbgs() << "Loading module summary " << Filename << "\n");
    auto ErrOrBuf = llvm::MemoryBuffer::getFile(Filename);
    if (!ErrOrBuf) {
      Instance.getDiags().diagnose(
          SourceLoc(), diag::error_no_such_file_or_directory, Filename);
      return 1;
    }

    auto HasErr = swift::modulesummary::loadModuleSummaryIndex(
        ErrOrBuf.get()->getMemBufferRef(), *TheSummary.get());

    if (HasErr)
      llvm::report_fatal_error("Invalid module summary");
  }

  TheSummary->setName("combined");
  
  auto PreservedGUIDs = computePreservedGUIDs(TheSummary.get());
  //  markDeadTypeRef(*TheSummary.get(), PreservedGUIDs);
  markDeadSymbols(*TheSummary.get(), PreservedGUIDs);

  modulesummary::writeModuleSummaryIndex(*TheSummary, Instance.getDiags(),
                                         OutputFilename);
  return 0;
}
