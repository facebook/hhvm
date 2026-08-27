# Fallback for distributions that ship RE2 without its CMake package config.
# HPHPFindLibs.cmake prefers find_package(re2 CONFIG), which already defines a
# correct re2::re2; this module is only consulted when that is unavailable.

find_package(PkgConfig QUIET)
if(PKG_CONFIG_FOUND)
  pkg_check_modules(PC_RE2 QUIET re2)
endif()

find_path(RE2_INCLUDE_DIR
  NAMES re2/re2.h
  HINTS ${PC_RE2_INCLUDE_DIRS}
)
find_library(RE2_LIBRARY
  NAMES re2
  HINTS ${PC_RE2_LIBRARY_DIRS}
)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(
  RE2
  REQUIRED_VARS RE2_LIBRARY RE2_INCLUDE_DIR
)

if(RE2_FOUND AND NOT TARGET re2::re2)
  # GLOBAL so the target stays usable from directories other than the one that
  # happened to run find_package() first.
  add_library(re2::re2 UNKNOWN IMPORTED GLOBAL)
  set_target_properties(re2::re2 PROPERTIES
    IMPORTED_LOCATION "${RE2_LIBRARY}"
    INTERFACE_INCLUDE_DIRECTORIES "${RE2_INCLUDE_DIR}"
  )
  # RE2 2023-and-later links against Abseil. Without the CONFIG package there
  # is no dependency graph to walk, so take the link line from re2.pc.
  if(PC_RE2_FOUND)
    set_property(TARGET re2::re2 APPEND PROPERTY
      INTERFACE_LINK_LIBRARIES ${PC_RE2_STATIC_LDFLAGS_OTHER})
    foreach(re2_dep ${PC_RE2_STATIC_LIBRARIES})
      if(NOT re2_dep STREQUAL "re2")
        set_property(TARGET re2::re2 APPEND PROPERTY
          INTERFACE_LINK_LIBRARIES "${re2_dep}")
      endif()
    endforeach()
    set_property(TARGET re2::re2 APPEND PROPERTY
      INTERFACE_LINK_DIRECTORIES ${PC_RE2_STATIC_LIBRARY_DIRS})
  endif()
endif()

mark_as_advanced(RE2_INCLUDE_DIR RE2_LIBRARY)
