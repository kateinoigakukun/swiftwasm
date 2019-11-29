// RUN: %target-run-simple-swift
public protocol P {
    func foo()
}

public struct A: P {
    public func foo() {}
}

@_optimize(none)
public func callFoo(_ t: P) { t.foo() }

public func callFooFromAny(_ any: Any) { callFoo(any as! P) }

callFooFromAny(A())

