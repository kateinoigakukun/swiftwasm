//===--- ModuleSummaryFormat.cpp - Read and write module summary files ----===//
//
// This source file is part of the Swift.org open source project
//
// Copyright (c) 2014 - 2018 Apple Inc. and the Swift project authors
// Licensed under Apache License v2.0 with Runtime Library Exception
//
// See https://swift.org/LICENSE.txt for license information
// See https://swift.org/CONTRIBUTORS.txt for the list of Swift project authors
//
//===----------------------------------------------------------------------===//

#include "ModuleSummaryFormat.h"
#include "BCReadingExtras.h"
#include "memory"
#include "swift/AST/FileSystem.h"
#include "llvm/Bitstream/BitstreamReader.h"
#include "llvm/Bitstream/BitstreamWriter.h"
#include "llvm/Support/CommandLine.h"

using namespace swift;
using namespace modulesummary;
using namespace llvm;

static llvm::cl::opt<bool> ModuleSummaryEmbedDebugName(
    "module-summary-embed-debug-name", llvm::cl::init(false),
    llvm::cl::desc("Embed function names for debugging purpose"));

namespace {

static Optional<FunctionSummary::Call::Kind> getEdgeKind(unsigned edgeKind) {
  if (edgeKind < unsigned(FunctionSummary::Call::Kind::kindCount))
    return FunctionSummary::Call::Kind(edgeKind);
  return None;
}

static Optional<VirtualMethodSlot::KindTy> getSlotKind(unsigned kind) {
  if (kind < unsigned(VirtualMethodSlot::KindTy::kindCount))
    return VirtualMethodSlot::KindTy(kind);
  return None;
}

class Serializer {
  SmallVector<char, 0> Buffer;
  BitstreamWriter Out{Buffer};

  /// A reusable buffer for emitting records.
  SmallVector<uint64_t, 64> ScratchRecord;
  std::array<unsigned, 256> AbbrCodes;

  template <typename Layout> void registerRecordAbbr() {
    using AbbrArrayTy = decltype(AbbrCodes);
    static_assert(Layout::Code <= std::tuple_size<AbbrArrayTy>::value,
                  "layout has invalid record code");
    AbbrCodes[Layout::Code] = Layout::emitAbbrev(Out);
  }

  void writeSignature();
  void writeBlockInfoBlock();
  void emitBlockID(unsigned ID, StringRef name,
                   SmallVectorImpl<unsigned char> &nameBuffer);

