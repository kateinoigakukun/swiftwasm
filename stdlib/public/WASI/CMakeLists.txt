add_swift_target_library(swiftWasiPthread ${SWIFT_STDLIB_LIBRARY_BUILD_TYPES}
  Pthread.cpp
  INSTALL_IN_COMPONENT stdlib)
