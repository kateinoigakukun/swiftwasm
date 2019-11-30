// RUN: %target-run-simple-swift
// public protocol P {
//     func foo()
// }
// 
// public struct A: P {
//     public func foo() {}
// }
// 
// @_optimize(none)
// public func callFoo(_ t: P) { t.foo() }
// 
// public func callFooFromAny(_ any: Any) { callFoo(any as! P) }
// 
// callFooFromAny(A())

public struct Box<T> {}
public extension Box where T == Int {
    static func foo() {}
}

type(of: ((Box<Int>() as Any) as! Box<Int>)).foo()
