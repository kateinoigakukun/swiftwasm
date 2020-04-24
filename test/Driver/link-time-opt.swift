// RUN: %target-swiftc_driver -driver-print-jobs %S/../Inputs/empty.swift -lto=llvm | %FileCheck %s --check-prefix=CHECK-%target-os
// CHECK-macosx: bin/swift -frontend -emit-bc
// CHECK-macosx-NEXT: bin/ld {{.+}} -lto_library {{.+}}/lib/libLTO.dylib

// RUN: rm -rf %t
// RUN: %empty-directory(%t/thin)
// RUN: %empty-directory(%t/full)
// RUN: %empty-directory(%t/thin-static)
// RUN: %empty-directory(%t/full-static)

// RUN: %target-swiftc_driver %S/Inputs/lto/lib.swift -lto=llvm -emit-library -emit-module -module-name A -working-directory %t/thin
// RUN: %target-swiftc_driver %S/Inputs/lto/main.swift -L. -I. -lA -lto=llvm -working-directory %t/thin

// RUN: %target-swiftc_driver %S/Inputs/lto/lib.swift -lto=llvm-full -emit-library -emit-module -module-name A -working-directory %t/full
// RUN: %target-swiftc_driver %S/Inputs/lto/main.swift -L. -I. -lA -lto=llvm-full -working-directory %t/full

// RUN: %target-swiftc_driver %S/Inputs/lto/lib.swift -static -lto=llvm -emit-library -emit-module -module-name A -working-directory %t/thin-static
// RUN: %target-swiftc_driver %S/Inputs/lto/main.swift -L. -I. -lA -lto=llvm -working-directory %t/thin-static

// RUN: %target-swiftc_driver %S/Inputs/lto/lib.swift -static -lto=llvm-full -emit-library -emit-module -module-name A -working-directory %t/full-static
// RUN: %target-swiftc_driver %S/Inputs/lto/main.swift -L. -I. -lA -lto=llvm-full -working-directory %t/full-static
