#ifndef SWIFT_SERIALIZATION_MODULE_SUMMARY_FILE_H
#define SWIFT_SERIALIZATION_MODULE_SUMMARY_FILE_H

#include "llvm/Bitcode/RecordLayout.h"
#include "llvm/Support/MemoryBuffer.h"
#include <memory>

namespace swift {

namespace modulesummary {

using llvm::BCArray;
using llvm::BCBlob;
using llvm::BCFixed;
using llvm::BCGenericRecordLayout;
using llvm::BCRecordLayout;
using llvm::BCVBR;

const unsigned char MODULE_SUMMARY_SIGNATURE[] = {'M', 'O', 'D', 'S'};

enum BlockID {
  MODULE_SUMMARY_ID = llvm::bitc::FIRST_APPLICATION_BLOCKID,

  FUNCTION_SUMMARY_ID,

  VIRTUAL_METHOD_INFO_ID,
};

namespace module_summary {
enum {
  MODULE_METADATA,
};

using ModuleMetadataLayout = BCRecordLayout<MODULE_METADATA,
                                            BCBlob // Module name
                                            >;
}; // namespace module_summary

namespace virtual_method_info {
enum {
  METHOD_METADATA,
  METHOD_IMPL,
};

using MethodMetadataLayout =
    BCRecordLayout<METHOD_METADATA,
                   BCFixed<1>, // KindTy (WitnessTable or VTable)
                   BCVBR<16>   // VirtualFunc GUID
                   >;

using MethodImplLayout = BCRecordLayout<METHOD_IMPL,
                                        BCVBR<16> // Impl func GUID
                                        >;
}; // namespace virtual_method_info

namespace function_summary {
enum {
  FUNC_METADATA,
  CALL_GRAPH_EDGE,
};

using FunctionMetadataLayout = BCRecordLayout<FUNC_METADATA,
                                              BCVBR<16>,  // Function GUID
                                              BCFixed<1>, // live
                                              BCFixed<1>, // preserved
                                              BCBlob      // Name string
                                              >;
using CallGraphEdgeLayout =
    BCRecordLayout<CALL_GRAPH_EDGE,
                   BCFixed<32>, // FunctionSummary::Edge::Kind
                   BCVBR<16>,   // Target Function GUID
                   BCBlob       // Name string
                   >;
} // namespace function_summary
} // namespace modulesummary
} // namespace swift

#endif
