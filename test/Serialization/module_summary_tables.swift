// RUN: %empty-directory(%t)
// RUN: %target-swift-frontend -emit-sib -emit-module-summary-path %t/tables.swiftmodule.summary -module-name tables -Xllvm -module-summary-embed-debug-name %s
// RUN: llvm-bcanalyzer -dump %t/function_summary.swiftmodule.summary | %FileCheck %s -check-prefix BCANALYZER

// BCANALYZER-NOT: UnknownCode

// `bar` references `foo` directly
// RUN: %swift-module-summary-test --to-yaml %t/tables.swiftmodule.summary -o - | %FileCheck %s


protocol P {
    func protoMember()
}

struct S : P {
    func protoMember() {}
}


class C {
    func classMember() {}
}

class D : C {
    override func classMember() {}
}
