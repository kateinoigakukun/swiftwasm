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

// public struct Box<T> {}
// public extension Box where T == Int {
//     static func foo() {}
// }
// 
// type(of: ((Box<Int>() as Any) as! Box<Int>)).foo()

// public protocol _P {}
// public protocol P: _P {
//     associatedtype X
//     func foo(_: X)
// }
// 
// 
// @_optimize(none)
// public func callFoo<T>(_ t: T, _ x: T.X) where T: P {
//     t.foo(x)
// }
// 
// func run() {
//     let any: _P = S<Int>()
//     callFoo(any as! S<Int>, 1)
// }
// 
// run()

public class S<Y> {
    let value: Y

    @_optimize(none)
    public init(value: Y) { self.value = value }
}

S(value: 1)

