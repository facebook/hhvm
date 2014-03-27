set(RECURSIVE_SOURCE_SUBDIRS base debugger eval server vm)
foreach (dir ${RECURSIVE_SOURCE_SUBDIRS})
	HHVM_SELECT_SOURCES("${CMAKE_CURRENT_SOURCE_DIR}/${dir}")
endforeach(dir ${RECURSIVE_SOURCE_SUBDIRS})
foreach (dir ${ZEND_COMPAT_BUILD_DIRS})
	HHVM_SELECT_SOURCES("${dir}")
endforeach(dir ${ZEND_COMPAT_BUILD_DIRS})

if(NOT LINUX)
	add_definitions(-DNO_HARDWARE_COUNTERS)
	list(REMOVE_ITEM CXX_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/vm/debug/elfwriter.cpp)
endif()

if(NOT ENABLE_FASTCGI)
	foreach (file ${CXX_SOURCES})
		if (${file} MATCHES "/fastcgi/")
			list(REMOVE_ITEM CXX_SOURCES ${file})
		endif()
	endforeach(file ${CXX_SOURCES})
endif()

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
