#include "swift/Demangling/Demangle.h"
#include "swift/SIL/SILFunction.h"
#include "swift/SIL/SILModule.h"
#include "swift/SILOptimizer/Analysis/BasicCalleeAnalysis.h"
#include "swift/SILOptimizer/Analysis/FunctionOrder.h"
#include "swift/SILOptimizer/PassManager/Transforms.h"
#include "swift/Serialization/ModuleSummary.h"
#include "llvm/Support/MD5.h"
#include "llvm/Support/raw_ostream.h"

#define DEBUG_TYPE "module-summary-index"

using namespace swift;
using namespace modulesummary;

GUID modulesummary::getGUIDFromUniqueName(llvm::StringRef Name) {
  return llvm::MD5Hash(Name);
}
namespace {
class FunctionSummaryIndexer {
  std::vector<FunctionSummary::Call> CallGraphEdgeList;
public:
  void indexInstruction(SILFunction &F, SILInstruction *I);
  void indexFunction(SILFunction &F);

  std::unique_ptr<FunctionSummary> takeSummary() {
    return std::make_unique<FunctionSummary>(std::move(CallGraphEdgeList));
  }
};

void FunctionSummaryIndexer::indexInstruction(SILFunction &F, SILInstruction *I) {
  // TODO: Handle dynamically replacable function ref inst
  if (auto *FRI = dyn_cast<FunctionRefInst>(I)) {
    SILFunction *callee = FRI->getReferencedFunctionOrNull();
    assert(callee);
    GUID guid = getGUIDFromUniqueName(callee->getName());
    FunctionSummary::Call edge(guid, callee->getName(),
                               FunctionSummary::Call::Direct);
    CallGraphEdgeList.push_back(edge);
    return;
  }

  if (auto *WMI = dyn_cast<WitnessMethodInst>(I)) {
    GUID guid = getGUIDFromUniqueName(WMI->getMember().mangle());
    FunctionSummary::Call edge(guid, WMI->getMember().mangle(),
                               FunctionSummary::Call::Witness);
    CallGraphEdgeList.push_back(edge);
    return;
  }

  if (auto *MI = dyn_cast<MethodInst>(I)) {
    GUID guid = getGUIDFromUniqueName(MI->getMember().mangle());
    FunctionSummary::Call edge(guid, MI->getMember().mangle(),
                               FunctionSummary::Call::VTable);
    CallGraphEdgeList.push_back(edge);
    return;
  }

  if (auto *KPI = dyn_cast<KeyPathInst>(I)) {
    for (auto &component : KPI->getPattern()->getComponents()) {
      component.visitReferencedFunctionsAndMethods(
          [this](SILFunction *F) {
            GUID guid = getGUIDFromUniqueName(F->getName());
            FunctionSummary::Call edge(guid, F->getName(),
                                       FunctionSummary::Call::Direct);
            CallGraphEdgeList.push_back(edge);
          },
          [this](SILDeclRef method) {
            auto decl = cast<AbstractFunctionDecl>(method.getDecl());
            if (auto clas = dyn_cast<ClassDecl>(decl->getDeclContext())) {
              GUID guid = getGUIDFromUniqueName(method.mangle());
              FunctionSummary::Call edge(guid, method.mangle(),
                                         FunctionSummary::Call::VTable);
              CallGraphEdgeList.push_back(edge);
            } else if (isa<ProtocolDecl>(decl->getDeclContext())) {
              GUID guid = getGUIDFromUniqueName(method.mangle());
              FunctionSummary::Call edge(guid, method.mangle(),
                                         FunctionSummary::Call::Witness);
              CallGraphEdgeList.push_back(edge);
            } else {
              llvm_unreachable(
                  "key path keyed by a non-class, non-protocol method");
            }
          });
    }
  }
}

void FunctionSummaryIndexer::indexFunction(SILFunction &F) {
  for (auto &BB : F) {
     for (auto &I : BB) {
       indexInstruction(F, &I);
     }
  }
}
};

std::unique_ptr<FunctionSummary>
buildFunctionSummaryIndex(SILFunction &F) {
  FunctionSummaryIndexer indexer;
  indexer.indexFunction(F);
  return indexer.takeSummary();
}

void indexWitnessTable(ModuleSummaryIndex &index, SILModule &M) {
  std::vector<FunctionSummary::Call> Preserved;
  for (auto &WT : M.getWitnessTableList()) {
    auto isExternalProto = WT.getDeclContext()->getParentModule() != M.getSwiftModule() ||
                           WT.getProtocol()->getParentModule() != M.getSwiftModule();
    for (auto entry : WT.getEntries()) {
      if (entry.getKind() != SILWitnessTable::Method) continue;
      
      auto methodWitness = entry.getMethodWitness();
      auto Witness = methodWitness.Witness;
      if (!Witness) continue;
      VirtualMethodSlot slot(methodWitness.Requirement, VirtualMethodSlot::KindTy::Witness);
      index.addImplementation(slot, getGUID(Witness->getName()));
      if (isExternalProto) {
        GUID guid = getGUIDFromUniqueName(Witness->getName());
        FunctionSummary::Call edge(guid, Witness->getName(),
                                   FunctionSummary::Call::Direct);
        Preserved.push_back(edge);
      }
    }
  }
  
  auto FS = std::make_unique<FunctionSummary>(Preserved);
  FS->setPreserved(true);
  LLVM_DEBUG(llvm::dbgs() << "Summary: Preserved " << Preserved.size() << " external witnesses\n");
  index.addFunctionSummary("__external_witnesses_preserved_fs", std::move(FS));
}


