cmake_minimum_required(VERSION 3.12.4)

# TODO: Fix RPATH usage to be CMP0068 compliant
# Disable Policy CMP0068 for CMake 3.9
# rdar://37725888
if(POLICY CMP0068)
  cmake_policy(SET CMP0068 OLD)
endif()

# Honour CMAKE_CXX_STANDARD in try_compile(), needed for check_cxx_native_regex.
if(POLICY CMP0067)
  cmake_policy(SET CMP0067 NEW)
endif()
# TODO(compnerd) once we have a newer CMake we should be able to use the new
# `Swift` LANGUAGE support in CMake to simplify the build.  For now, just add
# the project so that it is possible to start working on extracting the Swift
# standard library from the Swift compiler build.
project(swift-stdlib LANGUAGES C CXX)

# Add path for custom CMake modules.
list(APPEND CMAKE_MODULE_PATH
    "${CMAKE_CURRENT_SOURCE_DIR}/../cmake/modules")
list(APPEND CMAKE_MODULE_PATH
  "${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules")

set(SWIFT_BUILT_STANDALONE TRUE)
set(SWIFT_BUILD_STDLIB TRUE)

get_filename_component(SWIFT_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/.. REALPATH)
set(SWIFT_STDLIB_TOOLS_DIR "${CMAKE_CURRENT_BINARY_DIR}/bin")
set(SWIFT_BINARY_DIR "${SWIFT_COMPILER_BUILD_DIR}")
set(SWIFT_CMAKE_DIR "${SWIFT_SOURCE_DIR}/cmake/modules")
set(SWIFT_MAIN_INCLUDE_DIR "${SWIFT_SOURCE_DIR}/include")
set(SWIFT_INCLUDE_DIR "${CMAKE_CURRENT_BINARY_DIR}/include")

set(SWIFT_RUNTIME_OUTPUT_INTDIR "${CMAKE_BINARY_DIR}/${CMAKE_CFG_INTDIR}/bin")
set(SWIFT_LIBRARY_OUTPUT_INTDIR "${CMAKE_BINARY_DIR}/${CMAKE_CFG_INTDIR}/lib")

include(SwiftSharedCMakeInit)
include(AddSwiftStdlib)

# Create convenience targets for the Swift standard library.

# NOTE(compnerd) this will pass the *build* configuration to the *host*
# libraries.  Explicitly indicate to CMake that it should **NOT** track the
# implicit language runtimes.  This can go away once we migrate to an external
# project with its own configure with the CMAKE_SYSTEM_NAME set rather than
# using the custom cross-compilation solution
set(CMAKE_C_IMPLICIT_LINK_LIBRARIES "")
set(CMAKE_CXX_IMPLICIT_LINK_LIBRARIES "")

set(CMAKE_C_IMPLICIT_LINK_DIRECTORIES "")
set(CMAKE_CXX_IMPLICIT_LINK_DIRECTORIES "")

if(SWIFT_BUILD_RUNTIME_WITH_HOST_COMPILER)
  if(NOT "${CMAKE_C_COMPILER_ID}" MATCHES "Clang")
    message(FATAL_ERROR "Building the swift runtime is not supported with ${CMAKE_C_COMPILER_ID}. Use the just-built clang instead.")
  else()
    message(WARNING "Building the swift runtime using the host compiler, and not the just-built clang.")
  endif()
