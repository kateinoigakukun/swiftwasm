// RUN: %empty-directory(%t)
// RUN: %target-build-swift %s -module-name main -o %t/a.out
// RUN: %target-codesign %t/a.out
// RUN: %target-run %t/a.out

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
// enum EG<T, U> { case a }
// 
// extension EG {
//   struct NestedSG<V> { }
// }
// 
// assert(EG<Int, String>.NestedSG<Double>.self ==
//   _typeByName("4main2EGO8NestedSGVySiSS_SdG")!)
// public class ResettableValue<Value> {
//   public init(_ value: Value) {
//     self.value = value
//   }
// 
//   public var value: Value
// }
// 
// var hashIntoImpl = ResettableValue<(Int) -> Void>({ _ = $0 })
// hashIntoImpl.value(1)
// class C<T> {
//     var value: T
//     init(value: T) { self.value = value }
// }
// 
// func callVTable<T>(_ instance: C<T>) {
//     _ = instance.value
// }
// 
// callVTable(C<Int>(value: 1))

// protocol P1 { }
// protocol P2 { }
// protocol P3 { }
// struct ConformsToP3: P3 { }
// struct OuterTwoParams<T: P1, U: P2> {}
// struct ConformsToP1AndP2 : P1, P2 { }
// extension OuterTwoParams where U == T {
//   struct InnerEqualParams<V: P3> {
//     var x: T
//     var y: U
//     var z: V
//   }
// }
// let value = OuterTwoParams.InnerEqualParams(x: ConformsToP1AndP2(),
//                                             y: ConformsToP1AndP2(),
//                                             z: ConformsToP3())
// var output = ""
// dump(value, to: &output)
// 
// let expected =
//   "▿ (extension in main):main.OuterTwoParams<main.ConformsToP1AndP2, main.ConformsToP1AndP2>.InnerEqualParams<main.ConformsToP3>\n" +
//   "  - x: main.ConformsToP1AndP2\n" +
//   "  - y: main.ConformsToP1AndP2\n" +
//   "  - z: main.ConformsToP3\n"
// assert(output == expected)
public final class LifetimeTracked {
  public init(_ value: Int, identity: Int = 0) {
    LifetimeTracked.instances += 1
    LifetimeTracked._nextSerialNumber += 1
    serialNumber = LifetimeTracked._nextSerialNumber
    self.value = value
    self.identity = identity
  }

  deinit {
    assert(serialNumber > 0, "double destruction!")
    LifetimeTracked.instances -= 1
    serialNumber = -serialNumber
  }

  public static var instances: Int = 0
  internal static var _nextSerialNumber = 0

  public let value: Int
  public var identity: Int
  public var serialNumber: Int = 0
}

extension LifetimeTracked : Equatable {
  public static func == (x: LifetimeTracked, y: LifetimeTracked) -> Bool {
    return x.value == y.value
  }
}

extension LifetimeTracked : Hashable {
  public var hashValue: Int {
    return value
  }
  public func hash(into hasher: inout Hasher) {
    hasher.combine(value)
  }
}

extension LifetimeTracked : Strideable {
  public func distance(to other: LifetimeTracked) -> Int {
    return self.value.distance(to: other.value)
  }

  public func advanced(by n: Int) -> LifetimeTracked {
    return LifetimeTracked(self.value.advanced(by: n))
  }
}

extension LifetimeTracked : CustomStringConvertible {
  public var description: String {
    assert(serialNumber > 0, "dead Tracked!")
    return value.description
  }
}

public func < (x: LifetimeTracked, y: LifetimeTracked) -> Bool {
  return x.value < y.value
}
public func expectEqual<T : Equatable>(_ expected: T, _ actual: T,
  _ message: @autoclosure () -> String = "",
  showFrame: Bool = true,
  file: String = #file, line: UInt = #line) {
      if expected != actual {
          fatalError(message())
      }
}
final class C<T> {
  var x: Int
  var y: LifetimeTracked?
  var z: T
  let immutable: String
  private(set) var secretlyMutable: String

  var computed: T {
    get {
      return z
    }
    set {
      z = newValue
    }
  }

  init(x: Int, y: LifetimeTracked?, z: T) {
    self.x = x
    self.y = y
    self.z = z
    self.immutable = "\(x) \(y) \(z)"
    self.secretlyMutable = immutable
  }
}

