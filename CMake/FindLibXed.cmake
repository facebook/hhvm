find_path(LibXed_INCLUDE_DIR xed-interface.h)
find_library(LibXed_LIBRARY NAMES xed)

if (LibXed_INCLUDE_DIR AND LibXed_LIBRARY)
  include(CheckCSourceRuns)
  set(CMAKE_REQUIRED_INCLUDES ${LibXed_INCLUDE_DIR})
  set(CMAKE_REQUIRED_LIBRARIES ${LibXed_LIBRARY})
  check_c_source_runs("
    #include <xed-interface.h>
    #include <stdlib.h>
    #include <string.h>
    int main() {
      const char* version = xed_get_version();
      if(strlen(version) < 2) {
        return -1;
      }
      // Format is Vxx.yy*
      // we're only interested in the major version number
      return !(atoi(version + 1) >= 10);
    }"
    LibXed_GOOD_VERSION)
  set(CMAKE_REQUIRED_INCLUDES)
  set(CMAKE_REQUIRED_LIBRARIES)

  if (NOT LibXed_GOOD_VERSION)
    message(STATUS "LibXed version check failed")
  endif()
endif()

include(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(LibXed
                                  DEFAULT_MSG
                                  LibXed_LIBRARY
                                  LibXed_INCLUDE_DIR
                                  LibXed_GOOD_VERSION)
if (NOT LibXed_FOUND)
  message(STATUS "Using third-party bundled LibXed")
else()
  message(STATUS "Using LibXed from: ${LibXed_LIBRARY}")
endif()

mark_as_advanced(LibXed_INCLUDE_DIR LibXed_LIBRARY)
