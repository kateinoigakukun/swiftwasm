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

GUID getGUIDFromUniqueName(llvm::StringRef Name);

struct VirtualMethodSlot {
  enum KindTy {
    Witness,
    VTable,
    kindCount,
  };

  KindTy Kind;
  GUID VirtualFuncID;
  VirtualMethodSlot(KindTy kind, GUID virtualFuncID)
      : Kind(kind), VirtualFuncID(virtualFuncID) {}
  VirtualMethodSlot(const SILDeclRef &VirtualFuncRef, KindTy kind)
      : Kind(kind) {
    VirtualFuncID = getGUIDFromUniqueName(VirtualFuncRef.mangle());
  }

  bool operator<(const VirtualMethodSlot &rhs) const {
    if (Kind > rhs.Kind)
      return false;
    if (Kind < rhs.Kind)
      return true;
    return VirtualFuncID < rhs.VirtualFuncID;
  }
};

class FunctionSummary {
public:
  class Call {
  public:
    enum Kind {
      Direct,
      Witness,
      VTable,
      kindCount,
    };

  private:
    GUID calleeFn;
    Kind kind;
    std::string debugName;

  public:
    friend ::llvm::yaml::MappingTraits<Call>;
    Call() = default;
    Call(GUID callee, Kind kind, std::string debugName)
        : calleeFn(callee), kind(kind), debugName(debugName) {}

    Kind getKind() const { return kind; }
    GUID getCallee() const { return calleeFn; }
    std::string getDebugName() const { return debugName; };
    void setDebugName(std::string name) { debugName = name; }

    void dump(llvm::raw_ostream &os) const {
      os << "call: (kind: ";
      switch (kind) {
      case Kind::Witness: {
        os << "witness";
        break;
      }
      case Kind::VTable: {
        os << "vtable";
        break;
      }
      case Kind::Direct: {
        os << "direct";
        break;
      }
      case Kind::kindCount: {
        llvm_unreachable("impossible");
      }
      }
      os << ", name: " << getDebugName() << " , callee: " << getCallee()
         << ")\n";
    }

    VirtualMethodSlot slot() const {
      VirtualMethodSlot::KindTy slotKind;
      switch (kind) {
      case Kind::Witness: {
        slotKind = VirtualMethodSlot::Witness;
        break;
      }
      case Kind::VTable: {
        slotKind = VirtualMethodSlot::VTable;
        break;
      }
      case Kind::Direct: {
        llvm_unreachable("Can't get slot for static call");
      }
      case Kind::kindCount: {
        llvm_unreachable("impossible");
      }
      }
      return VirtualMethodSlot(slotKind, calleeFn);
    }
  };

  struct FlagsTy {
    bool Live;
    bool Preserved;
  };

  using CallGraphEdgeListTy = std::vector<Call>;

private:
  GUID guid;
  FlagsTy flags;
  CallGraphEdgeListTy CallGraphEdgeList;
  std::string debugName;

public:
  friend ::llvm::yaml::MappingTraits<FunctionSummary>;
  FunctionSummary() = default;
  FunctionSummary(GUID guid) : guid(guid) {}

  GUID getGUID() const { return guid; }
  void addCall(Call call) { CallGraphEdgeList.push_back(call); }

  ArrayRef<Call> calls() const { return CallGraphEdgeList; }

  bool isLive() const { return flags.Live; }
  void setLive(bool Live) { flags.Live = Live; }

  bool isPreserved() const { return flags.Preserved; }
  void setPreserved(bool Preserved) { flags.Preserved = Preserved; }

  std::string getDebugName() const { return debugName; }
  void setDebugName(std::string name) { this->debugName = name; }

  void dump(llvm::raw_ostream &os) const {
    os << "(func " << getDebugName();
    if (!getDebugName().empty()) {
      os << "name: " << getDebugName() << ", ";
    }
    os << "live: " << isLive() << ", ";
    os << "preserved: " << isPreserved() << ", ";
    os << "guid: " << getGUID() << ")\n";
  }
};

