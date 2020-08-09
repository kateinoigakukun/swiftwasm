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
  const SILFunction &F;
  std::unique_ptr<FunctionSummary> TheSummary;

  void indexDirectFunctionCall(const SILFunction &Callee);
  void indexIndirectFunctionCall(const SILDeclRef &Callee,
                                 FunctionSummary::Call::Kind Kind);
  void indexInstruction(const SILInstruction *I);

public:
  FunctionSummaryIndexer(const SILFunction &F) : F(F) {}
  void indexFunction();

  std::unique_ptr<FunctionSummary> takeSummary() {
    return std::move(TheSummary);
  }
};

void FunctionSummaryIndexer::indexDirectFunctionCall(
    const SILFunction &Callee) {
  GUID guid = getGUIDFromUniqueName(Callee.getName());
  FunctionSummary::Call call(guid, Callee.getName(), FunctionSummary::Call::Direct);
  TheSummary->addCall(call);
}

void FunctionSummaryIndexer::indexIndirectFunctionCall(
    const SILDeclRef &Callee, FunctionSummary::Call::Kind Kind) {
  StringRef mangledName = Callee.mangle();
  GUID guid = getGUIDFromUniqueName(mangledName);
  FunctionSummary::Call call(guid, mangledName, Kind);
  TheSummary->addCall(call);
}

void FunctionSummaryIndexer::indexInstruction(const SILInstruction *I) {
  // TODO: Handle dynamically replacable function ref inst
  if (auto *FRI = dyn_cast<FunctionRefInst>(I)) {
    SILFunction *callee = FRI->getReferencedFunctionOrNull();
    assert(callee);
    indexDirectFunctionCall(*callee);
    return;
  }

  if (auto *WMI = dyn_cast<WitnessMethodInst>(I)) {
    indexIndirectFunctionCall(WMI->getMember(), FunctionSummary::Call::Witness);
    return;
  }

  if (auto *MI = dyn_cast<MethodInst>(I)) {
    indexIndirectFunctionCall(MI->getMember(), FunctionSummary::Call::VTable);
    return;
  }

  if (auto *KPI = dyn_cast<KeyPathInst>(I)) {
    for (auto &component : KPI->getPattern()->getComponents()) {
      component.visitReferencedFunctionsAndMethods(
          [this](SILFunction *F) {
            assert(F);
            indexDirectFunctionCall(*F);
          },
          [this](SILDeclRef method) {
            auto decl = cast<AbstractFunctionDecl>(method.getDecl());
            if (auto clas = dyn_cast<ClassDecl>(decl->getDeclContext())) {
              indexIndirectFunctionCall(method, FunctionSummary::Call::VTable);
            } else if (isa<ProtocolDecl>(decl->getDeclContext())) {
              indexIndirectFunctionCall(method, FunctionSummary::Call::Witness);
            } else {
              llvm_unreachable(
                  "key path keyed by a non-class, non-protocol method");
            }
          });
    }
  }
}

bool shouldPreserveFunction(const SILFunction &F) {
  if (F.getRepresentation() == SILFunctionTypeRepresentation::ObjCMethod) {
    return true;
  }
  if (F.hasCReferences()) {
    return true;
  }
  return false;
}

void FunctionSummaryIndexer::indexFunction() {
  GUID guid = getGUIDFromUniqueName(F.getName());
  TheSummary = std::make_unique<FunctionSummary>(guid);
  TheSummary->setDebugName(F.getName());
  for (auto &BB : F) {
    for (auto &I : BB) {
      indexInstruction(&I);
    }
  }
  TheSummary->setPreserved(shouldPreserveFunction(F));
}
};

std::unique_ptr<FunctionSummary>
buildFunctionSummaryIndex(SILFunction &F) {
  FunctionSummaryIndexer indexer(F);
  indexer.indexFunction();
  return indexer.takeSummary();
}

void indexWitnessTable(ModuleSummaryIndex &index, SILModule &M) {
  auto FS = std::make_unique<FunctionSummary>(1);
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
        FS->addCall(edge);
      }
    }
  }

  FS->setPreserved(true);
  FS->setDebugName("__external_witnesses_preserved_fs");
  LLVM_DEBUG(llvm::dbgs() << "Summary: Preserved " << FS->calls().size()
                          << " external witnesses\n");
  index.addFunctionSummary(std::move(FS));
}


void indexVTable(ModuleSummaryIndex &index, SILModule &M) {
  auto FS = std::make_unique<FunctionSummary>(2);
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
        FS->addCall(edge);
      }
      auto methodMod = entry.getMethod().getDecl()->getModuleContext();
      auto isExternalMethod = methodMod != M.getSwiftModule();
      if (entry.getKind() == SILVTableEntry::Override && isExternalMethod) {
        GUID guid = getGUIDFromUniqueName(Impl->getName());
        FunctionSummary::Call edge(guid, Impl->getName(),
                                   FunctionSummary::Call::Direct);
        FS->addCall(edge);
      }
      VirtualMethodSlot slot(entry.getMethod(), VirtualMethodSlot::KindTy::VTable);
      index.addImplementation(slot, getGUID(Impl->getName()));
    }
  }

  FS->setPreserved(true);
  FS->setDebugName("__vtable_destructors_and_externals_preserved_fs");
  LLVM_DEBUG(llvm::dbgs() << "Summary: Preserved " << FS->calls().size()
                          << " deallocators\n");
  index.addFunctionSummary(std::move(FS));
}

void indexKeyPathComponent(ModuleSummaryIndex &index, SILModule &M) {
  auto FS = std::make_unique<FunctionSummary>(3);

  for (SILProperty &P : M.getPropertyList()) {
    if (auto component = P.getComponent()) {
      component->visitReferencedFunctionsAndMethods(
        [&](SILFunction *F) {
          auto FS = buildFunctionSummaryIndex(*F);
          LLVM_DEBUG(llvm::dbgs() << "Preserve keypath funcs " << F->getName() << "\n");
          FS->setPreserved(true);
          index.addFunctionSummary(std::move(FS));
        },
        [&](SILDeclRef method) {
          auto decl = cast<AbstractFunctionDecl>(method.getDecl());
          if (auto clas = dyn_cast<ClassDecl>(decl->getDeclContext())) {
            GUID guid = getGUIDFromUniqueName(method.mangle());
            FunctionSummary::Call edge(guid, method.mangle(),
                                       FunctionSummary::Call::VTable);
            FS->addCall(edge);
          } else if (isa<ProtocolDecl>(decl->getDeclContext())) {
            GUID guid = getGUIDFromUniqueName(method.mangle());
            FunctionSummary::Call edge(guid, method.mangle(),
                                       FunctionSummary::Call::Witness);
            FS->addCall(edge);
          } else {
            llvm_unreachable("key path keyed by a non-class, non-protocol method");
          }
        }
      );
    }
  }
  FS->setPreserved(true);
  FS->setDebugName("__keypath_preserved_fs");
  index.addFunctionSummary(std::move(FS));
}

ModuleSummaryIndex modulesummary::buildModuleSummaryIndex(SILModule &M) {
  ModuleSummaryIndex index;

  index.setModuleName(M.getSwiftModule()->getName().str());
  
  // Preserve keypath things temporarily
  indexKeyPathComponent(index, M);

  for (auto &F : M) {
    auto FS = buildFunctionSummaryIndex(F);
    FS->setLive(false);
    index.addFunctionSummary(std::move(FS));
  }

  indexWitnessTable(index, M);
  indexVTable(index, M);
  return index;
}
