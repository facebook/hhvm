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
  COMMAND "hphp/util/generate-buildinfo.sh"
  DEPENDS ${CXX_SOURCES}
  WORKING_DIRECTORY ${HPHP_HOME}
  COMMENT "Generating Repo Schema ID and Compiler ID"
  VERBATIM)

add_library(hphp_util STATIC ${CXX_SOURCES}
            "${HPHP_HOME}/hphp/hphp-repo-schema.h"
            "${HPHP_HOME}/hphp/hphp-build-info.cpp")
