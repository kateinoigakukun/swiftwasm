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
#include "swift/Serialization/ModuleSummary.h"
#include "llvm/Bitstream/BitstreamReader.h"
#include "llvm/Bitstream/BitstreamWriter.h"
#include "llvm/Support/CommandLine.h"

using namespace swift;
using namespace modulesummary;
using namespace llvm;

static cl::opt<bool> ModuleSummaryEmbedDebugName(
    "module-summary-embed-debug-name", cl::init(false),
    cl::desc("Embed function names for debugging purpose"));

namespace {
static Optional<FunctionSummary::Call::KindTy> getEdgeKind(unsigned edgeKind) {
  if (edgeKind < unsigned(FunctionSummary::Call::KindTy::kindCount))
    return FunctionSummary::Call::KindTy(edgeKind);
  return None;
}


static Optional<VFuncSlot::KindTy> getSlotKind(unsigned kind) {
  if (kind < unsigned(FunctionSummary::Call::KindTy::kindCount))
    return VFuncSlot::KindTy(kind);
  return None;
}

class Serializer {
  SmallVector<char, 0> Buffer;
  BitstreamWriter Out{Buffer};

  /// A reusable buffer for emitting records.
  SmallVector<uint64_t, 64> ScratchRecord;

  void writeSignature();
  void writeBlockInfoBlock();
  void emitBlockID(unsigned ID, StringRef name,
                   SmallVectorImpl<unsigned char> &nameBuffer);

  void emitRecordID(unsigned ID, StringRef name,
                    SmallVectorImpl<unsigned char> &nameBuffer);
  void emitVFuncTable(const VFuncToImplsMapTy T, VFuncSlot::KindTy kind);
public:
  void emitHeader();
  void emitModuleSummary(const ModuleSummaryIndex &index);
  void emitFunctionSummary(const FunctionSummary *summary);
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
  BLOCK_RECORD(record_block, CALL_GRAPH_EDGE);

