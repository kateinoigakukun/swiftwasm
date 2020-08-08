//===--- swift-module-summary-test.cpp - Test util for C parser library ---===//
//
// This source file is part of the Swift.org open source project
//
// Copyright (c) 2014 - 2019 Apple Inc. and the Swift project authors
// Licensed under Apache License v2.0 with Runtime Library Exception
//
// See https://swift.org/LICENSE.txt for license information
// See https://swift.org/CONTRIBUTORS.txt for the list of Swift project authors
//
//===----------------------------------------------------------------------===//
//
// Testing utility for the C API of the parser library.
//
//===----------------------------------------------------------------------===//

#include "swift/AST/FileSystem.h"
#include "swift/Basic/LLVM.h"
#include "swift/Basic/LLVMInitialize.h"
#include "swift/Serialization/ModuleSummary.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/MemoryBuffer.h"
#include "llvm/Support/YAMLParser.h"
#include "llvm/Support/YAMLTraits.h"

using namespace swift;
using namespace modulesummary;

enum class ActionType : unsigned {
  None,
  BinaryToYAML,
  YAMLToBinary,
};

namespace options {

static llvm::cl::OptionCategory Category("swift-module-summary-test Options");

static llvm::cl::opt<std::string>
    InputFilename(llvm::cl::Positional, llvm::cl::desc("<input file>"),
                  llvm::cl::init("-"), llvm::cl::value_desc("filename"));

static llvm::cl::opt<std::string>
    OutputFilename("o", llvm::cl::desc("Override output filename"),
                   llvm::cl::value_desc("filename"));

static llvm::cl::opt<ActionType>
    Action(llvm::cl::desc("Mode:"), llvm::cl::init(ActionType::None),
           llvm::cl::cat(Category),
           llvm::cl::values(
               clEnumValN(ActionType::BinaryToYAML, "to-yaml",
                          "Convert new binary .swiftdeps format to YAML"),
               clEnumValN(ActionType::YAMLToBinary, "from-yaml",
                          "Convert YAML to new binary .swiftdeps format")));

} // namespace options

LLVM_YAML_DECLARE_MAPPING_TRAITS(modulesummary::ModuleSummaryIndex)
LLVM_YAML_DECLARE_MAPPING_TRAITS(modulesummary::FunctionSummary)
LLVM_YAML_DECLARE_MAPPING_TRAITS(modulesummary::FunctionSummary::Call)
LLVM_YAML_IS_SEQUENCE_VECTOR(modulesummary::FunctionSummary::Call)
LLVM_YAML_DECLARE_ENUM_TRAITS(modulesummary::FunctionSummary::Call::Kind)

namespace llvm {
namespace yaml {

template <> struct MappingTraits<std::unique_ptr<FunctionSummary>> {
  static void mapping(IO &io, std::unique_ptr<FunctionSummary> &Ptr) {
    if (!Ptr) {
      Ptr.reset(new FunctionSummary());
    }
    MappingTraits<FunctionSummary>::mapping(io, *Ptr.get());
  }
};

void ScalarEnumerationTraits<FunctionSummary::Call::Kind>::enumeration(
    IO &io, FunctionSummary::Call::Kind &V) {
  using Kind = FunctionSummary::Call::Kind;
  io.enumCase(V, "direct", Kind::Direct);
  io.enumCase(V, "vtable", Kind::VTable);
  io.enumCase(V, "witness", Kind::Witness);
}

void MappingTraits<FunctionSummary::Call>::mapping(IO &io,
                                                   FunctionSummary::Call &V) {
  io.mapRequired("callee_name", V.debugName);
  io.mapRequired("callee", V.calleeFn);
  io.mapRequired("kind", V.kind);
}

void MappingTraits<FunctionSummary>::mapping(IO &io, FunctionSummary &V) {
  io.mapRequired("name", V.debugName);
  io.mapRequired("guid", V.guid);
  io.mapRequired("live", V.flags.Live);
  io.mapRequired("preserved", V.flags.Preserved);
  io.mapRequired("calls", V.CallGraphEdgeList);
}

template <>
struct CustomMappingTraits<ModuleSummaryIndex::FunctionSummaryMapTy> {
  static void inputOne(IO &io, StringRef Key,
                       ModuleSummaryIndex::FunctionSummaryMapTy &V) {
    GUID KeyInt;
    if (Key.getAsInteger(0, KeyInt)) {
      io.setError("key not an integer");
      return;
    }
    io.mapRequired(Key.str().c_str(), V[KeyInt]);
  }
  static void output(IO &io, ModuleSummaryIndex::FunctionSummaryMapTy &V) {
    for (auto &P : V)
      io.mapRequired(llvm::utostr(P.first).c_str(), P.second);
  }
};
void MappingTraits<ModuleSummaryIndex>::mapping(IO &io, ModuleSummaryIndex &V) {
  io.mapRequired("module_name", V.ModuleName);
  io.mapRequired("functions", V.FunctionSummaryInfoMap);
}
} // namespace yaml
} // namespace llvm

int main(int argc, char *argv[]) {
  PROGRAM_START(argc, argv);
  llvm::cl::ParseCommandLineOptions(argc, argv, "Swift Module Summary Test\n");

  StringRef fname = options::InputFilename;
  SourceManager sourceMgr;
  DiagnosticEngine diags(sourceMgr);

  auto fileBufOrErr = llvm::MemoryBuffer::getFile(fname);
  if (!fileBufOrErr) {
    llvm::errs() << "error opening file '" << fname
                 << "': " << fileBufOrErr.getError().message();
    return 1;
  }

  switch (options::Action) {
  case ActionType::None: {
    llvm::errs() << "action required\n";
    llvm::cl::PrintHelpMessage();
    return 1;
  }
  case ActionType::BinaryToYAML: {
    modulesummary::ModuleSummaryIndex summary;
    modulesummary::loadModuleSummaryIndex(fileBufOrErr.get()->getMemBufferRef(),
                                          summary);

    bool hadError = withOutputFile(diags, options::OutputFilename,
                                   [&](llvm::raw_pwrite_stream &out) {
                                     out << "# Module-summary v0\n";
                                     llvm::yaml::Output yamlWriter(out);
                                     yamlWriter << summary;
                                     return false;
                                   });

    if (hadError) {
      llvm::errs() << "Failed to write YAML swiftdeps\n";
    }
    break;
  }
  case ActionType::YAMLToBinary:
    ModuleSummaryIndex summary;
    llvm::yaml::Input yamlReader(fileBufOrErr.get()->getMemBufferRef(),
                                 nullptr);
    yamlReader >> summary;
    if (yamlReader.error()) {
      llvm::errs() << "Failed to parse YAML swiftdeps\n";
      return 1;
    }

    if (emitModuleSummaryIndex(summary, diags, options::OutputFilename)) {
      llvm::errs() << "Failed to write binary module summary\n";
      return 1;
    }

    return 1;
  }
  return 0;
}
