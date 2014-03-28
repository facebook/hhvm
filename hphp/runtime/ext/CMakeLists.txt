set(CXX_SOURCES)
set(C_SOURCES)
set(ASM_SOURCES)

# Pick up all files in the base ext/ folder unconditionally
auto_sources(files "*.cpp" "${CMAKE_CURRENT_SOURCE_DIR}")
list(APPEND CXX_SOURCES ${files})
auto_sources(files "*.c" "${CMAKE_CURRENT_SOURCE_DIR}")
list(APPEND C_SOURCES ${files})
auto_sources(files "*.S" "${CMAKE_CURRENT_SOURCE_DIR}")
list(APPEND ASM_SOURCES ${files})

set(HRE_LIBARIES)
include(EXTFunctions)

# For each directory under ext/, either pull in standard
# source lists, or let the particular extension make that
# decision on its own
file(GLOB subdirs ${CMAKE_CURRENT_SOURCE_DIR}/*)
foreach (dir ${subdirs})
	if (IS_DIRECTORY ${dir})
		if (EXISTS "${dir}/config.cmake")
			set(HRE_CURRENT_EXT_PATH "${dir}")
			include("${dir}/config.cmake")
		else ()
			HHVM_SELECT_SOURCES(${dir})
		endif()
	endif()
endforeach()

include_directories("${HPHP_HOME}/hphp/third_party")

ADD_LIBRARY(hphp_runtime_ext STATIC
            ${CXX_SOURCES} ${C_SOURCES} ${ASM_SOURCES})
target_link_libraries(hphp_runtime_ext ${HRE_LIBRARIES})
hphp_link(hphp_runtime_ext)
