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
  enum KindTy {
    Witness,
    VTable,
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
    enum Kind {
      Direct,
      Witness,
      VTable,
      kindCount,
    };

    Kind kind;

    Call(SILDeclRef &CalleeFn, Kind kind) : kind(kind) {
      this->Name = CalleeFn.mangle();
      this->CalleeFn = getGUIDFromUniqueName(CalleeFn.mangle());
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
      case Kind::Direct: {
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
      case Kind::Direct: {
        llvm_unreachable("Can't get slot for static call");
      }
      case Kind::kindCount: {
        llvm_unreachable("impossible");
      }
      }
      return VirtualMethodSlot(slotKind, CalleeFn);
    }
  };

  
  struct FlagsTy {
    unsigned Live : 1;
    unsigned Preserved: 1;
  };

  using CallGraphEdgeListTy = std::vector<Call>;

private:
  GUID guid;
  FlagsTy Flags;
  CallGraphEdgeListTy CallGraphEdgeList;
  std::string debugName;

public:
  FunctionSummary(GUID guid) : guid(guid) {}
//  FunctionSummary() = default;

  void addCall(GUID targetGUID, std::string name, Call::Kind kind) {
    CallGraphEdgeList.emplace_back(targetGUID, name, kind);
  }

  void addCall(Call call) { CallGraphEdgeList.push_back(call); }

  ArrayRef<Call> calls() const { return CallGraphEdgeList; }

  bool isLive() const { return Flags.Live; }
  void setLive(bool Live) { Flags.Live = Live; }

  bool isPreserved() const { return Flags.Preserved; }
  void setPreserved(bool Preserved) { Flags.Preserved = Preserved; }
  std::string getDebugName() const { return debugName; }
  void setDebugName(std::string name) { this->debugName = name; }
  GUID getGUID() const { return guid; }
};

class ModuleSummaryIndex {
  using FunctionSummaryMapTy = std::map<GUID, std::unique_ptr<FunctionSummary>>;
  using VirtualFunctionMapTy = std::map<VirtualMethodSlot, std::vector<GUID>>;

  FunctionSummaryMapTy FunctionSummaryInfoMap;
  VirtualFunctionMapTy VirtualMethodInfoMap;

  std::string ModuleName;

public:
  ModuleSummaryIndex() = default;

  std::string getModuleName() const { return this->ModuleName; }
  void setModuleName(std::string name) {
    this->ModuleName = name;
  }

  void addFunctionSummary(std::unique_ptr<FunctionSummary> summary) {
    FunctionSummaryInfoMap.insert(
        std::make_pair(summary->getGUID(), std::move(summary)));
  }

  FunctionSummary *getFunctionSummary(GUID guid) const {
    auto found = FunctionSummaryInfoMap.find(guid);
    if (found == FunctionSummaryInfoMap.end()) {
      return nullptr;
    }
    auto &entry = found->second;
    return entry.get();
  }

  void addImplementation(VirtualMethodSlot slot, GUID funcGUID) {
    auto found = VirtualMethodInfoMap.find(slot);
    if (found == VirtualMethodInfoMap.end()) {
      VirtualMethodInfoMap.insert(std::make_pair(slot, std::vector<GUID>{ funcGUID }));
      return;
    }
    found->second.push_back(funcGUID);
  }

  ArrayRef<GUID>
  getImplementations(VirtualMethodSlot slot) const {
    auto found = VirtualMethodInfoMap.find(slot);
    if (found == VirtualMethodInfoMap.end()) {
      return ArrayRef<GUID>();
    }
    return ArrayRef<GUID>(found->second);
  }

  const VirtualFunctionMapTy &virtualMethods() const {
    return VirtualMethodInfoMap;
  }

  FunctionSummaryMapTy::const_iterator functions_begin() const {
    return FunctionSummaryInfoMap.begin();
  }
  FunctionSummaryMapTy::const_iterator functions_end() const {
    return FunctionSummaryInfoMap.end();
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
