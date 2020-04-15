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

# Add path for custom CMake modules.
list(APPEND CMAKE_MODULE_PATH
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules")
set(SWIFT_BUILT_STANDALONE FALSE)
if("${CMAKE_SOURCE_DIR}" STREQUAL "${CMAKE_CURRENT_SOURCE_DIR}")
  set(SWIFT_BUILT_STANDALONE TRUE)
endif()

if(SWIFT_BUILT_STANDALONE)
  project(Swift C CXX ASM)
endif()

get_filename_component(SWIFT_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR} REALPATH)
set(SWIFT_BINARY_DIR "${CMAKE_CURRENT_BINARY_DIR}")
set(SWIFT_CMAKE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules")
set(SWIFT_MAIN_INCLUDE_DIR "${SWIFT_SOURCE_DIR}/include")
set(SWIFT_INCLUDE_DIR "${CMAKE_CURRENT_BINARY_DIR}/include")

set(SWIFT_RUNTIME_OUTPUT_INTDIR "${CMAKE_BINARY_DIR}/${CMAKE_CFG_INTDIR}/bin")
set(SWIFT_LIBRARY_OUTPUT_INTDIR "${CMAKE_BINARY_DIR}/${CMAKE_CFG_INTDIR}/lib")
if("${SWIFT_NATIVE_SWIFT_TOOLS_PATH}" STREQUAL "")
  set(SWIFT_NATIVE_SWIFT_TOOLS_PATH "${SWIFT_RUNTIME_OUTPUT_INTDIR}")
endif()

# Include shared prologue
include(SwiftSharedCMakeInit)

# Add all of the subdirectories, where we actually do work.

if(SWIFT_BUILD_STDLIB OR SWIFT_INCLUDE_TOOLS)
  add_subdirectory(stdlib/public/SwiftShims)
endif()

if(SWIFT_INCLUDE_APINOTES)
  add_subdirectory(apinotes)
endif()

add_subdirectory(include)

if(SWIFT_INCLUDE_TOOLS)
  add_subdirectory(lib)

  # Always include this after including stdlib/!
  # Refer to the large comment above the add_subdirectory(stdlib) call.
  # https://bugs.swift.org/browse/SR-5975
  add_subdirectory(tools)
endif()

add_subdirectory(utils)

add_subdirectory(userdocs)

if(SWIFT_INCLUDE_TESTS)
  add_subdirectory(unittests)
endif()
if(SWIFT_INCLUDE_DOCS)
  add_subdirectory(docs)
endif()

add_subdirectory(cmake/modules)

swift_install_in_component(FILES "LICENSE.txt"
                           DESTINATION "share/swift"
                           COMPONENT license)

# Add a documentation target so that documentation shows up in the
# Xcode project.
if(XCODE)
  add_custom_target(Documentation
      SOURCES
        README.md
        docs)

  file(GLOB SWIFT_TOPLEVEL_HEADERS
      ${CMAKE_CURRENT_SOURCE_DIR}/include/swift${dir}/*.h
      ${CMAKE_CURRENT_SOURCE_DIR}/include/swift${dir}/*.td
      ${CMAKE_CURRENT_SOURCE_DIR}/include/swift${dir}/*.def)
  add_custom_target(Miscellaneous
      SOURCES ${SWIFT_TOPLEVEL_HEADERS})
endif()
