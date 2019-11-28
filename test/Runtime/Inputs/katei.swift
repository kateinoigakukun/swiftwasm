// RUN: %target-run-simple-swift | %FileCheck %s

public class A<T> {
    let value: T
    init(value: T) { self.value = value }
}

A<Int>(value: 1)

print(1)
// CHECK: 1
