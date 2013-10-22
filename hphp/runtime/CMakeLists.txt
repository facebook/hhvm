set(RECURSIVE_SOURCE_SUBDIRS base debugger eval ext server vm)
foreach (dir ${RECURSIVE_SOURCE_SUBDIRS})
	auto_sources(files "*.cpp" "RECURSE" "${CMAKE_CURRENT_SOURCE_DIR}/${dir}")
	list(APPEND CXX_SOURCES ${files})

	auto_sources(files "*.c" "RECURSE" "${CMAKE_CURRENT_SOURCE_DIR}/${dir}")
	list(APPEND C_SOURCES ${files})

	auto_sources(files "*.S" "RECURSE" "${CMAKE_CURRENT_SOURCE_DIR}/${dir}")
	list(APPEND ASM_SOURCES ${files})
endforeach(dir ${RECURSIVE_SOURCE_SUBDIRS})

if(NOT LINUX)
	add_definitions(-DNO_HARDWARE_COUNTERS)
	list(REMOVE_ITEM CXX_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/vm/debug/elfwriter.cpp)
endif()

# remove ext_hhvm, and anything in a test folder
foreach (file ${CXX_SOURCES})
	if (${file} MATCHES "ext_hhvm")
		list(REMOVE_ITEM CXX_SOURCES ${file})
	endif()
	if (${file} MATCHES "/test/")
		list(REMOVE_ITEM CXX_SOURCES ${file})
	endif()
endforeach(file ${CXX_SOURCES})

if(NOT HAVE_CUSTOM_LIBEVENT)
	# Not working with off-the-shelf libevent
	list(REMOVE_ITEM CXX_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/server/server-name-indication.cpp")
	foreach (file ${CXX_SOURCES})
		if(${file} MATCHES "/server/libevent-")
			list(REMOVE_ITEM CXX_SOURCES ${file})
		endif()
	endforeach(file ${CXX_SOURES})
endif()

include_directories("${HPHP_HOME}/hphp/third_party")

ADD_LIBRARY(hphp_runtime_static STATIC
            ${CXX_SOURCES} ${C_SOURCES} ${ASM_SOURCES})
SET_TARGET_PROPERTIES(hphp_runtime_static PROPERTIES OUTPUT_NAME "hphp_runtime")
SET_TARGET_PROPERTIES(hphp_runtime_static PROPERTIES PREFIX "lib")
SET_TARGET_PROPERTIES(hphp_runtime_static PROPERTIES CLEAN_DIRECT_OUTPUT 1)

SET(CMAKE_CXX_ARCHIVE_APPEND "<CMAKE_AR> q <TARGET> <LINK_FLAGS> <OBJECTS>")

hphp_link(hphp_runtime_static)
add_dependencies(hphp_runtime_static hphp_parser)

