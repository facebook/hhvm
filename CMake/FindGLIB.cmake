find_path(GLIB_INCLUDE_DIR
          NAMES "glib.h"
          PATHS "/usr/include" "/usr/local/include"
          PATH_SUFFIXES "glib-2.0")
find_path(GLIB_CONFIG_INCLUDE_DIR
          NAMES "glibconfig.h"
          PATHS "/usr/lib64" "/usr/lib" "/usr/local/lib64" "/usr/local/lib" "/usr/lib/x86_64-linux-gnu" "/usr/lib/aarch64-linux-gnu" "/usr/lib/powerpc64le-linux-gnu/"
          PATH_SUFFIXES "glib-2.0/include")
include(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(GLIB DEFAULT_MSG
                                  GLIB_INCLUDE_DIR
                                  GLIB_CONFIG_INCLUDE_DIR)
mark_as_advanced(GLIB_INCLUDE_DIR
                 GLIB_CONFIG_INCLUDE_DIR)
