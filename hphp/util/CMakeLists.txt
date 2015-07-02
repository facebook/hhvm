set(CXX_SOURCES)
auto_sources(files "*.cpp" "RECURSE" "${CMAKE_CURRENT_SOURCE_DIR}")
list(APPEND CXX_SOURCES ${files})

set(ASM_SOURCES)
auto_sources(asmfiles "*.S" "RECURSE" "${CMAKE_CURRENT_SOURCE_DIR}")
list(APPEND ASM_SOURCES ${asmfiles})

# Disable Channeled JSON until we're sure it's going to stick
# sgolemon(2014-02-19)
HHVM_REMOVE_MATCHES_FROM_LISTS(CXX_SOURCES MATCHES "/test/" "channeled-json-")

add_custom_command(
  OUTPUT "${CMAKE_CURRENT_SOURCE_DIR}/../hphp-repo-schema.h"
         "${CMAKE_CURRENT_SOURCE_DIR}/../hphp-build-info.cpp"
  COMMAND "${CMAKE_CURRENT_SOURCE_DIR}/generate-buildinfo.sh"
  DEPENDS ${CXX_SOURCES}
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../
  COMMENT "Generating Repo Schema ID and Compiler ID"
  VERBATIM)

## Disabling numa support (for now) since distro
## libnuma packages don't always have numa_init available
## Find a better way to make sure numa initialization
## happens on time. -sgolemon (2013-10-16)

#find_package(LibNuma)
#if (LIBNUMA_INCLUDE_DIRS)
#  add_definitions("-DHAVE_NUMA=1")
#  include_directories(${LIBNUMA_INCLUDE_DIRS})
#endif()

add_library(hphp_util STATIC ${CXX_SOURCES} ${ASM_SOURCES}
            "${CMAKE_CURRENT_SOURCE_DIR}/../hphp-repo-schema.h"
            "${CMAKE_CURRENT_SOURCE_DIR}/../hphp-build-info.cpp")
if (ENABLE_COTIRE)
  cotire(hphp_util)
endif()

#if (LIBNUMA_LIBRARIES)
#  target_link_libraries(hphp_util ${LIBNUMA_LIBRARIES})
#endif()
if (ENABLE_ASYNC_MYSQL)
  add_dependencies(hphp_util webscalesqlclient)
endif ()

auto_sources(files "*.h" "RECURSE")
HHVM_PUBLIC_HEADERS(util ${files})
