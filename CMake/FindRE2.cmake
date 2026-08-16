find_path(RE2_INCLUDE_DIR NAMES re2/re2.h)
find_library(RE2_LIBRARY NAMES re2)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(
  RE2
  REQUIRED_VARS RE2_LIBRARY RE2_INCLUDE_DIR
)

if(RE2_FOUND AND NOT TARGET re2::re2)
  add_library(re2::re2 UNKNOWN IMPORTED)
  set_target_properties(re2::re2 PROPERTIES
    IMPORTED_LOCATION "${RE2_LIBRARY}"
    INTERFACE_INCLUDE_DIRECTORIES "${RE2_INCLUDE_DIR}"
  )
endif()

mark_as_advanced(RE2_INCLUDE_DIR RE2_LIBRARY)
