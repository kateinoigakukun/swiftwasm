// RUN: %target-run-simple-swift
// public protocol P0 {
//     func foo0()
// }
// 
// public struct A0: P0 {
//     public func foo0() {}
// }
// 
// @_optimize(none)
// public func callFoo0(_ t: P0) { t.foo0() }
// 
// public func callFooFromAny0(_ any: Any) { callFoo0(any as! P0) }
// 
// callFooFromAny0(A0())
// 
// public struct Box0<T> {}
// public extension Box0 where T == Int {
//     static func foo1() {}
// }
// 
// type(of: ((Box0<Int>() as Any) as! Box0<Int>)).foo1()
// 
// public protocol _P {}
// public protocol P1: _P {
//     associatedtype X
//     func foo1(_: X)
// }
// 
// public protocol P2 {
//     associatedtype X1
//     static func foo2()
//     init(value: X1)
// }
// 
// 
// @_optimize(none)
// public func callFoo1<T>(_ t: T, _ x: T.X1) where T: P2 {
//     type(of: t).foo2()
// }
// 
// public final class S1<Y>: P2 {
//     let value: Y
// 
//     @_optimize(none)
//     public init(value: Y) { self.value = value }
//     public static func foo2() {}
// }
// 
// 
// @_optimize(none)
// public func foo<T: P2>(_ type: T.Type, value: T.X1) {
//     type.foo2()
//     _ = T(value: value)
// }
// 
// foo(S1<Int>.self, value: 1)
// 
// _ = ""
print("")