else()
  # If we use Clang-cl or MSVC, CMake provides default compiler and linker flags that are incompatible
  # with the frontend of Clang or Clang++.
  if(SWIFT_COMPILER_IS_MSVC_LIKE)
    set(CMAKE_CXX_COMPILER "${SWIFT_NATIVE_LLVM_TOOLS_PATH}/clang-cl")
    set(CMAKE_C_COMPILER "${SWIFT_NATIVE_LLVM_TOOLS_PATH}/clang-cl")
  else()
    set(CMAKE_CXX_COMPILER "${SWIFT_NATIVE_LLVM_TOOLS_PATH}/clang++")
    set(CMAKE_C_COMPILER "${SWIFT_NATIVE_LLVM_TOOLS_PATH}/clang")
  endif()

  if(CMAKE_C_COMPILER_LAUNCHER MATCHES ".*distcc")
    set(CMAKE_C_COMPILER_LAUNCHER "")
  endif()
  if(CMAKE_CXX_COMPILER_LAUNCHER MATCHES ".*distcc")
    set(CMAKE_CXX_COMPILER_LAUNCHER "")
  endif()

  # The sanitizers require using the same version of the compiler for
  # everything and there are various places where we link runtime code with
  # code built by the host compiler. Disable sanitizers for the runtime for
  # now.
  add_compile_options(-fno-sanitize=all)
endif()

# Do not enforce checks for LLVM's ABI-breaking build settings.
# The Swift runtime uses some header-only code from LLVM's ADT classes,
# but we do not want to link libSupport into the runtime. These checks rely
# on the presence of symbols in libSupport to identify how the code was
# built and cause link failures for mismatches. Without linking that library,
# we get link failures regardless, so instead, this just disables the checks.
if(CMAKE_VERSION VERSION_LESS 3.12)
  append("-DLLVM_DISABLE_ABI_BREAKING_CHECKS_ENFORCING=1" CMAKE_C_FLAGS CMAKE_CXX_FLAGS)
else()
  add_compile_definitions(LLVM_DISABLE_ABI_BREAKING_CHECKS_ENFORCING=1)
endif()

set(SWIFT_STDLIB_LIBRARY_BUILD_TYPES)
if(SWIFT_BUILD_DYNAMIC_STDLIB)
  list(APPEND SWIFT_STDLIB_LIBRARY_BUILD_TYPES SHARED)
endif()
if(SWIFT_BUILD_STATIC_STDLIB)
  list_intersect("${SWIFT_APPLE_PLATFORMS}" "${SWIFT_SDKS}" building_darwin_sdks)
  if(building_darwin_sdks)
    message(SEND_ERROR "cannot build static standard library for Darwin SDKs")
  else()
    list(APPEND SWIFT_STDLIB_LIBRARY_BUILD_TYPES STATIC)
  endif()
endif()

function(swift_create_stdlib_targets name variant define_all_alias)
  if(NOT variant STREQUAL "")
    set(variant "-${variant}")
  endif()

  if(define_all_alias)
    add_custom_target(${name}${variant}-all)
    set_target_properties(${name}${variant}-all
      PROPERTIES
      FOLDER "Swift libraries/Aggregate")
  endif()

  foreach(sdk ${SWIFT_SDKS})
    add_custom_target(${name}-${SWIFT_SDK_${sdk}_LIB_SUBDIR}${variant})
    set_target_properties(${name}-${SWIFT_SDK_${sdk}_LIB_SUBDIR}${variant}
      PROPERTIES
      FOLDER "Swift libraries/Aggregate")

    foreach(arch ${SWIFT_SDK_${sdk}_ARCHITECTURES})
      set(target_variant -${SWIFT_SDK_${sdk}_LIB_SUBDIR}-${arch})

      add_custom_target(${name}${target_variant}${variant})
      set_target_properties(${name}${target_variant}${variant}
        PROPERTIES
        FOLDER "Swift libraries/Aggregate")
      if(define_all_alias)
        add_dependencies(${name}${variant}-all
          ${name}${target_variant}${variant})
      endif()
      add_dependencies(${name}-${SWIFT_SDK_${sdk}_LIB_SUBDIR}${variant}
        ${name}${target_variant}${variant})
    endforeach()
  endforeach()

  if(NOT define_all_alias)
    set(ALL_keyword ALL)
  endif()
  add_custom_target(${name}${variant}
    ${ALL_keyword}
    DEPENDS
    ${name}${SWIFT_PRIMARY_VARIANT_SUFFIX}${variant})
  set_target_properties(${name}${variant}
    PROPERTIES
    FOLDER "Swift libraries/Aggregate")
