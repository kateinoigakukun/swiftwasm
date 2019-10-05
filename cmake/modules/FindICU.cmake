# Find libicu's libraries

include(FindPackageHandleStandardArgs)

find_package(PkgConfig)

set(ICU_REQUIRED)
foreach(MODULE ${ICU_FIND_COMPONENTS})
  string(TOUPPER "${MODULE}" MODULE)
  string(TOLOWER "${MODULE}" module)
  list(APPEND ICU_REQUIRED
    ICU_${MODULE}_INCLUDE_DIRS ICU_${MODULE}_LIBRARIES)

  pkg_check_modules(PC_ICU_${MODULE} QUIET icu-${module})
  if(NOT PKG_CONFIG_FOUND)
    # PkgConfig doesn't exist on this system, so we manually provide hints via CMake.
    set(PC_ICU_${MODULE}_INCLUDE_DIRS "${ICU_${MODULE}_INCLUDE_DIRS}")
    set(PC_ICU_${MODULE}_LIBRARY_DIRS "${ICU_${MODULE}_LIBRARY_DIRS}")
  endif()

  find_path(ICU_${MODULE}_INCLUDE_DIRS unicode
    HINTS ${PC_ICU_${MODULE}_INCLUDE_DIRS})
  find_library(ICU_${MODULE}_LIBRARIES NAMES icu${module} ${ICU_${MODULE}_LIB_NAME}
    HINTS ${PC_ICU_${MODULE}_LIBRARY_DIRS})
endforeach()

foreach(sdk ANDROID;FREEBSD;LINUX;WINDOWS;HAIKU;WASM)
  foreach(MODULE ${ICU_FIND_COMPONENTS})
    string(TOUPPER "${MODULE}" MODULE)
    if("${SWIFT_${sdk}_${SWIFT_HOST_VARIANT_ARCH}_ICU_${MODULE}_INCLUDE}" STREQUAL "")
      set(SWIFT_${sdk}_${SWIFT_HOST_VARIANT_ARCH}_ICU_${MODULE}_INCLUDE ${ICU_${MODULE}_INCLUDE_DIRS} CACHE STRING "" FORCE)
    endif()
    if("${SWIFT_${sdk}_${SWIFT_HOST_VARIANT_ARCH}_ICU_${MODULE}}" STREQUAL "")
      set(SWIFT_${sdk}_${SWIFT_HOST_VARIANT_ARCH}_ICU_${MODULE} ${ICU_${MODULE}_LIBRARIES} CACHE STRING "" FORCE)
    endif()
  endforeach()
endforeach()

message("[katei in FindICU] ICU_I18N_INCLUDE_DIRS=${ICU_I18N_INCLUDE_DIRS}")
find_package_handle_standard_args(ICU DEFAULT_MSG ${ICU_REQUIRED})
mark_as_advanced(${ICU_REQUIRED})