struct Point: Equatable {
  var x: Double
  var y: Double
  var trackLifetime = LifetimeTracked(123)
  let hypotenuse: Double
  private(set) var secretlyMutableHypotenuse: Double
  
  init(x: Double, y: Double) {
    self.x = x
    self.y = y
    hypotenuse = x*x + y*y
    secretlyMutableHypotenuse = x*x + y*y
  }
  
  static func ==(a: Point, b: Point) -> Bool {
    return a.x == b.x && a.y == b.y
  }
}

struct S<T: Equatable>: Equatable {
  var x: Int
  var y: LifetimeTracked?
  var z: T
  var p: Point
  var c: C<T>
  
  static func ==(a: S, b: S) -> Bool {
    return a.x == b.x
      && a.y === b.y
      && a.z == b.z
      && a.p == b.p
      && a.c === b.c
  }
}

final class ComputedA {
  var readOnly: ComputedB { fatalError() }
  var nonmutating: ComputedB {
    get { fatalError() }
    set { fatalError() }
  }
  var reabstracted: () -> () = {}
}

struct ComputedB {
  var readOnly: ComputedA { fatalError() }
  var mutating: ComputedA { 
    get { fatalError() }
    set { fatalError() }
  }
  var nonmutating: ComputedA {
    get { fatalError() }
    nonmutating set { fatalError() }
  }
  var reabstracted: () -> () = {}
}

typealias Tuple<T: Equatable, U> = (S<T>, C<U>)
//  for _ in 1...2 {
//    let s_x = (\S<Int>.x as AnyKeyPath) as! WritableKeyPath<S<Int>, Int>
//    let s_y = (\S<Int>.y as AnyKeyPath) as! WritableKeyPath<S<Int>, LifetimeTracked?>
//    let s_z = (\S<Int>.z as AnyKeyPath) as! WritableKeyPath<S<Int>, Int>
//    let s_p = (\S<Int>.p as AnyKeyPath) as! WritableKeyPath<S<Int>, Point>
//    let s_p_x = (\S<Int>.p.x as AnyKeyPath) as! WritableKeyPath<S<Int>, Double>
//    let s_p_y = (\S<Int>.p.y as AnyKeyPath) as! WritableKeyPath<S<Int>, Double>
//    let s_c = (\S<Int>.c as AnyKeyPath) as! WritableKeyPath<S<Int>, C<Int>>
//    let s_c_x = (\S<Int>.c.x as AnyKeyPath) as! ReferenceWritableKeyPath<S<Int>, Int>
//
//    let t_0s = (\Tuple<Int, Int>.0 as AnyKeyPath) as! WritableKeyPath<Tuple<Int, Int>, S<Int>>
//    let t_1c = (\Tuple<Int, Int>.1 as AnyKeyPath) as! WritableKeyPath<Tuple<Int, Int>, C<Int>>
//    let t_0s_x = (\Tuple<Int, Int>.0.x as AnyKeyPath) as! WritableKeyPath<Tuple<Int, Int>, Int>
//    let t_0s_p_hypotenuse = (\Tuple<Int, Int>.0.p.hypotenuse as AnyKeyPath) as! KeyPath<Tuple<Int, Int>, Double>
//    let t_1c_x = (\Tuple<Int, Int>.1.x as AnyKeyPath) as! ReferenceWritableKeyPath<Tuple<Int, Int>, Int>
//    let t_1c_immutable = (\Tuple<Int, Int>.1.immutable as AnyKeyPath) as! KeyPath<Tuple<Int, Int>, String>
//
//    let c_x = (\C<Int>.x as AnyKeyPath) as! ReferenceWritableKeyPath<C<Int>, Int>
//    let s_c_x_2 = s_c.appending(path: c_x)
//    expectEqual(s_c_x, s_c_x_2)
//    expectEqual(s_c_x_2, s_c_x)
//    expectEqual(s_c_x.hashValue, s_c_x_2.hashValue)
//
//    let t_1c_x_2 = t_1c.appending(path: c_x)
//
//    expectEqual(t_1c_x, t_1c_x_2)
//    expectEqual(t_1c_x_2, t_1c_x)
//    expectEqual(t_1c_x.hashValue, t_1c_x_2.hashValue)
//
//    let point_x = (\Point.x as AnyKeyPath) as! WritableKeyPath<Point, Double>
//    let point_y = (\Point.y as AnyKeyPath) as! WritableKeyPath<Point, Double>
//
//    let s_p_x_2 = s_p.appending(path: point_x)
//    let s_p_y_2 = s_p.appending(path: point_y)
//
//    expectEqual(s_p_x, s_p_x_2)
//    expectEqual(s_p_x_2, s_p_x)
//    expectEqual(s_p_x_2.hashValue, s_p_x.hashValue)
//    expectEqual(s_p_y, s_p_y_2)
//    expectEqual(s_p_y_2, s_p_y)
//    expectEqual(s_p_y_2.hashValue, s_p_y.hashValue)
//
    let ca_readOnly = (\ComputedA.readOnly as AnyKeyPath) as! KeyPath<ComputedA, ComputedB>