  BLOCK_RECORD(record_block, METHOD_METADATA);
  BLOCK_RECORD(record_block, METHOD_IMPL);
}

void Serializer::emitHeader() {
  writeSignature();
  writeBlockInfoBlock();
}

void Serializer::emitFunctionSummary(const FunctionSummary *summary) {
  using namespace record_block;
  FunctionMetadataLayout MDlayout(Out);
  StringRef debugFuncName =
      ModuleSummaryEmbedDebugName ? summary->getName() : "";
  MDlayout.emit(ScratchRecord, summary->getGUID(),
                unsigned(summary->isLive()),
                unsigned(summary->isPreserved()), debugFuncName);

  for (auto call : summary->calls()) {
    CallGraphEdgeLayout edgeLayout(Out);
    StringRef debugName =
        ModuleSummaryEmbedDebugName ? call.getName() : "";
    edgeLayout.emit(ScratchRecord, unsigned(call.getKind()),
                    call.getCallee(), debugName);
  }
}

void Serializer::emitVFuncTable(const VFuncToImplsMapTy T, VFuncSlot::KindTy kind) {
  for (auto &pair : T) {
    auto &guid = pair.first;
    auto impls = pair.second;
    using namespace record_block;

    MethodMetadataLayout MDLayout(Out);

    MDLayout.emit(ScratchRecord, unsigned(kind), guid);

    for (auto impl : impls) {
      MethodImplLayout ImplLayout(Out);
      ImplLayout.emit(ScratchRecord, impl);
    }
  }
}

void Serializer::emitModuleSummary(const ModuleSummaryIndex &index) {
  using namespace record_block;

  BCBlockRAII restoreBlock(Out, RECORD_BLOCK_ID, 4);
  ModuleMetadataLayout MDLayout(Out);
  MDLayout.emit(ScratchRecord, index.getModuleName());
  for (auto FI = index.functions_begin(), FE = index.functions_end(); FI != FE;
       ++FI) {
    emitFunctionSummary(FI->second.get());
  }

  emitVFuncTable(index.getWitnessTableMethodMap(), VFuncSlot::Witness);
  emitVFuncTable(index.getVTableMethodMap(), VFuncSlot::VTable);
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
  bool enterTopLevelBlock();
  bool readFunctionSummary(const BitstreamEntry &);
  bool readFunctionSummaryList();
  bool readModuleSummaryMetadata();
  bool readVirtualMethodInfo(const BitstreamEntry &entry);
  bool readVFuncTableList();

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

bool Deserializer::enterTopLevelBlock() {
  // Read the BLOCKINFO_BLOCK, which contains metadata used when dumping
  // the binary data with llvm-bcanalyzer.
  {
    auto next = Cursor.advance();
    if (!next) {
      consumeError(next.takeError());
      return true;
    }

    if (next->Kind != llvm::BitstreamEntry::SubBlock)
      return true;

    if (next->ID != llvm::bitc::BLOCKINFO_BLOCK_ID)
      return true;

    if (!Cursor.ReadBlockInfoBlock())
      return true;
  }

  // Enters our subblock, which contains the actual summary information.
  {
    auto next = Cursor.advance();
    if (!next) {
      consumeError(next.takeError());
      return true;
    }

    if (next->Kind != llvm::BitstreamEntry::SubBlock)
      return true;

    if (next->ID != RECORD_BLOCK_ID)
      return true;

    if (auto err = Cursor.EnterSubBlock(RECORD_BLOCK_ID)) {
      consumeError(std::move(err));
      return true;
    }
  }
  return false;
}

// TODO: Rename to readModuleMetadata
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

  moduleSummary.setName(BlobData.str());

  return false;
}

bool Deserializer::readFunctionSummary(const BitstreamEntry &entry) {
  using namespace record_block;
  assert(entry.ID == record_block::FUNC_METADATA);
  BitstreamEntry next = entry;

  GUID guid;
  std::string Name;
  FunctionSummary *FS;
  std::unique_ptr<FunctionSummary> NewFSOwner;

  unsigned isLive, isPreserved;
  FunctionMetadataLayout::readRecord(Scratch, guid, isLive, isPreserved);
  Name = BlobData.str();
  if (auto summary = moduleSummary.getFunctionSummary(guid)) {
    FS = summary;
  } else {
    NewFSOwner = std::make_unique<FunctionSummary>(guid);
    FS = NewFSOwner.get();
  }
  FS->setLive(isLive);
  FS->setPreserved(isPreserved);
  FS->setName(Name);

  auto maybeNext = Cursor.advance();
  if (!maybeNext)
    report_fatal_error("Should have next entry");

  next = maybeNext.get();

  while (next.Kind == BitstreamEntry::Record &&
         next.ID == record_block::CALL_GRAPH_EDGE) {
    Scratch.clear();

    auto maybeKind = Cursor.readRecord(next.ID, Scratch, &BlobData);

    if (!maybeKind)
      report_fatal_error("Should have kind");

    unsigned edgeKindID;
    GUID targetGUID;
    CallGraphEdgeLayout::readRecord(Scratch, edgeKindID, targetGUID);
    auto callKind = getEdgeKind(edgeKindID);
    if (!callKind)
      report_fatal_error("Bad edge kind");
    if (!FS)
      report_fatal_error("Invalid state");

    FS->addCall(targetGUID, BlobData.str(), callKind.getValue());

    auto maybeNext = Cursor.advance();
    if (!maybeNext)
      report_fatal_error("Should have next entry");

    next = maybeNext.get();
  }

  if (auto &FS = NewFSOwner) {
    moduleSummary.addFunctionSummary(std::move(FS));
  }
  return false;
}

bool Deserializer::readVirtualMethodInfo(const BitstreamEntry &entry) {
  using namespace record_block;
  assert(entry.ID == record_block::FUNC_METADATA);
  BitstreamEntry next = entry;

  Optional<VFuncSlot> slot;

  using namespace record_block;

  unsigned methodKindID;
  GUID virtualFuncGUID;
  MethodMetadataLayout::readRecord(Scratch, methodKindID, virtualFuncGUID);

  auto Kind = getSlotKind(methodKindID);
  if (!Kind)
    report_fatal_error("Bad method kind");

  slot = VFuncSlot(Kind.getValue(), virtualFuncGUID);

  auto maybeNext = Cursor.advance();
  if (!maybeNext)
    report_fatal_error("Should have next entry");

  next = maybeNext.get();

  while (next.Kind == BitstreamEntry::Record &&
         next.ID == record_block::METHOD_IMPL) {
    Scratch.clear();

    auto maybeKind = Cursor.readRecord(next.ID, Scratch, &BlobData);

    if (!maybeKind)
      report_fatal_error("Should have kind");

    GUID implGUID;
    MethodImplLayout::readRecord(Scratch, implGUID);
    if (!slot)
      report_fatal_error("Slot should be set before impl");
    moduleSummary.addImplementation(slot.getValue(), implGUID);

    auto maybeNext = Cursor.advance();
    if (!maybeNext)
      report_fatal_error("Should have next entry");

    next = maybeNext.get();
  }

  return false;
}

bool Deserializer::readFunctionSummaryList() {
  Expected<BitstreamEntry> maybeNext = Cursor.advance();
  if (!maybeNext) {
    // No function summary
    return false;
  }

  BitstreamEntry next = maybeNext.get();

  while (next.Kind == BitstreamEntry::Record &&
         next.ID == record_block::FUNC_METADATA) {
    if (readFunctionSummary(next)) {
      return true;
    }
  }
  return false;
}

bool Deserializer::readVFuncTableList() {
  Expected<BitstreamEntry> maybeNext = Cursor.advance();
  if (!maybeNext) {
    // No vfunc table
    return false;
  }

  BitstreamEntry next = maybeNext.get();

  while (next.Kind == BitstreamEntry::Record &&
         next.ID == record_block::METHOD_METADATA) {
    if (readVirtualMethodInfo(next)) {
      return true;
    }
  }
  return false;
}

bool Deserializer::readModuleSummary() {
  if (readSignature()) {
    return true;
  }
  if (enterTopLevelBlock()) {
    return true;
  }

  if (readModuleSummaryMetadata()) {
    return true;
  }

  if (readFunctionSummaryList()) {
    return true;
  }

  if (readVFuncTableList()) {
    return true;
  }

  return false;
}

}; // namespace

bool modulesummary::writeModuleSummaryIndex(const ModuleSummaryIndex &index,
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