class ModuleSummaryIndex {
public:
  using FunctionSummaryMapTy = std::map<GUID, std::unique_ptr<FunctionSummary>>;
  using VirtualFunctionMapTy = std::map<GUID, std::vector<GUID>>;

private:
  FunctionSummaryMapTy FunctionSummaryInfoMap;
  VirtualFunctionMapTy WitnessFunctionMap;
  VirtualFunctionMapTy VTableFunctionMap;

  std::string ModuleName;

  const VirtualFunctionMapTy &
  getFunctionMap(VirtualMethodSlot::KindTy kind) const {
    switch (kind) {
    case VirtualMethodSlot::Witness:
      return WitnessFunctionMap;
    case VirtualMethodSlot::VTable:
      return VTableFunctionMap;
    case VirtualMethodSlot::kindCount: {
      llvm_unreachable("impossible");
    }
    }
  }
  VirtualFunctionMapTy &getFunctionMap(VirtualMethodSlot::KindTy kind) {
    switch (kind) {
    case VirtualMethodSlot::Witness:
      return WitnessFunctionMap;
    case VirtualMethodSlot::VTable:
      return VTableFunctionMap;
    case VirtualMethodSlot::kindCount: {
      llvm_unreachable("impossible");
    }
    }
  }

public:
  friend ::llvm::yaml::MappingTraits<ModuleSummaryIndex>;
  ModuleSummaryIndex() = default;

  std::string getModuleName() const { return this->ModuleName; }
  void setModuleName(std::string name) { this->ModuleName = name; }

  void addFunctionSummary(std::unique_ptr<FunctionSummary> summary) {
    auto guid = summary->getGUID();
    FunctionSummaryInfoMap.insert(std::make_pair(guid, std::move(summary)));
  }

  const llvm::Optional<FunctionSummary *> getFunctionSummary(GUID guid) const {
    auto found = FunctionSummaryInfoMap.find(guid);
    if (found == FunctionSummaryInfoMap.end()) {
      return None;
    }
    return found->second.get();
  }

  void addImplementation(VirtualMethodSlot slot, GUID implFuncGUID) {
    auto table = getFunctionMap(slot.Kind);
    auto found = table.find(slot.VirtualFuncID);
    if (found == table.end()) {
      table.insert(
          std::make_pair(slot.VirtualFuncID, std::vector<GUID>{implFuncGUID}));
      return;
    }
    found->second.push_back(implFuncGUID);
  }

  llvm::Optional<ArrayRef<GUID>>
  getImplementations(VirtualMethodSlot slot) const {
    auto table = getFunctionMap(slot.Kind);
    auto found = table.find(slot.VirtualFuncID);
    if (found == table.end()) {
      return None;
    }
    return ArrayRef<GUID>(found->second);
  }

  const VirtualFunctionMapTy &getWitnessMethods() const {
    return WitnessFunctionMap;
  }
  const VirtualFunctionMapTy &getVTableMethods() const {
    return VTableFunctionMap;
  }

  FunctionSummaryMapTy::const_iterator functions_begin() const {
    return FunctionSummaryInfoMap.begin();
  }
  FunctionSummaryMapTy::const_iterator functions_end() const {
    return FunctionSummaryInfoMap.end();
  }

  void dump(llvm::raw_ostream &os) const {
    os << "(module name: " << ModuleName << ")\n";
    for (auto &entry : FunctionSummaryInfoMap) {
      entry.second->dump(os);
    }
  }
};

std::unique_ptr<ModuleSummaryIndex> buildModuleSummaryIndex(SILModule &M);

bool emitModuleSummaryIndex(const ModuleSummaryIndex &index,
                            DiagnosticEngine &diags, StringRef path);

bool loadModuleSummaryIndex(llvm::MemoryBufferRef inputBuffer,
                            ModuleSummaryIndex &moduleSummary);
} // namespace modulesummary
}; // namespace swift

#endif
