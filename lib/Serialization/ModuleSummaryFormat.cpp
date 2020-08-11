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

  BCBlockRAII restoreBlock(Out, RECORD_BLOCK_ID, 32);
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
  bool readFunctionSummary(FunctionSummary &FS);
  bool readFunctionSummaryList();
  bool readModuleSummaryMetadata();
  bool readVirtualMethodInfo(VFuncSlot slot);
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

bool Deserializer::readFunctionSummary(FunctionSummary &FS) {
  using namespace record_block;

  Expected<BitstreamEntry> entry = Cursor.advance();
  if (!entry)
    report_fatal_error("Should have next entry");

  if (entry->Kind != BitstreamEntry::Record) {
    return false;
  }

  Scratch.clear();
  auto recordID = Cursor.readRecord(entry->ID, Scratch, &BlobData);

  if (!recordID)
    report_fatal_error("Should have kind");

  while (recordID.get() == record_block::CALL_GRAPH_EDGE) {
    unsigned edgeKindID;
    GUID targetGUID;
    CallGraphEdgeLayout::readRecord(Scratch, edgeKindID, targetGUID);
    auto callKind = getEdgeKind(edgeKindID);
    if (!callKind)
      report_fatal_error("Bad edge kind");

    FS.addCall(targetGUID, BlobData.str(), callKind.getValue());

    entry = Cursor.advance();
    if (!entry)
      report_fatal_error("Should have next entry");

    if (entry->Kind != BitstreamEntry::Record) {
      return false;
    }

    Scratch.clear();
    recordID = Cursor.readRecord(entry->ID, Scratch, &BlobData);

    if (!recordID)
      report_fatal_error("Should have kind");
  }

  return false;
}

bool Deserializer::readVirtualMethodInfo(VFuncSlot slot) {
  using namespace record_block;
  Expected<BitstreamEntry> entry = Cursor.advance();
  if (!entry) {
    return true;
  }

  Scratch.clear();
  auto recordID = Cursor.readRecord(entry->ID, Scratch, &BlobData);

  if (!recordID)
    report_fatal_error("Should have kind");

  while (entry->Kind == BitstreamEntry::Record &&
         recordID.get() == record_block::METHOD_IMPL) {
    GUID implGUID;
    MethodImplLayout::readRecord(Scratch, implGUID);
    moduleSummary.addImplementation(slot, implGUID);

    entry = Cursor.advance();
    if (!entry)
      report_fatal_error("Should have next entry");

    recordID = Cursor.readRecord(entry->ID, Scratch, &BlobData);

    if (!recordID)
      report_fatal_error("Should have kind");
    Scratch.clear();
  }

  return false;
}

bool Deserializer::readFunctionSummaryList() {
  using namespace record_block;
  unsigned recordID;
  std::unique_ptr<FunctionSummary> NewFSOwner;
  FunctionSummary *Current;

  do {
    Expected<BitstreamEntry> entry = Cursor.advance();
    if (!entry) {
      return false;
    }

    if (entry->Kind != BitstreamEntry::Record) {
      return false;
    }
    auto maybeRecordID = Cursor.readRecord(entry->ID, Scratch, &BlobData);
    if (!maybeRecordID) {
      return false;
    }
    recordID = maybeRecordID.get();
    switch (recordID) {
    case record_block::FUNC_METADATA: {
      if (auto &FS = NewFSOwner) {
        moduleSummary.addFunctionSummary(std::move(FS));
      }
      GUID guid;
      std::string Name;

      unsigned isLive, isPreserved;
      FunctionMetadataLayout::readRecord(Scratch, guid, isLive, isPreserved);
      Name = BlobData.str();
      if (auto summary = moduleSummary.getFunctionSummary(guid)) {
        Current = summary;
      } else {
        NewFSOwner = std::make_unique<FunctionSummary>(guid);
        Current = NewFSOwner.get();
      }
      Current->setLive(isLive);
      Current->setPreserved(isPreserved);
      Current->setName(Name);
      break;
    }
    case record_block::CALL_GRAPH_EDGE: {
      unsigned edgeKindID;
      GUID targetGUID;
      CallGraphEdgeLayout::readRecord(Scratch, edgeKindID, targetGUID);
      auto callKind = getEdgeKind(edgeKindID);
      if (!callKind)
        report_fatal_error("Bad edge kind");

      Current->addCall(targetGUID, BlobData.str(), callKind.getValue());
      break;
    }
    default:
      break;
    }
  } while (recordID == record_block::FUNC_METADATA ||
           recordID == record_block::CALL_GRAPH_EDGE);

  if (auto &FS = NewFSOwner) {
    moduleSummary.addFunctionSummary(std::move(FS));
  }
  return false;
}

