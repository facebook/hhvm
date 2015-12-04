set(CXX_SOURCES)
auto_sources(files "*.cpp" "RECURSE" "${CMAKE_CURRENT_SOURCE_DIR}")
list(APPEND CXX_SOURCES ${files})

set(ASM_SOURCES)
auto_sources(files "*.S" "RECURSE" "${CMAKE_CURRENT_SOURCE_DIR}")
list(APPEND ASM_SOURCES ${files})

set(HEADER_SOURCES)
auto_sources(files "*.h" "RECURSE" "${CMAKE_CURRENT_SOURCE_DIR}")
list(APPEND HEADER_SOURCES ${files})

# Disable Channeled JSON until we're sure it's going to stick
# sgolemon(2014-02-19)
HHVM_REMOVE_MATCHES_FROM_LISTS(ASM_SOURCES CXX_SOURCES HEADER_SOURCES
  MATCHES "/test/" "channeled-json-")

if (NOT MSVC)
  list(REMOVE_ITEM CXX_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/portability/fnmatch.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/portability/glob.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/portability/rand_r.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/portability/strfmon.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/portability/strptime.cpp
  )
  list(REMOVE_ITEM HEADER_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/portability/fnmatch.h
    ${CMAKE_CURRENT_SOURCE_DIR}/portability/glob.h
    ${CMAKE_CURRENT_SOURCE_DIR}/portability/rand_r.h
    ${CMAKE_CURRENT_SOURCE_DIR}/portability/strfmon.h
    ${CMAKE_CURRENT_SOURCE_DIR}/portability/strptime.h
  )
endif()

HHVM_PUBLIC_HEADERS(util ${HEADER_SOURCES})

add_custom_command(
  OUTPUT "${CMAKE_CURRENT_SOURCE_DIR}/../hphp-repo-schema.h"
         "${CMAKE_CURRENT_SOURCE_DIR}/../hphp-build-info.cpp"
  COMMAND "${CMAKE_CURRENT_SOURCE_DIR}/generate-buildinfo.sh"
  DEPENDS ${CXX_SOURCES}
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/
  COMMENT "Generating Repo Schema ID and Compiler ID"
  VERBATIM)

find_package(LibNuma)
if (LIBNUMA_INCLUDE_DIRS)
  add_definitions("-DHAVE_NUMA=1")
  include_directories(${LIBNUMA_INCLUDE_DIRS})
endif()

if(MSVC)
  list(REMOVE_ITEM CXX_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/afdt-util.cpp")
  list(REMOVE_ITEM HEADER_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/afdt-util.h")
  list(REMOVE_ITEM CXX_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/light-process.cpp")
  list(REMOVE_ITEM HEADER_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/light-process.h")
endif()

add_library(hphp_util STATIC ${CXX_SOURCES} ${ASM_SOURCES} ${HEADER_SOURCES}
            "${CMAKE_CURRENT_SOURCE_DIR}/../hphp-repo-schema.h"
            "${CMAKE_CURRENT_SOURCE_DIR}/../hphp-build-info.cpp")
auto_source_group("hphp_util" "${CMAKE_CURRENT_SOURCE_DIR}"
  ${ASM_SOURCES} ${CXX_SOURCES} ${HEADER_SOURCES})
if (ENABLE_COTIRE)
  cotire(hphp_util)
endif()

if (LIBNUMA_LIBRARIES AND JEMALLOC_ENABLED)
  target_link_libraries(hphp_util ${LIBNUMA_LIBRARIES})
endif()
if (ENABLE_ASYNC_MYSQL)
  add_dependencies(hphp_util webscalesqlclient)
endif ()
