# swift_build_support/products/swift.py -------------------------*- python -*-
#
# This source file is part of the Swift.org open source project
#
# Copyright (c) 2014 - 2017 Apple Inc. and the Swift project authors
# Licensed under Apache License v2.0 with Runtime Library Exception
#
# See https://swift.org/LICENSE.txt for license information
# See https://swift.org/CONTRIBUTORS.txt for the list of Swift project authors
#
# ----------------------------------------------------------------------------

from . import product
from ..cmake import CMakeOptions

def swift_runtime_sanitizer_flags(args):
    sanitizer_list = []
    if args.enable_tsan_runtime:
        sanitizer_list += ['Thread']
    if len(sanitizer_list) == 0:
        return []
    return [('SWIFT_RUNTIME_USE_SANITIZERS', ';'.join(sanitizer_list))]

def swift_compiler_vendor_flags(args):
    if args.compiler_vendor == "none":
        return []

    if args.compiler_vendor != "apple":
        raise RuntimeError("Unknown compiler vendor?! Was build-script \
ted without updating swift.py?")

    swift_compiler_version = ""
    if args.swift_compiler_version is not None:
        swift_compiler_version = args.swift_compiler_version

    return [
        ('SWIFT_VENDOR', 'Apple'),
        ('SWIFT_VENDOR_UTI', 'com.apple.compilers.llvm.swift'),

        # This has a default of 3.0, so it should be safe to use here.
        ('SWIFT_VERSION', str(args.swift_user_visible_version)),

        # FIXME: We are matching build-script-impl here. But it seems like
        # bit rot since this flag is specified in another place with the
        # exact same value in build-script-impl.
        ('SWIFT_COMPILER_VERSION', str(swift_compiler_version)),
    ]

def swift_version_flags(args):
    r = CMakeOptions()
    if args.swift_compiler_version is not None:
        swift_compiler_version = args.swift_compiler_version
        r.define('SWIFT_COMPILER_VERSION', str(swift_compiler_version))
    if args.clang_compiler_version is not None:
        clang_compiler_version = args.clang_compiler_version
        r.define('CLANG_COMPILER_VERSION', str(clang_compiler_version))
    return r

class Swift(product.Product):

    def __init__(self, args, toolchain, source_dir, build_dir):
        product.Product.__init__(self, args, toolchain, source_dir,
                                 build_dir)
        # Add any runtime sanitizer arguments.
        self.cmake_options.extend(self._runtime_sanitizer_flags)

        # Add any compiler vendor cmake flags.
        self.cmake_options.extend(self._compiler_vendor_flags)

        # Add any swift version related cmake flags.
        self.cmake_options.extend(self._version_flags)

        # Generate the compile db.
        self.cmake_options.extend(self._compile_db_flags)

        # Add the flag if we are supposed to force the typechecker to compile
        # with optimization.
        self.cmake_options.extend(self._force_optimized_typechecker_flags)

    @property
    def _runtime_sanitizer_flags(self):
        return swift_runtime_sanitizer_flags(self.args)

    @property
    def _compiler_vendor_flags(self):
        return swift_compiler_vendor_flags(self.args)

    @property
    def _version_flags(self):
        return swift_version_flags(self.args)

    @property
    def _compile_db_flags(self):
        return [('CMAKE_EXPORT_COMPILE_COMMANDS', True)]

    @property
    def _force_optimized_typechecker_flags(self):
        return [('SWIFT_FORCE_OPTIMIZED_TYPECHECKER:BOOL',
                 self.args.force_optimized_typechecker)]