void indexVTable(ModuleSummaryIndex &index, SILModule &M) {
  std::vector<FunctionSummary::Call> Preserved;
  for (auto &VT : M.getVTables()) {
    for (auto entry : VT->getEntries()) {
      auto Impl = entry.getImplementation();
      if (entry.getMethod().kind == SILDeclRef::Kind::Deallocator ||
          entry.getMethod().kind == SILDeclRef::Kind::IVarDestroyer) {
        // Destructors are alive because they are called from swift_release
        GUID guid = getGUIDFromUniqueName(Impl->getName());
        FunctionSummary::Call edge(guid, Impl->getName(),
                                   FunctionSummary::Call::Direct);
        LLVM_DEBUG(llvm::dbgs() << "Preserve deallocator '" << Impl->getName() << "'\n");
        Preserved.push_back(edge);
      }
      auto methodMod = entry.getMethod().getDecl()->getModuleContext();
      auto isExternalMethod = methodMod != M.getSwiftModule();
      if (entry.getKind() == SILVTableEntry::Override && isExternalMethod) {
        GUID guid = getGUIDFromUniqueName(Impl->getName());
        FunctionSummary::Call edge(guid, Impl->getName(),
                                   FunctionSummary::Call::Direct);
        Preserved.push_back(edge);
      }
      VirtualMethodSlot slot(entry.getMethod(), VirtualMethodSlot::KindTy::VTable);
      index.addImplementation(slot, getGUID(Impl->getName()));
    }
  }
  
  auto FS = std::make_unique<FunctionSummary>(Preserved);
  FS->setPreserved(true);
  LLVM_DEBUG(llvm::dbgs() << "Summary: Preserved " << Preserved.size() << " deallocators\n");
  index.addFunctionSummary("__vtable_destructors_and_externals_preserved_fs", std::move(FS));
}

void indexKeyPathComponent(ModuleSummaryIndex &index, SILModule &M) {
  std::vector<FunctionSummary::Call> CallGraphEdgeList;
  for (SILProperty &P : M.getPropertyList()) {
    if (auto component = P.getComponent()) {
      component->visitReferencedFunctionsAndMethods(
        [&](SILFunction *F) {
          auto FS = buildFunctionSummaryIndex(*F);
          LLVM_DEBUG(llvm::dbgs() << "Preserve keypath funcs " << F->getName() << "\n");
          FS->setPreserved(true);
          index.addFunctionSummary(F->getName(), std::move(FS));
        },
        [&](SILDeclRef method) {
          auto decl = cast<AbstractFunctionDecl>(method.getDecl());
          if (auto clas = dyn_cast<ClassDecl>(decl->getDeclContext())) {
            GUID guid = getGUIDFromUniqueName(method.mangle());
            FunctionSummary::Call edge(guid, method.mangle(),
                                       FunctionSummary::Call::VTable);
            CallGraphEdgeList.push_back(edge);
          } else if (isa<ProtocolDecl>(decl->getDeclContext())) {
            GUID guid = getGUIDFromUniqueName(method.mangle());
            FunctionSummary::Call edge(guid, method.mangle(),
                                       FunctionSummary::Call::Witness);
            CallGraphEdgeList.push_back(edge);
          } else {
            llvm_unreachable("key path keyed by a non-class, non-protocol method");
          }
        }
      );
    }
  }
  auto FS = std::make_unique<FunctionSummary>(std::move(CallGraphEdgeList));
  FS->setPreserved(true);
  index.addFunctionSummary("__keypath_preserved_fs", std::move(FS));
}

ModuleSummaryIndex modulesummary::buildModuleSummaryIndex(SILModule &M) {
  ModuleSummaryIndex index;

  index.setModuleName(M.getSwiftModule()->getName().str());
  
  // Preserve keypath things temporarily
  indexKeyPathComponent(index, M);

  for (auto &F : M) {
    auto FS = buildFunctionSummaryIndex(F);
    if (F.getRepresentation() == SILFunctionTypeRepresentation::ObjCMethod) {
      LLVM_DEBUG(llvm::dbgs() << "Preserve " << F.getName() << " due to ObjCMethod\n");
      FS->setPreserved(true);
    }
    if (F.hasCReferences()) {
      LLVM_DEBUG(llvm::dbgs() << "Preserve " << F.getName() << " due to @_silgen_name or @_cdecl\n");
      FS->setPreserved(true);
    }

    FS->setLive(false);
    index.addFunctionSummary(F.getName(), std::move(FS));
  }

  indexWitnessTable(index, M);
  indexVTable(index, M);
  return index;
}
