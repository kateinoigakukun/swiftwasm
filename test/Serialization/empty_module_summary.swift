// RUN: %empty-directory(%t)
// RUN: %target-swift-frontend -emit-sib -emit-module-summary-path %t/empty.swiftmodule.summary -module-name empty %s
// RUN: llvm-bcanalyzer -dump %t/empty.swiftmodule.summary | %FileCheck %s

// CHECK-NOT: UnknownCode
// CHECK: <MODULE_SUMMARY {{.*}}>
// CHECK: <MODULE_METADATA abbrevid={{.*}}/> blob data = 'empty'
// CHECK: </MODULE_SUMMARY>
// CHECK-NOT: <MODULE_SUMMARY {{.*}}>
