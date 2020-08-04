// RUN: %empty-directory(%t)
// RUN: echo 'print("Hello, World!")' >%t/main.swift
// RUN: cd %t

// RUN: %target-swift-frontend -emit-sib -emit-module-summary-path %t/main.swiftmodule.summary %t/main.swift
// RUN: test -f %t/main.swiftmodule.summary

// RUN: echo '%t/main.swift: { swiftmodule-summary: %t/foo.swiftmodule.summary }' >%t/filemap.yaml
// RUN: %target-swift-frontend -emit-sib -supplementary-output-file-map %t/filemap.yaml %t/main.swift
// RUN: test -f %t/foo.swiftmodule.summary
