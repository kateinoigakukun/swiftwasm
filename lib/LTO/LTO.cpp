//===--- LTO.cpp - Swift LTO ----------------------------------------------===//
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

#define DEBUG_TYPE "lto-pipeline"

#include "swift/LTO/LTO.h"
#include "swift/AST/DiagnosticsFrontend.h"
#include "swift/AST/IRGenRequests.h"
#include "swift/ClangImporter/ClangImporter.h"
#include "swift/SIL/SILModule.h"
#include "swift/SIL/SILVisitor.h"
#include "swift/SILOptimizer/PassManager/PassManager.h"
#include "swift/SILOptimizer/PassManager/PassPipeline.h"
#include "swift/SILOptimizer/PassManager/Passes.h"
#include "swift/Serialization/SerializedModuleLoader.h"
#include "swift/Serialization/SerializedSILLoader.h"
#include "llvm/Bitcode/BitcodeWriter.h"
#include "llvm/IR/Module.h"

#include "Passes.h"

namespace swift {

namespace lto {

using namespace llvm;

//class SILReferenceResolver {
//  virtual SILVTable *resolveVTable(ClassDecl *D) = 0;
//  
//};
//
//class CrossModuleLinker : public SILInstructionVisitor<CrossModuleLinker, void> {
//  std::vector<SILModule *> Modules;
//public:
//  CrossModuleLinker(std::vector<SILModule *> Modules) : Modules(Modules) {}
//  
//  
//  void process() {
//    for (auto Module : Modules) {
////      Module->lookUpFunction(<#SILDeclRef fnRef#>)
//    }
//  }
//};

class LTOModule {
private:
  std::unique_ptr<SILModule> SILMod;
  std::unique_ptr<Lowering::TypeConverter> TC;

  LTOModule(const LTOModule &) = delete;

public:
  LTOModule(ModuleDecl &SwiftModule)
      : TC(std::make_unique<Lowering::TypeConverter>(SwiftModule)) {
    SILOptions SILOpts = {};
    SILMod = performASTLowering(&SwiftModule, *TC, SILOpts);
  }

  void performLink() {
    llvm::SmallVector<PassKind, 1> Pass = {PassKind::PerformanceSILLinker};
    auto &opts = SILMod->getOptions();
    auto plan = SILPassPipelinePlan::getPassPipelineForKinds(opts, Pass);
    executePassPipelinePlan(SILMod.get(), plan);
  }

  SILModule *getSILModule() { return SILMod.get(); }

