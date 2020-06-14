//===--- CrossModuleDFE.cpp - Swift LTO -----------------------------------===//
//
// This source file is part of the Swift.org open source project
//
// Copyright (c) 2014 - 2020 Apple Inc. and the Swift project authors
// Licensed under Apache License v2.0 with Runtime Library Exception
//
// See https://swift.org/LICENSE.txt for license information
// See https://swift.org/CONTRIBUTORS.txt for the list of Swift project authors
//
//===----------------------------------------------------------------------===//

#define DEBUG_TYPE "cross-module-dfe"

#include "Passes.h"
#include "swift/SILOptimizer/PassManager/Passes.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/Support/Debug.h"

namespace {
using namespace swift;

class CrossModuleDeadFunctionElimination {
  llvm::SmallSetVector<SILFunction *, 16> Worklist;
  llvm::SmallSetVector<StringRef, 32> AliveFunctions;

public:
  void run(std::vector<SILModule *> Modules) {
    for (auto M : Modules) {
      findAliveFunctions(*M);
    }

    for (auto Name : AliveFunctions) {
      LLVM_DEBUG(llvm::dbgs() << Name << " is living\n");
    }

    std::vector<SILFunction *> DeadFunctions;
    for (SILModule *M : Modules) {
      for (SILFunction &F : *M) {
        if (!isAlive(&F)) {
          DeadFunctions.push_back(&F);
        }
      }
    }

    while (!DeadFunctions.empty()) {
      SILFunction *F = DeadFunctions.back();
      DeadFunctions.pop_back();

      LLVM_DEBUG(llvm::dbgs()
                 << "  erase dead function " << F->getName() << "\n");
      auto &M = F->getModule();
      M.eraseFunction(F);
    }
  }

  bool isAnchorFunction(SILFunction &F) {
    if (F.getName() == SWIFT_ENTRY_POINT_FUNCTION)
      return true;
    // TODO: Ensure alive functions that are expected to be exported
    // by linker.

    return false;
  }

  void findAnchors(SILModule &M) {
    for (SILFunction &F : M) {
      if (isAnchorFunction(F)) {
        LLVM_DEBUG(llvm::dbgs()
                   << "  anchor function: " << F.getName() << "\n");
        ensureAlive(&F);
      }
    }
  }
  void findAliveFunctions(SILModule &M) {
    findAnchors(M);

    while (!Worklist.empty()) {
      auto F = Worklist.pop_back_val();
      scanFunction(*F);
    }
  }

  bool isAlive(SILFunction *F) {
    return AliveFunctions.count(F->getName()) != 0;
  }

  void makeAlive(SILFunction *F) {
    AliveFunctions.insert(F->getName());
    Worklist.insert(F);
  }

  void ensureAlive(SILFunction *F) {
    if (!isAlive(F))
      makeAlive(F);
  }

  void scanFunction(SILFunction &F) {
    LLVM_DEBUG(llvm::dbgs() << "    scan function " << F.getName() << '\n');
    for (SILBasicBlock &BB : F) {
      for (SILInstruction &I : BB) {
        if (auto *MI = dyn_cast<MethodInst>(&I)) {
          
        } else if (auto *FRI = dyn_cast<FunctionRefInst>(&I)) {
          ensureAlive(FRI->getInitiallyReferencedFunction());
        } else if (auto *FRI = dyn_cast<DynamicFunctionRefInst>(&I)) {
          ensureAlive(FRI->getInitiallyReferencedFunction());
        } else if (auto *FRI = dyn_cast<PreviousDynamicFunctionRefInst>(&I)) {
          ensureAlive(FRI->getInitiallyReferencedFunction());
        }
      }
    }
  }
};
} // namespace

void swift::lto::performCrossModuleDeadFunctionElimination(
    std::vector<SILModule *> Modules) {
  CrossModuleDeadFunctionElimination().run(Modules);
}
