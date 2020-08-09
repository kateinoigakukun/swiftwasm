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
static GUID getGUID(llvm::StringRef Str) { return llvm::MD5Hash(Str); }
struct VirtualMethodSlot {
  enum class KindTy {
    Witness, VTable,
    kindCount,
  };

  KindTy Kind;
  GUID VirtualFuncID;
  VirtualMethodSlot(KindTy kind, GUID virtualFuncID)
    : Kind(kind), VirtualFuncID(virtualFuncID) { }
  VirtualMethodSlot(SILDeclRef VirtualFuncRef, KindTy kind) : Kind(kind) {
    VirtualFuncID = getGUID(VirtualFuncRef.mangle());
  }

  bool operator<(const VirtualMethodSlot &rhs)  const {
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
    GUID CalleeFn;
    std::string Name;
  public:

    enum class Kind {
      Static,
      Witness,
      VTable,
      kindCount,
    };

    Kind kind;

    Call(SILDeclRef &CalleeFn, Kind kind) : kind(kind) {
      this->Name = CalleeFn.mangle();
      this->CalleeFn = getGUID(CalleeFn.mangle());
    }
    Call(GUID callee, std::string name, Kind kind)
        : CalleeFn(callee), Name(name), kind(kind) {}

  public:
    Kind getKind() const { return kind; }
    GUID getCallee() const { return CalleeFn; }
    std::string getName() const { return Name; };
    
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
      case Kind::Static: {
        os << "direct";
        break;
      }
      case Kind::kindCount: {
        llvm_unreachable("impossible");
      }
      }
      os << ", name: " << getName() << " , callee: " << getCallee()
         << ")\n";
    }

    VirtualMethodSlot slot() const {
      VirtualMethodSlot::KindTy slotKind;
      switch (kind) {
      case Kind::Witness: {
        slotKind = VirtualMethodSlot::KindTy::Witness;
        break;
      }
      case Kind::VTable: {
        slotKind = VirtualMethodSlot::KindTy::VTable;
        break;
      }
      case Kind::Static: {
        llvm_unreachable("Can't get slot for static call");
      }
      case Kind::kindCount: {
        llvm_unreachable("impossible");
      }
      }
      return VirtualMethodSlot(slotKind, CalleeFn);
    }

    static Call staticCall(SILFunction *CalleeFn) {
      GUID guid = getGUID(CalleeFn->getName());
      return Call(guid, CalleeFn->getName(), Kind::Static);
    }

    static Call witnessCall(SILDeclRef Callee) {
      return Call(Callee, Kind::Witness);
    }
    static Call vtableCall(SILDeclRef Callee) {
      return Call(Callee, Kind::VTable);
    }
  };

  
  struct FlagsTy {
    unsigned Live : 1;
    unsigned Preserved: 1;
  };

  using CallGraphEdgeListTy = std::vector<Call>;

private:
  FlagsTy Flags;
  CallGraphEdgeListTy CallGraphEdgeList;
  std::string debugName;

public:
  FunctionSummary(std::vector<Call> CGEdges)
      : CallGraphEdgeList(std::move(CGEdges)) {}
  FunctionSummary() = default;

  void addCall(GUID targetGUID, std::string name, Call::Kind kind) {
    CallGraphEdgeList.emplace_back(targetGUID, name, kind);
  }

  ArrayRef<Call> calls() const { return CallGraphEdgeList; }

  bool isLive() const { return Flags.Live; }
  void setLive(bool Live) { Flags.Live = Live; }

  bool isPreserved() const { return Flags.Preserved; }
  void setPreserved(bool Preserved) { Flags.Preserved = Preserved; }
  std::string getDebugName() const { return debugName; }
  void setDebugName(std::string name) { this->debugName = name; }
};

class ModuleSummaryIndex {
  using FunctionSummaryInfoMapTy = std::map<GUID, std::unique_ptr<FunctionSummary>>;
  using VirtualFunctionMapTy = std::map<VirtualMethodSlot, std::vector<GUID>>;

  FunctionSummaryInfoMapTy FunctionSummaryInfoMap;
  VirtualFunctionMapTy VirtualMethodInfoMap;

  std::string ModuleName;

public:
  ModuleSummaryIndex() = default;

  std::string getModuleName() const { return this->ModuleName; }
  void setModuleName(std::string name) {
    this->ModuleName = name;
  }

  void addFunctionSummary(std::string name,
                          std::unique_ptr<FunctionSummary> summary) {
    auto guid = getGUID(name);
    summary->setDebugName(name);
    FunctionSummaryInfoMap.insert(
        std::make_pair(guid, std::move(summary)));
  }

  const llvm::Optional<std::pair<FunctionSummary *, StringRef>>
  getFunctionInfo(GUID guid) const {
    auto found = FunctionSummaryInfoMap.find(guid);
    if (found == FunctionSummaryInfoMap.end()) {
      return None;
    }
    auto &entry = found->second;
    return std::make_pair(entry.get(), StringRef(entry->getDebugName()));
  }

  void addImplementation(VirtualMethodSlot slot, GUID funcGUID) {
    auto found = VirtualMethodInfoMap.find(slot);
    if (found == VirtualMethodInfoMap.end()) {
      VirtualMethodInfoMap.insert(std::make_pair(slot, std::vector<GUID>{ funcGUID }));
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

  const VirtualFunctionMapTy &virtualMethods() const {
    return VirtualMethodInfoMap;
  }

  FunctionSummaryInfoMapTy::const_iterator begin() const {
    return FunctionSummaryInfoMap.begin();
  }
  FunctionSummaryInfoMapTy::const_iterator end() const {
    return FunctionSummaryInfoMap.end();
  }
};

ModuleSummaryIndex buildModuleSummaryIndex(SILModule &M);

bool emitModuleSummaryIndex(const ModuleSummaryIndex &index,
                            DiagnosticEngine &diags, StringRef path);

bool loadModuleSummaryIndex(llvm::MemoryBufferRef inputBuffer,
                            ModuleSummaryIndex &moduleSummary);
} // namespace modulesummary
}; // namespace swift

#endif