  std::unique_ptr<SILModule> consume() { return std::move(SILMod); }
};

static GeneratedModule generateIR(std::unique_ptr<LTOModule> LTOMod) {
  auto SILMod = LTOMod->consume();
  auto SwiftModule = SILMod->getSwiftModule();
  const PrimarySpecificPaths PSPs;
  const StringRef ModuleName = SwiftModule->getName().str();
  IRGenOptions Opts = {};
  Opts.OutputKind = IRGenOutputKind::Module;

  auto Mod = performIRGeneration(Opts, SwiftModule, std::move(SILMod),
                                 ModuleName, PSPs, ArrayRef<std::string>());
  return Mod;
}

bool LTOPipeline::addModule(std::unique_ptr<MemoryBuffer> Buffer) {
  serialization::ExtendedValidationInfo extendedInfo;
  serialization::ValidationInfo info =
      serialization::validateSerializedAST(Buffer->getBuffer(), &extendedInfo);
  if (info.status != serialization::Status::Valid) {
    Diags.diagnose(SourceLoc(), diag::invalid_serialized_module);
    return true;
  }

  if (!Ctx) {
    Ctx.reset(createASTContext(info, extendedInfo));
  }

  MBL->registerMemoryBuffer(info.name, std::move(Buffer));

  ModuleNames.emplace_back(Ctx->getIdentifier(info.name));
  return false;
}

bool LTOPipeline::emitLLVMModules(GetStreamFn GetStream) {
  IRGenOptions Opts = {};
  Opts.OutputKind = IRGenOutputKind::Module;

  std::vector<std::unique_ptr<LTOModule>> ModuleOwners;
  std::vector<SILModule *> SILModules;
  for (auto &ModuleName : ModuleNames) {
    std::vector<swift::Located<swift::Identifier>> AccessPath;
    AccessPath.emplace_back(ModuleName, SourceLoc());

    auto SwiftModule = Ctx->getModule(AccessPath);
    if (!SwiftModule) {
      Diags.diagnose(SourceLoc(), diag::unable_to_load_serialized_module,
                     ModuleName.get());
      return true;
    }

    auto LTOMod = std::make_unique<LTOModule>(*SwiftModule);
    LTOMod->performLink();

    SILModules.push_back(LTOMod->getSILModule());
    ModuleOwners.push_back(std::move(LTOMod));
  }

  performCrossModuleDeadFunctionElimination(SILModules);

  for (std::unique_ptr<LTOModule> &Module : ModuleOwners) {
    auto IRModule = generateIR(std::move(Module));
    auto LLVMMod = IRModule.getModule();
    if (auto OS = GetStream(LLVMMod->getName())) {
      WriteBitcodeToFile(*LLVMMod, *OS);
    }
  }
  return false;
}

ASTContext *
LTOPipeline::createASTContext(serialization::ValidationInfo info,
                              serialization::ExtendedValidationInfo extInfo) {
  auto Ctx = ASTContext::get(LangOpts, TCOpts, SearchPathOpts, SM, Diags);
  Diags.addConsumer(PrintDiags);
  LangOpts.setTarget(Triple(info.targetTriple));
  SearchPathOpts.SDKPath = extInfo.getSDKPath();

  SearchPathOpts.RuntimeLibraryPaths.insert(
      SearchPathOpts.RuntimeLibraryPaths.end(), RuntimeLibraryPaths.begin(),
      RuntimeLibraryPaths.end());
  SearchPathOpts.RuntimeLibraryImportPaths.insert(
      SearchPathOpts.RuntimeLibraryImportPaths.end(),
      RuntimeLibraryImportPaths.begin(), RuntimeLibraryImportPaths.end());
  SearchPathOpts.RuntimeResourcePath = RuntimeResourcePath;

  // MARK: Setup module loaders
  std::unique_ptr<ClangImporter> clangImporter =
      ClangImporter::create(*Ctx, ClangOpts, "", nullptr);
  auto const &Clang = clangImporter->getClangInstance();
  std::string ModuleCachePath = getModuleCachePathFromClang(Clang);

  auto MIL = ModuleInterfaceLoader::create(*Ctx, ModuleCachePath, "", nullptr,
                                           ModuleLoadingMode::PreferSerialized);
  Ctx->addModuleLoader(std::move(MIL));
  auto MBL = MemoryBufferSerializedModuleLoader::create(
      *Ctx, nullptr, ModuleLoadingMode::OnlySerialized, true);
  this->MBL = MBL.get();

  auto SML = SerializedModuleLoader::create(
      *Ctx, nullptr, ModuleLoadingMode::OnlySerialized, true);

  Ctx->addModuleLoader(std::move(MBL));
  Ctx->addModuleLoader(std::move(SML));
  Ctx->addModuleLoader(std::move(clangImporter), /*isClang*/ true);

  registerIRGenRequestFunctions(Ctx->evaluator);
  registerSILOptimizerRequestFunctions(Ctx->evaluator);
  registerParseRequestFunctions(Ctx->evaluator);
  registerTypeCheckerRequestFunctions(Ctx->evaluator);
  registerSILGenRequestFunctions(Ctx->evaluator);
  registerIRGenSILTransforms(*Ctx);
  return Ctx;
}

} // namespace lto
} // namespace swift
