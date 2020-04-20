// REQUIRES: OS=macosx
// RUN: %target-swiftc_driver -driver-print-jobs %S/../Inputs/empty.swift -llvm-lto | %FileCheck %s
// CHECK: bin/swift -frontend -emit-bc
// CHECK-NEXT: bin/ld {{.+}} -lto_library {{.+}}/lib/libLTO.dylib

// RUN: %target-swiftc_driver %S/Inputs/lto/lib.swift -llvm-lto -emit-library -emit-module -module-name A
// RUN: %target-swiftc_driver %S/Inputs/lto/main.swift -L. -I. -lA -llvm-lto