//    let ca_nonmutating = (\ComputedA.nonmutating as AnyKeyPath) as! ReferenceWritableKeyPath<ComputedA, ComputedB>
//    let ca_reabstracted = (\ComputedA.reabstracted as AnyKeyPath) as! ReferenceWritableKeyPath<ComputedA, () -> ()>
//
//    let cb_readOnly = (\ComputedB.readOnly as AnyKeyPath) as! KeyPath<ComputedB, ComputedA>
//    let cb_mutating = (\ComputedB.mutating as AnyKeyPath) as! WritableKeyPath<ComputedB, ComputedA>
//    let cb_nonmutating = (\ComputedB.nonmutating as AnyKeyPath) as! ReferenceWritableKeyPath<ComputedB, ComputedA>
//    let cb_reabstracted = (\ComputedB.reabstracted as AnyKeyPath) as! WritableKeyPath<ComputedB, () -> ()>
//
//    let ca_readOnly_mutating = (\ComputedA.readOnly.mutating as AnyKeyPath) as! KeyPath<ComputedA, ComputedA>
//    let cb_mutating_readOnly = (\ComputedB.mutating.readOnly as AnyKeyPath) as! KeyPath<ComputedB, ComputedB>
//    let ca_readOnly_nonmutating = (\ComputedA.readOnly.nonmutating as AnyKeyPath) as! ReferenceWritableKeyPath<ComputedA, ComputedA>
//    let cb_readOnly_reabstracted = (\ComputedB.readOnly.reabstracted as AnyKeyPath) as! ReferenceWritableKeyPath<ComputedB, () -> ()>
//
//    let ca_readOnly_mutating2 = ca_readOnly.appending(path: cb_mutating)
//    expectEqual(ca_readOnly_mutating, ca_readOnly_mutating2)
//    expectEqual(ca_readOnly_mutating2, ca_readOnly_mutating)
//    expectEqual(ca_readOnly_mutating.hashValue, ca_readOnly_mutating2.hashValue)
//
//    let cb_mutating_readOnly2 = cb_mutating.appending(path: ca_readOnly)
//    expectEqual(cb_mutating_readOnly, cb_mutating_readOnly2)
//    expectEqual(cb_mutating_readOnly2, cb_mutating_readOnly)
//    expectEqual(cb_mutating_readOnly.hashValue, cb_mutating_readOnly2.hashValue)
//
//    let ca_readOnly_nonmutating2 = ca_readOnly.appending(path: cb_nonmutating)
//    expectEqual(ca_readOnly_nonmutating, ca_readOnly_nonmutating2)
//    expectEqual(ca_readOnly_nonmutating2, ca_readOnly_nonmutating)
//    expectEqual(ca_readOnly_nonmutating.hashValue,
//                ca_readOnly_nonmutating2.hashValue)
//
//    let cb_readOnly_reabstracted2 = cb_readOnly.appending(path: ca_reabstracted)
//    expectEqual(cb_readOnly_reabstracted,
//                cb_readOnly_reabstracted2)
//    expectEqual(cb_readOnly_reabstracted2,
//                cb_readOnly_reabstracted)
//    expectEqual(cb_readOnly_reabstracted2.hashValue,
//                cb_readOnly_reabstracted.hashValue)
//  }