endfunction()

swift_create_stdlib_targets("swift-stdlib" "" TRUE)
if(SWIFT_STDLIB_ENABLE_SIB_TARGETS)
  swift_create_stdlib_targets("swift-stdlib" "sib" TRUE)
  swift_create_stdlib_targets("swift-stdlib" "sibopt" TRUE)
  swift_create_stdlib_targets("swift-stdlib" "sibgen" TRUE)
endif()
swift_create_stdlib_targets("swift-test-stdlib" "" FALSE)

function(_swift_symlink name)
  set(options IS_DIRECTORY)
  set(oneValueArgs SOURCE DESTINATION COMMENT)
  cmake_parse_arguments(SCB
    "${options}"
    "${oneValueArgs}"
    ""
    ${ARGN})
  
  if("${CMAKE_SYSTEM_NAME}" STREQUAL "Windows")
    if(SCB_IS_DIRECTORY)
      set(cmake_symlink_option "copy_directory")
    else()
      set(cmake_symlink_option "copy_if_different")
    endif()
  else()
      set(cmake_symlink_option "create_symlink")
  endif()
  
  add_custom_target("${name}"
    COMMAND
      "${CMAKE_COMMAND}" "-E" "${cmake_symlink_option}"
      "${SCB_SOURCE}"
      "${SCB_DESTINATION}"
    COMMENT "${SCB_COMMENT}")
endfunction()

add_subdirectory(include)

_swift_symlink(link_apinotes
  IS_DIRECTORY
  SOURCE "${SWIFT_COMPILER_BUILD_DIR}/lib/swift/apinotes"
  DESTINATION "${CMAKE_CURRENT_BINARY_DIR}/lib/swift/apinotes"
)

_swift_symlink(link_clang_lib
  IS_DIRECTORY
  SOURCE "${SWIFT_COMPILER_BUILD_DIR}/lib/swift/clang"
  DESTINATION "${CMAKE_CURRENT_BINARY_DIR}/lib/swift/clang"
)

_swift_symlink(link_shims
  IS_DIRECTORY
  SOURCE "${SWIFT_COMPILER_BUILD_DIR}/lib/swift/shims"
  DESTINATION "${CMAKE_CURRENT_BINARY_DIR}/lib/swift/shims"
)

add_custom_target(link_target_sdks)
foreach(sdk ${SWIFT_SDKS})
  _swift_symlink("link_target_sdk_${sdk}"
    IS_DIRECTORY
    SOURCE "${SWIFT_LIBRARY_OUTPUT_INTDIR}/swift/${SWIFT_SDK_${sdk}_LIB_SUBDIR}"
    DESTINATION "${SWIFT_COMPILER_BUILD_DIR}/lib/swift/${SWIFT_SDK_${sdk}_LIB_SUBDIR}")
  add_dependencies(link_target_sdks "link_target_sdk_${sdk}")
endforeach()

# FIXME: Include the toolchain directory before the public directory. Otherwise
# the clang resource directory symlink stops installing correctly.
add_subdirectory(toolchain)
add_subdirectory(public)
add_subdirectory(private)
add_subdirectory(tools)

if ("${CMAKE_SYSTEM_NAME}" STREQUAL "Darwin")
  if(SWIFT_BUILD_PERF_TESTSUITE)
    add_subdirectory("${SWIFT_SOURCE_DIR}/benchmark" "${CMAKE_CURRENT_BINARY_DIR}/benchmark")
  endif()
  if(SWIFT_BUILD_EXTERNAL_PERF_TESTSUITE)
    include(SwiftExternalBenchmarkBuild)
    add_external_benchmark_suite()
  endif()
endif()

if(SWIFT_INCLUDE_TESTS)
  add_subdirectory("${SWIFT_SOURCE_DIR}/test" "${CMAKE_CURRENT_BINARY_DIR}/test")
endif()
