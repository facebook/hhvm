set(RECURSIVE_SOURCE_SUBDIRS base debugger eval server vm)
foreach (dir ${RECURSIVE_SOURCE_SUBDIRS})
  HHVM_SELECT_SOURCES("${CMAKE_CURRENT_SOURCE_DIR}/${dir}")
endforeach(dir ${RECURSIVE_SOURCE_SUBDIRS})

if(NOT LINUX)
  add_definitions(-DNO_HARDWARE_COUNTERS)
  list(REMOVE_ITEM CXX_SOURCES
       ${CMAKE_CURRENT_SOURCE_DIR}/vm/debug/elfwriter.cpp)
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
  list(REMOVE_ITEM CXX_SOURCES
       "${CMAKE_CURRENT_SOURCE_DIR}/server/server-name-indication.cpp")
  foreach (file ${CXX_SOURCES})
    if(${file} MATCHES "/server/libevent-")
      list(REMOVE_ITEM CXX_SOURCES ${file})
    endif()
  endforeach(file ${CXX_SOURES})
endif()

ADD_LIBRARY(hphp_runtime_static STATIC
            ${CXX_SOURCES} ${C_SOURCES} ${ASM_SOURCES})
SET_TARGET_PROPERTIES(hphp_runtime_static PROPERTIES OUTPUT_NAME "hphp_runtime")
SET_TARGET_PROPERTIES(hphp_runtime_static PROPERTIES PREFIX "lib")
SET_TARGET_PROPERTIES(hphp_runtime_static PROPERTIES CLEAN_DIRECT_OUTPUT 1)

SET(CMAKE_CXX_ARCHIVE_APPEND "<CMAKE_AR> q <TARGET> <LINK_FLAGS> <OBJECTS>")

hphp_link(hphp_runtime_static)
HHVM_PUBLIC_HEADERS(runtime ${HEADER_SOURCES})

if (ENABLE_COTIRE)
  cotire(hphp_runtime_static)
endif()