  void emitRecordID(unsigned ID, StringRef name,
                    SmallVectorImpl<unsigned char> &nameBuffer);
  void emitFunctionSummary(const FunctionSummary &FS);
  void emitVirtualMethodTable(const ModuleSummaryIndex::VirtualFunctionMapTy &T,
                              VirtualMethodSlot::KindTy kind);

public:
  void emitHeader();
  void emitModuleSummary(const ModuleSummaryIndex &index);
  void write(raw_ostream &os);
};

void Serializer::emitBlockID(unsigned ID, StringRef name,
                             SmallVectorImpl<unsigned char> &nameBuffer) {
  SmallVector<unsigned, 1> idBuffer;
  idBuffer.push_back(ID);
  Out.EmitRecord(bitc::BLOCKINFO_CODE_SETBID, idBuffer);

  // Emit the block name if present.
  if (name.empty())
    return;
  nameBuffer.resize(name.size());
  memcpy(nameBuffer.data(), name.data(), name.size());
  Out.EmitRecord(bitc::BLOCKINFO_CODE_BLOCKNAME, nameBuffer);
}

void Serializer::emitRecordID(unsigned ID, StringRef name,
                              SmallVectorImpl<unsigned char> &nameBuffer) {
  assert(ID < 256 && "can't fit record ID in next to name");
  nameBuffer.resize(name.size() + 1);
  nameBuffer[0] = ID;
  memcpy(nameBuffer.data() + 1, name.data(), name.size());
  Out.EmitRecord(bitc::BLOCKINFO_CODE_SETRECORDNAME, nameBuffer);
}

void Serializer::writeSignature() {
  for (auto c : MODULE_SUMMARY_SIGNATURE)
    Out.Emit((unsigned)c, 8);
}

void Serializer::writeBlockInfoBlock() {
  BCBlockRAII restoreBlock(Out, bitc::BLOCKINFO_BLOCK_ID, 2);

  SmallVector<unsigned char, 64> nameBuffer;
#define BLOCK(X) emitBlockID(X##_ID, #X, nameBuffer)
#define BLOCK_RECORD(K, X) emitRecordID(K::X, #X, nameBuffer)

  BLOCK(RECORD_BLOCK);
  BLOCK_RECORD(record_block, MODULE_METADATA);
  BLOCK_RECORD(record_block, FUNC_METADATA);
  BLOCK_RECORD(record_block, CALL_GRAPH_ENTRY);
  BLOCK_RECORD(record_block, METHOD_METADATA);
  BLOCK_RECORD(record_block, METHOD_IMPL_ENTRY);
}

void Serializer::emitHeader() {
  writeSignature();
  writeBlockInfoBlock();
}

void Serializer::emitFunctionSummary(const FunctionSummary &FS) {
    using namespace record_block;
  StringRef debugFuncName =
      ModuleSummaryEmbedDebugName ? FS.getDebugName() : "";
  FunctionMetadataLayout::emitRecord(
      Out, ScratchRecord, AbbrCodes[FunctionMetadataLayout::Code],
      FS.getGUID(), FS.isLive(), FS.isPreserved(), debugFuncName);

  for (auto call : FS.calls()) {
    CallGraphEntryLayout edgeLayout(Out);
    StringRef debugName =
        ModuleSummaryEmbedDebugName ? call.getDebugName() : "";
    CallGraphEntryLayout::emitRecord(
        Out, ScratchRecord, AbbrCodes[CallGraphEntryLayout::Code],
        unsigned(call.getKind()), call.getCallee(), debugName);
  }
}

void Serializer::emitVirtualMethodTable(
    const ModuleSummaryIndex::VirtualFunctionMapTy &T,
    VirtualMethodSlot::KindTy kind) {
    using namespace record_block;
  for (auto &method : T) {
    auto &virtualFuncGUID = method.first;
    auto impls = method.second;

    MethodMetadataLayout::emitRecord(Out, ScratchRecord,
                                     AbbrCodes[MethodMetadataLayout::Code],
                                     unsigned(kind), virtualFuncGUID);

    for (auto impl : impls) {
      MethodImplEntryLayout::emitRecord(Out, ScratchRecord,
                                        AbbrCodes[MethodImplEntryLayout::Code], impl);
    }
  }
}

void Serializer::emitModuleSummary(const ModuleSummaryIndex &index) {
  using namespace record_block;

  BCBlockRAII restoreBlock(Out, RECORD_BLOCK_ID, 8);
  registerRecordAbbr<ModuleMetadataLayout>();
  registerRecordAbbr<FunctionMetadataLayout>();
  registerRecordAbbr<CallGraphEntryLayout>();
  registerRecordAbbr<MethodMetadataLayout>();
  registerRecordAbbr<MethodImplEntryLayout>();
    ModuleMetadataLayout::emitRecord(
      Out, ScratchRecord, AbbrCodes[ModuleMetadataLayout::Code],
      index.getModuleName());
  for (auto FI = index.functions_begin(), FE = index.functions_end(); FI != FE;
       ++FI) {
    emitFunctionSummary(*FI->second);
  }
  emitVirtualMethodTable(index.getWitnessMethods(), VirtualMethodSlot::Witness);
  emitVirtualMethodTable(index.getVTableMethods(), VirtualMethodSlot::VTable);
}

void Serializer::write(raw_ostream &os) {
  os.write(Buffer.data(), Buffer.size());
  os.flush();
}

class Deserializer {
  BitstreamCursor Cursor;
  SmallVector<uint64_t, 64> Scratch;
  StringRef BlobData;

  ModuleSummaryIndex &moduleSummary;

