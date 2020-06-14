add_swift_host_library(swiftLTO STATIC
  LTO.cpp
  CrossModuleDFE.cpp)
target_link_libraries(swiftLTO PRIVATE
  swiftFrontend
  swiftIRGen
  swiftSILGen
  swiftSILOptimizer)
