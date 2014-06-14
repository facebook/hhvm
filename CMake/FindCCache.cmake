find_program(CCACHE
             NAMES ccache)

# handle REQUIRED and QUIET options
include(FindPackageHandleStandardArgs)
if (CMAKE_VERSION LESS 2.8.3)
  find_package_handle_standard_args(CCache DEFAULT_MSG CCACHE)
else ()
  find_package_handle_standard_args(CCache REQUIRED_VARS CCACHE)
endif ()

mark_as_advanced(CCACHE)