  bool readSignature();
  bool readModuleSummaryMetadata();
  bool readVirtualMethodInfo();
  bool readFunctionSummary();
  bool readSingleModuleSummary();

public:
  Deserializer(MemoryBufferRef inputBuffer, ModuleSummaryIndex &moduleSummary)
      : Cursor{inputBuffer}, moduleSummary(moduleSummary) {}
  bool readModuleSummary();
};

bool Deserializer::readSignature() {
  for (unsigned char byte : MODULE_SUMMARY_SIGNATURE) {
    if (Cursor.AtEndOfStream())
      return true;
    if (auto maybeRead = Cursor.Read(8)) {
      if (maybeRead.get() != byte)
        return true;
    } else {
      return true;
    }
  }
  return false;
}

bool Deserializer::readModuleSummaryMetadata() {
  Expected<BitstreamEntry> maybeEntry = Cursor.advance();
  if (!maybeEntry)
    report_fatal_error("Should have next entry");

  BitstreamEntry entry = maybeEntry.get();

  if (entry.Kind != BitstreamEntry::Record) {
    return true;
  }
  Scratch.clear();
  auto maybeKind = Cursor.readRecord(entry.ID, Scratch, &BlobData);

  if (!maybeKind) {
    consumeError(maybeKind.takeError());
    return true;
  }

  if (maybeKind.get() != record_block::MODULE_METADATA) {
    return true;
  }

  moduleSummary.setModuleName(BlobData.str());

  return false;
}

bool Deserializer::readFunctionSummary() {
  using namespace record_block;

  Expected<BitstreamEntry> maybeNext = Cursor.advance();
  if (!maybeNext)
    report_fatal_error("Should have next entry");

  BitstreamEntry next = maybeNext.get();

  GUID guid;
  std::string Name;
  FunctionSummary *FS;
  std::unique_ptr<FunctionSummary> NewFSOwner;

  while (next.Kind == BitstreamEntry::Record) {
    Scratch.clear();

    auto maybeKind = Cursor.readRecord(next.ID, Scratch, &BlobData);

    if (!maybeKind)
      report_fatal_error("Should have kind");

    switch (maybeKind.get()) {
    case FUNC_METADATA: {
      unsigned isLive, isPreserved;
      FunctionMetadataLayout::readRecord(Scratch, guid, isLive,
                                         isPreserved);
      Name = BlobData.str();
      if (auto summary = moduleSummary.getFunctionSummary(guid)) {
        FS = summary.getValue();
      } else {
        NewFSOwner = std::make_unique<FunctionSummary>(guid);
        FS = NewFSOwner.get();
      }
      if (isLive) {
        FS->setLive(isLive);
      }
      if (isPreserved) {
        FS->setPreserved(isPreserved);
      }
      FS->setDebugName(Name);
      break;
    }
    case CALL_GRAPH_ENTRY: {
      unsigned edgeKindID;
      GUID targetGUID;
      CallGraphEntryLayout::readRecord(Scratch, edgeKindID,
                                      targetGUID);
      auto callKind = getEdgeKind(edgeKindID);
      if (!callKind)
        report_fatal_error("Bad edge kind");
      if (!FS)
        report_fatal_error("Invalid state");

      FunctionSummary::Call call(targetGUID, callKind.getValue(),
                                 BlobData.str());
      FS->addCall(std::move(call));
      break;
    }
    }

    maybeNext = Cursor.advance();
    if (!maybeNext)
      report_fatal_error("Should have next entry");

    next = maybeNext.get();
  }

  if (auto &FS = NewFSOwner) {
    moduleSummary.addFunctionSummary(std::move(FS));
  }
  return false;
}

bool Deserializer::readVirtualMethodInfo() {
  if (Error Err = Cursor.EnterSubBlock(VIRTUAL_METHOD_INFO_ID)) {
    report_fatal_error("Can't enter subblock");
  }

  Expected<BitstreamEntry> maybeNext = Cursor.advance();
  if (!maybeNext)
    report_fatal_error("Should have next entry");

  BitstreamEntry next = maybeNext.get();

  Optional<VirtualMethodSlot> slot;

  while (next.Kind == BitstreamEntry::Record) {
    Scratch.clear();

    auto maybeKind = Cursor.readRecord(next.ID, Scratch, &BlobData);

    if (!maybeKind)
      report_fatal_error("Should have kind");

    switch (maybeKind.get()) {
    case virtual_method_info::METHOD_METADATA: {
      unsigned methodKindID;
      GUID virtualFuncGUID;
      virtual_method_info::MethodMetadataLayout::readRecord(
          Scratch, methodKindID, virtualFuncGUID);

      auto Kind = getSlotKind(methodKindID);
      if (!Kind)
        report_fatal_error("Bad method kind");

      slot = VirtualMethodSlot(Kind.getValue(), virtualFuncGUID);
      break;
    }
    case virtual_method_info::METHOD_IMPL: {
      GUID implGUID;
      virtual_method_info::MethodImplLayout::readRecord(Scratch, implGUID);
      if (!slot)
        report_fatal_error("Slot should be set before impl");
      moduleSummary.addImplementation(slot.getValue(), implGUID);
      break;
    }
    }

    maybeNext = Cursor.advance();
    if (!maybeNext)
      report_fatal_error("Should have next entry");

    next = maybeNext.get();
  }

  return false;
}

bool Deserializer::readSingleModuleSummary() {
  if (readMoleSummaryMetadata()) {
    return true;
  }

  Expected<BitstreamEntry> maybeNext = Cursor.advance();
  if (!maybeNext)
    report_fatal_error("Should have next entry");

  BitstreamEntry next = maybeNext.get();
  while (next.Kind == BitstreamEntry::SubBlock) {
    switch (next.ID) {
    case FUNCTION_SUMMARY_ID: {
      if (readFunctionSummary()) {
        report_fatal_error("Failed to read FS");
      }
      break;
    }
    case VIRTUAL_METHOD_INFO_ID: {
      if (readVirtualMethodInfo()) {
        report_fatal_error("Failed to read virtual method info");
      }
      break;
    }
    }

    maybeNext = Cursor.advance();
    if (!maybeNext) {
      consumeError(maybeNext.takeError());
      return true;
    }
    next = maybeNext.get();
  }
  return false;
}

bool Deserializer::readModuleSummary() {
  if (readSignature()) {
    report_fatal_error("Invalid signature");
  }

  while (!Cursor.AtEndOfStream()) {
    Expected<BitstreamEntry> maybeEntry =
        Cursor.advance(BitstreamCursor::AF_DontPopBlockAtEnd);
    if (!maybeEntry) {
      report_fatal_error("Should have entry");
    }

    auto entry = maybeEntry.get();
    if (entry.Kind != BitstreamEntry::SubBlock)
      break;

    switch (entry.ID) {
    case bitc::BLOCKINFO_BLOCK_ID: {
      if (Cursor.SkipBlock()) {
        return true;
      }
      break;
    }
    case MODULE_SUMMARY_ID: {
      if (readSingleModuleSummary()) {
        return true;
      }
      break;
    }
    case FUNCTION_SUMMARY_ID:
    case VIRTUAL_METHOD_INFO_ID: {
      llvm_unreachable("FUNCTION_SUMMARY and VIRTUAL_METHOD_INFO_ID blocks "
                       "should be handled in "
                       "'readSingleModuleSummary'");
      break;
    }
    }
  }
  return false;
}

}; // namespace

bool modulesummary::emitModuleSummaryIndex(const ModuleSummaryIndex &index,
                                           DiagnosticEngine &diags,
                                           StringRef path) {
  return withOutputFile(diags, path, [&](raw_ostream &out) {
    Serializer serializer;
    serializer.emitHeader();
    serializer.emitModuleSummary(index);
    serializer.write(out);
    return false;
  });
}
bool modulesummary::loadModuleSummaryIndex(MemoryBufferRef inputBuffer,
                                           ModuleSummaryIndex &moduleSummary) {
  Deserializer deserializer(inputBuffer, moduleSummary);
  return deserializer.readModuleSummary();
}
