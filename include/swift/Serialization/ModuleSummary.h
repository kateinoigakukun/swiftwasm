//===----- ModuleSummary.h --------------------------------------*- C++ -*-===//
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

#ifndef SWIFT_SIL_MODULE_SUMMARY_H
#define SWIFT_SIL_MODULE_SUMMARY_H

#include "swift/AST/ASTMangler.h"
#include "swift/AST/Decl.h"
#include "swift/SIL/SILDeclRef.h"
#include "swift/SIL/SILFunction.h"
#include "llvm/Support/YAMLTraits.h"

namespace swift {

namespace modulesummary {

using GUID = uint64_t;

/// Compute globally unique identifier from the symbol.
GUID getGUIDFromUniqueName(llvm::StringRef Name);

/// Function summary information to help callee analysis
class FunctionSummary {
public:
  /// Function call information
  class Call {
  public:
    /// Kinds of callee reference.
    enum KindTy {
      /// The call references a function statically
      Direct,
      /// The call references a function via a witness table.
      Witness,
      /// The call references a function via a vtable.
      VTable,
      kindCount,
    };

  private:
    // For import/export
    friend llvm::yaml::MappingTraits<FunctionSummary::Call>;

    /// The callee function GUID. This can be a static function or a virtual
    /// function GUID.
    GUID Callee;
    /// The symbol name of the callee function only for debug and test purposes.
    std::string Name;
    /// Kind of the callee reference.
    KindTy Kind;
  public:
    Call() = default;
    Call(GUID callee, std::string name, KindTy kind)
        : Callee(callee), Name(name), Kind(kind) {}

    KindTy getKind() const { return Kind; }
    GUID getCallee() const { return Callee; }
    std::string getName() const { return Name; };
  };

  /// Function state flags
  struct FlagsTy {
    /// In per-module summary, always false.
    /// In combined summary, indicates that the function is live.
    bool Live;
    /// Indicates that the function must be considered a live root for liveness
    /// analysis.
    bool Preserved;
  };

  using CallGraphEdgeListTy = std::vector<Call>;

private:
  // For import/export
  friend llvm::yaml::MappingTraits<FunctionSummary>;

  /// The function identity.
  GUID Guid;
  /// The function state flags.
  FlagsTy Flags;
  /// List of Call from this function.
  CallGraphEdgeListTy CallGraphEdgeList;
  /// The symbol name of the function only for debug and test purposes.
  std::string Name;

public:
  FunctionSummary() = default;
  FunctionSummary(GUID guid) : Guid(guid), Flags({false, false}) {}

  void addCall(GUID calleeGUID, std::string name, Call::KindTy kind) {
    CallGraphEdgeList.emplace_back(calleeGUID, name, kind);
  }

  /// Add a call to the list.
  void addCall(Call call) { CallGraphEdgeList.push_back(call); }

  /// Return the list of Call from this function
  ArrayRef<Call> calls() const { return CallGraphEdgeList; }

  bool isLive() const { return Flags.Live; }
  void setLive(bool Live) { Flags.Live = Live; }

  bool isPreserved() const { return Flags.Preserved; }
  void setPreserved(bool Preserved) { Flags.Preserved = Preserved; }

  std::string getName() const { return Name; }
  void setName(std::string name) { this->Name = name; }

  GUID getGUID() const { return Guid; }
};

/// A slot in a set of virtual tables.
struct VFuncSlot {
  /// Kinds of table.
  enum KindTy {
    Witness,
    VTable,
    kindCount,
  };

  /// Kind of table.
  KindTy Kind;
  /// The virtual function GUID.
  GUID VFuncID;

  VFuncSlot(KindTy Kind, GUID VFuncID) : Kind(Kind), VFuncID(VFuncID) {}

  VFuncSlot(SILDeclRef VFuncRef, KindTy Kind) : Kind(Kind) {
    VFuncID = getGUIDFromUniqueName(VFuncRef.mangle());
  }
};

using FunctionSummaryMapTy = std::map<GUID, std::unique_ptr<FunctionSummary>>;
using VFuncToImplsMapTy = std::map<GUID, std::vector<GUID>>;

/// Module summary that consists of function summaries and virtual function
/// tables.
class ModuleSummaryIndex {
  // For import/export
  friend llvm::yaml::MappingTraits<ModuleSummaryIndex>;

