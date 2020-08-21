// RUN: %empty-directory(%t)
// RUN: cd %t
// RUN: %target-swift-frontend -emit-sib -emit-module-summary-path %t/vtable.swiftmodule.summary -O -module-name vtable -Xllvm -module-summary-embed-debug-name %s
// RUN: %swift_frontend_plain -cross-module-opt %t/vtable.swiftmodule.summary -module-summary-embed-debug-name -o %t/vtable.swiftmodule.merged-summary
// RUN: %swift-module-summary-test --to-yaml %t/vtable.swiftmodule.merged-summary -o %t/vtable.merged-summary.yaml

// RUN: %target-swift-frontend -c -module-summary-path %t/vtable.swiftmodule.merged-summary vtable.sib -o %t/vtable.o
// RUN: %target-swiftc_driver %t/vtable.o -o %t/vtable
// RUN: %t/vtable

public class A1 {
    let b: B1
    public init(b: B1) {
        self.b = b
    }
    public func run2() -> Int {
        return b.run()
    }
}

public class B1 {
    func f1() -> Int { return 0 }
    public func run() -> Int {
        return f1()
    }
}

public class C1: B1 {
    let x: Int
    init(x: Int) {
        self.x = x
    }
    override public func f1() -> Int {
        return x + 2
    }
}

let a: B1 = C1(x:1)
var cnt = 0
cnt += a.run()