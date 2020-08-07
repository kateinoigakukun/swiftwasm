#ifndef SWIFT_SIL_MODULE_SUMMARY_H
#define SWIFT_SIL_MODULE_SUMMARY_H

#include "swift/AST/ASTMangler.h"
#include "swift/AST/Decl.h"
#include "swift/SIL/SILDeclRef.h"
#include "swift/SIL/SILFunction.h"

namespace swift {

namespace modulesummary {
using GUID = uint64_t;

GUID getGUIDFromUniqueName(llvm::StringRef Name);

struct VirtualMethodSlot {
  enum Kind {
    Witness,
    VTable,
    kindCount,
  };

  Kind kind;
  GUID virtualFuncID;
  VirtualMethodSlot(Kind kind, GUID virtualFuncID)
      : kind(kind), virtualFuncID(virtualFuncID) {}
  VirtualMethodSlot(const SILDeclRef &VirtualFuncRef, Kind kind) : kind(kind) {
    virtualFuncID = getGUIDFromUniqueName(VirtualFuncRef.mangle());
  }

  bool operator<(const VirtualMethodSlot &rhs) const {
    if (kind > rhs.kind)
      return false;
    if (kind < rhs.kind)
      return true;
    return virtualFuncID < rhs.virtualFuncID;
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
      VirtualMethodSlot::Kind slotKind;
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
  using VirtualMethodMapTy = std::map<VirtualMethodSlot, std::vector<GUID>>;

private:
  FunctionSummaryMapTy FunctionSummaryInfoMap;
  VirtualMethodMapTy VirtualMethodInfoMap;

  std::string ModuleName;

public:
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

  void addImplementation(VirtualMethodSlot slot, GUID funcGUID) {
    auto found = VirtualMethodInfoMap.find(slot);
    if (found == VirtualMethodInfoMap.end()) {
      VirtualMethodInfoMap.insert(
          std::make_pair(slot, std::vector<GUID>{funcGUID}));
      return;
    }
    found->second.push_back(funcGUID);
  }

  llvm::Optional<ArrayRef<GUID>>
  getImplementations(VirtualMethodSlot slot) const {
    auto found = VirtualMethodInfoMap.find(slot);
    if (found == VirtualMethodInfoMap.end()) {
      return None;
    }
    return ArrayRef<GUID>(found->second);
  }

  const VirtualMethodMapTy &virtualMethods() const {
    return VirtualMethodInfoMap;
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
