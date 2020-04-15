# swift_build_support/products/swiftstdlib.py -------------------------*- python -*-
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
from . import swift
from ..cmake import CMakeOptions


class SwiftStdlib(product.Product):

    @classmethod
    def product_source_name(cls):
        return "swift"

    def __init__(self, args, toolchain, source_dir, build_dir):
        product.Product.__init__(self, args, toolchain, source_dir,
                                 build_dir)
        # Add any runtime sanitizer arguments.
        self.cmake_options.extend(self._runtime_sanitizer_flags)

        # Add any compiler vendor cmake flags.
        self.cmake_options.extend(self._compiler_vendor_flags)

        # Add any swift version related cmake flags.
        self.cmake_options.extend(self._version_flags)

        # Add benchmark specific flags.
        self.cmake_options.extend(self._benchmark_flags)

        # Generate the compile db.
        self.cmake_options.extend(self._compile_db_flags)

        # Add any exclusivity checking flags for stdlibcore.
        self.cmake_options.extend(self._stdlibcore_exclusivity_checking_flags)

        # Add experimental differentiable programming flag.
        self.cmake_options.extend(
            self._enable_experimental_differentiable_programming)

    @property
    def _runtime_sanitizer_flags(self):
        return swift.swift_runtime_sanitizer_flags(self.args)

    @property
    def _compiler_vendor_flags(self):
        return swift.swift_compiler_vendor_flags(self.args)

    @property
    def _version_flags(self):
        return swift.swift_version_flags(self.args)

    @property
    def _benchmark_flags(self):
        if not self.args.benchmark:
            return []

        onone_iters = self.args.benchmark_num_onone_iterations
        o_iters = self.args.benchmark_num_o_iterations
        return [
            ('SWIFT_BENCHMARK_NUM_ONONE_ITERATIONS', onone_iters),
            ('SWIFT_BENCHMARK_NUM_O_ITERATIONS', o_iters)
        ]

    @property
    def _compile_db_flags(self):
        return [('CMAKE_EXPORT_COMPILE_COMMANDS', True)]

    @property
    def _stdlibcore_exclusivity_checking_flags(self):
        return [('SWIFT_STDLIB_ENABLE_STDLIBCORE_EXCLUSIVITY_CHECKING:BOOL',
                 self.args.enable_stdlibcore_exclusivity_checking)]

    @property
    def _enable_experimental_differentiable_programming(self):
        return [('SWIFT_ENABLE_EXPERIMENTAL_DIFFERENTIABLE_PROGRAMMING:BOOL',
                 self.args.enable_experimental_differentiable_programming)]
