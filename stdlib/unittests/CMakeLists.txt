include(AddSwiftUnittests)

if(SWIFT_INCLUDE_TOOLS AND SWIFT_BUILD_SDK_OVERLAY)
  # We can't link C++ unit tests unless we build the tools.
  add_subdirectory(runtime)
  add_subdirectory(Reflection)
endif()