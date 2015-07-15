set(RECURSIVE_SOURCE_SUBDIRS base debugger eval server vm)
foreach (dir ${RECURSIVE_SOURCE_SUBDIRS})
  HHVM_SELECT_SOURCES("${CMAKE_CURRENT_SOURCE_DIR}/${dir}")
endforeach(dir ${RECURSIVE_SOURCE_SUBDIRS})
list(APPEND HEADER_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/version.h")

if(NOT LINUX)
  list(REMOVE_ITEM CXX_SOURCES
       ${CMAKE_CURRENT_SOURCE_DIR}/vm/debug/elfwriter.cpp)
endif()

if(NOT ENABLE_FASTCGI)
  HHVM_REMOVE_MATCHES_FROM_LISTS(CXX_SOURCES MATCHES "/fastcgi/")
endif()

if(NOT ENABLE_PROXYGEN_SERVER)
  HHVM_REMOVE_MATCHES_FROM_LISTS(CXX_SOURCES MATCHES "/proxygen/")
endif()

if(NOT HAVE_CUSTOM_LIBEVENT)
  HHVM_REMOVE_MATCHES_FROM_LISTS(CXX_SOURCES MATCHES "/server/libevent-")
endif()

add_custom_command(
  OUTPUT "${CMAKE_CURRENT_SOURCE_DIR}/ir-opcode-generated.h"
  COMMAND "${CMAKE_CURRENT_SOURCE_DIR}/../doc/generate-ir-opcodes.sh"
  DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/../doc/ir.specification"
          "${CMAKE_CURRENT_SOURCE_DIR}/../doc/generate-ir-opcodes.pl"
          "${CMAKE_CURRENT_SOURCE_DIR}/../doc/generate-ir-opcodes.sh"
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
  COMMENT "Generating HHIR opcode table"
  VERBATIM)

ADD_LIBRARY(hphp_runtime_static STATIC
            ${CXX_SOURCES} ${C_SOURCES} ${ASM_SOURCES}
            "ir-opcode-generated.h")
SET_TARGET_PROPERTIES(hphp_runtime_static PROPERTIES OUTPUT_NAME "hphp_runtime")
SET_TARGET_PROPERTIES(hphp_runtime_static PROPERTIES PREFIX "lib")
SET_TARGET_PROPERTIES(hphp_runtime_static PROPERTIES CLEAN_DIRECT_OUTPUT 1)

SET(CMAKE_CXX_ARCHIVE_APPEND "<CMAKE_AR> q <TARGET> <LINK_FLAGS> <OBJECTS>")

hphp_link(hphp_runtime_static)
HHVM_PUBLIC_HEADERS(runtime ${HEADER_SOURCES})

if (ENABLE_COTIRE)
  cotire(hphp_runtime_static)
endif()

if(CYGWIN AND CMAKE_BUILD_TYPE MATCHES "Debug")
  set_source_files_properties(
    ${CMAKE_CURRENT_SOURCE_DIR}/vm/jit/hhbc-translator.cpp
    PROPERTIES COMPILE_FLAGS -Og)
endif()

set(EZC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../runtime/ext_zend_compat/")
include_directories("${EZC_DIR}/php-src")
include_directories("${EZC_DIR}/php-src/main")
include_directories("${EZC_DIR}/php-src/Zend")
include_directories("${EZC_DIR}/php-src/TSRM")