  /// Map from function GUID to function summary.
  FunctionSummaryMapTy FunctionSummaryMap;
  /// Map from virtual function GUID to list of implementations for witness
  /// tables.
  VFuncToImplsMapTy WitnessTableMethodMap;
  /// Map from virtual function GUID to list of implementations for vtables.
  VFuncToImplsMapTy VTableMethodMap;
  /// The symbol name of the module.
  std::string Name;

  VFuncToImplsMapTy &getVFuncMap(VFuncSlot::KindTy kind) {
    switch (kind) {
    case VFuncSlot::Witness:
      return WitnessTableMethodMap;
    case VFuncSlot::VTable:
      return VTableMethodMap;
    case VFuncSlot::kindCount: {
      llvm_unreachable("impossible");
    }
    }
  }
  const VFuncToImplsMapTy &getVFuncMap(VFuncSlot::KindTy kind) const {
    switch (kind) {
    case VFuncSlot::Witness:
      return WitnessTableMethodMap;
    case VFuncSlot::VTable:
      return VTableMethodMap;
    case VFuncSlot::kindCount: {
      llvm_unreachable("impossible");
    }
    }
  }
public:
  ModuleSummaryIndex() = default;

  std::string getName() const { return this->Name; }
  void setName(std::string name) { this->Name = name; }

  /// Add a global value summary.
  void addFunctionSummary(std::unique_ptr<FunctionSummary> summary) {
    FunctionSummaryMap.insert(
        std::make_pair(summary->getGUID(), std::move(summary)));
  }

  /// Return a FunctionSummary for GUID if it exists, otherwise return nullptr.
  FunctionSummary *getFunctionSummary(GUID guid) const {
    auto found = FunctionSummaryMap.find(guid);
    if (found == FunctionSummaryMap.end()) {
      return nullptr;
    }
    auto &entry = found->second;
    return entry.get();
  }

  /// Record a implementation for the virtual function slot.
  void addImplementation(VFuncSlot slot, GUID implGUID) {
    VFuncToImplsMapTy &table = getVFuncMap(slot.Kind);
    auto found = table.find(slot.VFuncID);
    if (found == table.end()) {
      table.insert(std::make_pair(slot.VFuncID, std::vector<GUID>{implGUID}));
      return;
    }
    found->second.push_back(implGUID);
  }

  /// Return a list of implementations for the virtual function slot.
  ArrayRef<GUID> getImplementations(VFuncSlot slot) const {
    const VFuncToImplsMapTy &table = getVFuncMap(slot.Kind);
    auto found = table.find(slot.VFuncID);
    if (found == table.end()) {
      return ArrayRef<GUID>();
    }
    return ArrayRef<GUID>(found->second);
  }

  const VFuncToImplsMapTy &getWitnessTableMethodMap() const {
    return WitnessTableMethodMap;
  }
  const VFuncToImplsMapTy &getVTableMethodMap() const {
    return VTableMethodMap;
  }

  FunctionSummaryMapTy::const_iterator functions_begin() const {
    return FunctionSummaryMap.begin();
  }
  FunctionSummaryMapTy::const_iterator functions_end() const {
    return FunctionSummaryMap.end();
  }
};

std::unique_ptr<ModuleSummaryIndex> buildModuleSummaryIndex(SILModule &M);

/// Serializes a module summary to the given output file.
///
/// \returns false on success, true on error.
bool writeModuleSummaryIndex(const ModuleSummaryIndex &index,
                             DiagnosticEngine &diags, StringRef path);

/// Attempt to deserialize the module summary.
///
/// \returns false on success, true on error.
bool loadModuleSummaryIndex(llvm::MemoryBufferRef inputBuffer,
                            ModuleSummaryIndex &moduleSummary);
} // namespace modulesummary
} // namespace swift

#endif
