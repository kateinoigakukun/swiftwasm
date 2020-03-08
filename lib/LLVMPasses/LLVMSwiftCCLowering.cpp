//===--- LLVMSwiftCCLowering.cpp ----------------------------------------------===//
//
// This source file is part of the Swift.org open source project
//
// Copyright (c) 2014 - 2017 Apple Inc. and the Swift project authors
// Licensed under Apache License v2.0 with Runtime Library Exception
//
// See https://swift.org/LICENSE.txt for license information
// See https://swift.org/CONTRIBUTORS.txt for the list of Swift project authors
//
//===----------------------------------------------------------------------===//

#define DEBUG_TYPE "swiftcc-lowering"
#include "swift/LLVMPasses/Passes.h"

using namespace llvm;
using namespace swift;
using swift::SwiftCCLowering;

/// Pimpl implementation of SwiftCCLoweringPass.
namespace {

class SwiftCCLoweringImpl {
  /// Was a change made while running the optimization.
  bool Changed;

  /// The function that we are processing.
  Function &F;

public:
  SwiftCCLoweringImpl(Function &InF)
    : Changed(false), F(InF) {}

  // The top level run routine of the pass.
  bool run();
};

} // end anonymous namespace

bool SwiftCCLoweringImpl::run() {
  // intra-BB retain/release merging.
  DenseMap<Value *, LocalState> PtrToLocalStateMap;
  for (BasicBlock &BB : F) {
    for (auto II = BB.begin(), IE = BB.end(); II != IE; ) {
      // Preincrement iterator to avoid iteration issues in the loop.
      Instruction &Inst = *II++;
      auto *CI = cast<CallInst>(&Inst);

      auto Kind = classifyInstruction(Inst);
      switch (Kind) {
      // These instructions should not reach here based on the pass ordering.
      // i.e. LLVMARCOpt -> LLVMContractOpt.
      case RT_RetainN:
      case RT_UnknownObjectRetainN:
      case RT_BridgeRetainN:
      case RT_ReleaseN:
      case RT_UnknownObjectReleaseN:
      case RT_BridgeReleaseN:
        llvm_unreachable("These are only created by LLVMARCContract !");
      // Delete all fix lifetime and end borrow instructions. After llvm-ir they
      // have no use and show up as calls in the final binary.
      case RT_FixLifetime:
      case RT_EndBorrow:
        Inst.eraseFromParent();
        ++NumNoopDeleted;
        continue;
      case RT_Retain: {
        auto *CI = cast<CallInst>(&Inst);
        auto *ArgVal = RC->getSwiftRCIdentityRoot(CI->getArgOperand(0));

        LocalState &LocalEntry = PtrToLocalStateMap[ArgVal];
        LocalEntry.RetainList.push_back(CI);
        continue;
      }
      case RT_UnknownObjectRetain: {
        auto *CI = cast<CallInst>(&Inst);
        auto *ArgVal = RC->getSwiftRCIdentityRoot(CI->getArgOperand(0));

        LocalState &LocalEntry = PtrToLocalStateMap[ArgVal];
        LocalEntry.UnknownObjectRetainList.push_back(CI);
        continue;
      }
      case RT_Release: {
        // Stash any releases that we see.
        auto *CI = cast<CallInst>(&Inst);
        auto *ArgVal = RC->getSwiftRCIdentityRoot(CI->getArgOperand(0));

        LocalState &LocalEntry = PtrToLocalStateMap[ArgVal];
        LocalEntry.ReleaseList.push_back(CI);
        continue;
      }
      case RT_UnknownObjectRelease: {
        // Stash any releases that we see.
        auto *CI = cast<CallInst>(&Inst);
        auto *ArgVal = RC->getSwiftRCIdentityRoot(CI->getArgOperand(0));

        LocalState &LocalEntry = PtrToLocalStateMap[ArgVal];
        LocalEntry.UnknownObjectReleaseList.push_back(CI);
        continue;
      }
      case RT_BridgeRetain: {
        auto *CI = cast<CallInst>(&Inst);
        auto *ArgVal = RC->getSwiftRCIdentityRoot(CI->getArgOperand(0));

        LocalState &LocalEntry = PtrToLocalStateMap[ArgVal];
        LocalEntry.BridgeRetainList.push_back(CI);
        continue;
      }
      case RT_BridgeRelease: {
        auto *CI = cast<CallInst>(&Inst);
        auto *ArgVal = RC->getSwiftRCIdentityRoot(CI->getArgOperand(0));

        LocalState &LocalEntry = PtrToLocalStateMap[ArgVal];
        LocalEntry.BridgeReleaseList.push_back(CI);
        continue;
      }
      case RT_Unknown:
      case RT_AllocObject:
      case RT_NoMemoryAccessed:
      case RT_RetainUnowned:
      case RT_CheckUnowned:
      case RT_ObjCRelease:
      case RT_ObjCRetain:
        break;
      }

      if (Kind != RT_Unknown)
        continue;
      
      performRRNOptimization(PtrToLocalStateMap);
    }

    // Perform the RRNOptimization.
    performRRNOptimization(PtrToLocalStateMap);
    PtrToLocalStateMap.clear();
  }

  return Changed;
}

bool SwiftCCLowering::runOnFunction(Function &F) {
  return SwiftCCLoweringImpl(F).run();
}

char SwiftCCLowering::ID = 0;
INITIALIZE_PASS_BEGIN(SwiftCCLowering,
                      "swiftcc-lowering", "swiftcc lowering",
                      false, false)
INITIALIZE_PASS_END(SwiftCCLowering,
                    "swiftcc-lowering", "swiftcc lowering",
                    false, false)

llvm::FunctionPass *swift::createSwiftCCLoweringPass() {
  initializeSwiftCCLoweringPass(*llvm::PassRegistry::getPassRegistry());
  return new SwiftCCLowering();
}

