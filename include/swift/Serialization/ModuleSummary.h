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
  GUID VFuncID;
  VirtualMethodSlot(KindTy Kind, GUID VFuncID)
    : Kind(Kind), VFuncID(VFuncID) { }
  VirtualMethodSlot(SILDeclRef VirtualFuncRef, KindTy Kind) : Kind(Kind) {
      VFuncID = getGUIDFromUniqueName(VirtualFuncRef.mangle());
  }

  bool operator<(const VirtualMethodSlot &rhs)  const {
    if (Kind > rhs.Kind)
      return false;
    if (Kind < rhs.Kind)
      return true;
    return VFuncID < rhs.VFuncID;
  }
};

class FunctionSummary {
public:
  class Call {
  public:
    enum KindTy {
      Direct,
      Witness,
      VTable,
      kindCount,
    };
  private:
    GUID Callee;
    std::string Name;
    KindTy Kind;
  public:
    friend llvm::yaml::MappingTraits<FunctionSummary::Call>;
    Call() = default;
    Call(GUID callee, std::string name, KindTy kind)
        : Callee(callee), Name(name), Kind(kind) {}

    KindTy getKind() const { return Kind; }
    GUID getCallee() const { return Callee; }
    std::string getName() const { return Name; };
    
    void dump(llvm::raw_ostream &os) const {
      os << "call: (kind: ";
      switch (Kind) {
      case KindTy::Witness: {
        os << "witness";
        break;
      }
      case KindTy::VTable: {
        os << "vtable";
        break;
      }
      case KindTy::Direct: {
        os << "direct";
        break;
      }
      case KindTy::kindCount: {
        llvm_unreachable("impossible");
      }
      }
      os << ", name: " << getName() << " , callee: " << getCallee()
         << ")\n";
    }

    VirtualMethodSlot slot() const {
      VirtualMethodSlot::KindTy slotKind;
      switch (Kind) {
      case KindTy::Witness: {
        slotKind = VirtualMethodSlot::KindTy::Witness;
        break;
      }
      case KindTy::VTable: {
        slotKind = VirtualMethodSlot::KindTy::VTable;
        break;
      }
      case KindTy::Direct: {
        llvm_unreachable("Can't get slot for static call");
      }
      case KindTy::kindCount: {
        llvm_unreachable("impossible");
      }
      }
      return VirtualMethodSlot(slotKind, Callee);
    }
  };

  
  struct FlagsTy {
    bool Live;
    bool Preserved;
  };

  using CallGraphEdgeListTy = std::vector<Call>;

private:
  GUID Guid;
  FlagsTy Flags;
  CallGraphEdgeListTy CallGraphEdgeList;
  std::string Name;

public:
  friend llvm::yaml::MappingTraits<FunctionSummary>;
  FunctionSummary(GUID guid) : Guid(guid), Flags({false, false}) {}
  FunctionSummary() = default;

  void addCall(GUID targetGUID, std::string name, Call::KindTy kind) {
    CallGraphEdgeList.emplace_back(targetGUID, name, kind);
  }

  void addCall(Call call) { CallGraphEdgeList.push_back(call); }

  ArrayRef<Call> calls() const { return CallGraphEdgeList; }

  bool isLive() const { return Flags.Live; }
  void setLive(bool Live) { Flags.Live = Live; }

  bool isPreserved() const { return Flags.Preserved; }
  void setPreserved(bool Preserved) { Flags.Preserved = Preserved; }
  std::string getName() const { return Name; }
  void setName(std::string name) { this->Name = name; }
  GUID getGUID() const { return Guid; }
};

using FunctionSummaryMapTy = std::map<GUID, std::unique_ptr<FunctionSummary>>;
using VFuncToImplsMapTy = std::map<GUID, std::vector<GUID>>;

class ModuleSummaryIndex {
  FunctionSummaryMapTy FunctionSummaryInfoMap;
  VFuncToImplsMapTy WitnessTableMethodMap;
  VFuncToImplsMapTy VTableMethodMap;

  std::string Name;
    VFuncToImplsMapTy &getVFuncMap(VirtualMethodSlot::KindTy kind) {
        switch (kind) {
            case VirtualMethodSlot::Witness: return WitnessTableMethodMap;
            case VirtualMethodSlot::VTable: return VTableMethodMap;
            case VirtualMethodSlot::kindCount: {
              llvm_unreachable("impossible");
            }
        }
    }
    const VFuncToImplsMapTy &getVFuncMap(VirtualMethodSlot::KindTy kind) const {
        switch (kind) {
            case VirtualMethodSlot::Witness: return WitnessTableMethodMap;
            case VirtualMethodSlot::VTable: return VTableMethodMap;
            case VirtualMethodSlot::kindCount: {
              llvm_unreachable("impossible");
            }
        }
    }
public:
  friend llvm::yaml::MappingTraits<ModuleSummaryIndex>;
  ModuleSummaryIndex() = default;

  std::string getModuleName() const { return this->Name; }
  void setName(std::string name) { this->Name = name; }

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
      auto table = getVFuncMap(slot.Kind);
    auto found = table.find(slot.VFuncID);
    if (found == table.end()) {
      table.insert(std::make_pair(slot.VFuncID, std::vector<GUID>{ funcGUID }));
      return;
    }
    found->second.push_back(funcGUID);
  }

  ArrayRef<GUID>
  getImplementations(VirtualMethodSlot slot) const {
      auto table = getVFuncMap(slot.Kind);
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