bool Deserializer::readVFuncTableList() {
  using namespace record_block;
  Expected<BitstreamEntry> entry = Cursor.advance();
  if (!entry) {
    // No vfunc table
    return false;
  }

  auto recordID = Cursor.readRecord(entry->ID, Scratch, &BlobData);
  if (!recordID) {
    return false;
  }

  while (entry->Kind == BitstreamEntry::Record &&
         recordID.get() == record_block::METHOD_METADATA) {
    unsigned rawVFuncKind;
    GUID vFuncGUID;
    MethodMetadataLayout::readRecord(Scratch, rawVFuncKind, vFuncGUID);

    auto Kind = getSlotKind(rawVFuncKind);
    if (!Kind) {
      return true;
    }

    VFuncSlot slot = VFuncSlot(Kind.getValue(), vFuncGUID);

    if (readVirtualMethodInfo(slot)) {
      return true;
    }

    entry = Cursor.advance();
    if (!entry)
      report_fatal_error("Should have next entry");
    Scratch.clear();
    recordID = Cursor.readRecord(entry->ID, Scratch, &BlobData);
    if (!recordID) {
      return false;
    }
  }
  return false;
}

bool Deserializer::readModuleSummary() {
  using namespace record_block;
  if (readSignature()) {
    return true;
  }
  if (enterTopLevelBlock()) {
    return true;
  }

  if (readModuleSummaryMetadata()) {
    return true;
  }

  std::unique_ptr<FunctionSummary> NewFSOwner;
  FunctionSummary *CurrentFunc;
  Optional<VFuncSlot> CurrentSlot;

  while (!Cursor.AtEndOfStream()) {
    Scratch.clear();
    Expected<BitstreamEntry> entry = Cursor.advance();
    if (!entry) {
      // no content
      return false;
    }
    if (entry->Kind == llvm::BitstreamEntry::EndBlock) {
      Cursor.ReadBlockEnd();
      break;
    }

    if (entry->Kind != llvm::BitstreamEntry::Record) {
      llvm::report_fatal_error("Bad bitstream entry kind");
    }

    auto recordID = Cursor.readRecord(entry->ID, Scratch, &BlobData);
    if (!recordID) {
      return true;
    }

    switch (recordID.get()) {
    case MODULE_METADATA:
      // METADATA must appear at the beginning and is handled by
      // readModuleSummaryMetadata().
      llvm::report_fatal_error("Unexpected MODULE_METADATA record");
    case FUNC_METADATA: {
      GUID guid;
      std::string Name;

      unsigned isLive, isPreserved;
      FunctionMetadataLayout::readRecord(Scratch, guid, isLive, isPreserved);
      Name = BlobData.str();
      if (auto summary = moduleSummary.getFunctionSummary(guid)) {
        CurrentFunc = summary;
      } else {
        auto NewFS = std::make_unique<FunctionSummary>(guid);
        CurrentFunc = NewFS.get();
        moduleSummary.addFunctionSummary(std::move(NewFS));
      }
      CurrentFunc->setLive(isLive);
      CurrentFunc->setPreserved(isPreserved);
      CurrentFunc->setName(Name);
      break;
    }
    case CALL_GRAPH_EDGE: {
      if (!CurrentFunc) {
        report_fatal_error("Unexpected CALL_GRAPH_EDGE record");
      }
      unsigned edgeKindID;
      GUID targetGUID;
      CallGraphEdgeLayout::readRecord(Scratch, edgeKindID, targetGUID);
      auto callKind = getEdgeKind(edgeKindID);
      if (!callKind)
        report_fatal_error("Bad edge kind");
      CurrentFunc->addCall(targetGUID, BlobData.str(), callKind.getValue());
      break;
    }
    case METHOD_METADATA: {
      unsigned rawVFuncKind;
      GUID vFuncGUID;
      MethodMetadataLayout::readRecord(Scratch, rawVFuncKind, vFuncGUID);

      auto Kind = getSlotKind(rawVFuncKind);
      if (!Kind) {
        return true;
      }
      CurrentSlot = VFuncSlot(Kind.getValue(), vFuncGUID);
      break;
    }
    case METHOD_IMPL: {
      if (!CurrentSlot) {
        report_fatal_error("Unexpected METHOD_IMPL record");
      }
      GUID implGUID;
      MethodImplLayout::readRecord(Scratch, implGUID);
      moduleSummary.addImplementation(CurrentSlot.getValue(), implGUID);
      break;
    }
    }
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
