set(CXX_SOURCES)
auto_sources(files "*.cpp" "RECURSE" "${CMAKE_CURRENT_SOURCE_DIR}")
list(APPEND CXX_SOURCES ${files})

foreach (file ${files})
	if (${file} MATCHES "/test/")
		list(REMOVE_ITEM CXX_SOURCES ${file})
	endif()
endforeach()

add_custom_command(
  OUTPUT "${HPHP_HOME}/hphp/hphp-repo-schema.h"
         "${HPHP_HOME}/hphp/hphp-build-info.cpp"
  COMMAND "HPHP_HOME=${HPHP_HOME}" "hphp/util/generate-buildinfo.sh"
  DEPENDS ${CXX_SOURCES}
  WORKING_DIRECTORY ${HPHP_HOME}
  COMMENT "Generating Repo Schema ID and Compiler ID"
  VERBATIM)

## Disabling numa support (for now) since distro
## libnuma packages don't always have numa_init available
## Find a better way to make sure numa initialization
## happens on time. -sgolemon (2013-10-16)

#find_package(LibNuma)
#if (LIBNUMA_INCLUDE_DIRS)
#	add_definitions("-DHAVE_NUMA=1")
#	include_directories(${LIBNUMA_INCLUDE_DIRS})
#endif()

add_library(hphp_util STATIC ${CXX_SOURCES}
            "${HPHP_HOME}/hphp/hphp-repo-schema.h"
            "${HPHP_HOME}/hphp/hphp-build-info.cpp")

#if (LIBNUMA_LIBRARIES)
#	target_link_libraries(hphp_util ${LIBNUMA_LIBRARIES})
#endif()
