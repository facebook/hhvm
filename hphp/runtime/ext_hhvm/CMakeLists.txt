set(files)
set(CXX_SOURCES)
set(C_SOURCES)
set(CXX_HEADERS)
set(EXT_HHVM_ARCH "x64")
set(EXT_HHVM_OS "linux")

exec_program("uname -m" OUTPUT_VARIABLE UNAME_M)
if (UNAME_M STREQUAL "aarch64")
	set(EXT_HHVM_ARCH "arm")
endif()

if (APPLE)
	set(EXT_HHVM_OS "darwin")
endif()

macro(EXT_HHVM_FILE SOURCES HEADERS REL EHHVM_OS EHHVM_ARCH)
	set(f_SRC "${HPHP_HOME}/hphp/runtime/${REL}")
	set(f_OBJ "${HPHP_HOME}/hphp/runtime/CMakeFiles/hphp_runtime_static.dir/${REL}.o")
	list(APPEND ${SOURCES} "${f_SRC}.ext_hhvm.cpp")
	list(APPEND ${HEADERS} "${f_SRC}.ext_hhvm.h")
	add_custom_command(OUTPUT "${f_SRC}.ext_hhvm.cpp" "${f_SRC}.ext_hhvm.h"
	                   DEPENDS ${f_SRC} gen-ext-hhvm
	                   COMMAND "${HPHP_HOME}/hphp/tools/bootstrap/gen-ext-hhvm.sh"
	                   ARGS ${EHHVM_OS} ${EHHVM_ARCH} ${f_OBJ}
                               "${f_SRC}.ext_hhvm.cpp" "${f_SRC}.ext_hhvm.h"
	                   WORKING_DIRECTORY "${HPHP_HOME}/hphp/tools/bootstrap"
	                   COMMENT "Generating ext_hhvm wrapper for ${REL}")
endmacro()

# Generate ext_hhvm stubs for ext/*.cpp
string(LENGTH "${HPHP_HOME}/hphp/runtime/" HPHP_DIR_LEN)
auto_sources(files "*.cpp" "RECURSE" "${HPHP_HOME}/hphp/runtime/ext")
foreach (f ${files})
	if (NOT ${f} MATCHES ".ext_hhvm.cpp")
		string(LENGTH ${f} f_LEN)
		math(EXPR f_REL_LEN "${f_LEN} - ${HPHP_DIR_LEN}")
		string(SUBSTRING ${f} ${HPHP_DIR_LEN} ${f_REL_LEN} f_REL)
		EXT_HHVM_FILE(CXX_SOURCES CXX_HEADERS ${f_REL} ${EXT_HHVM_OS} ${EXT_HHVM_ARCH})
	endif()
endforeach()
# ...and explicitly for builtin-functions.cpp
EXT_HHVM_FILE(CXX_SOURCES CXX_HEADERS "base/builtin-functions.cpp" ${EXT_HHVM_OS} ${EXT_HHVM_ARCH})


#
## ext_hhvm_infotabs
#

set(INFOTABS_HEADER "${CMAKE_CURRENT_SOURCE_DIR}/ext_hhvm_infotabs.h")
set(INFOTABS_SOURCE "${CMAKE_CURRENT_SOURCE_DIR}/ext_hhvm_infotabs.cpp")

# Exclude ext_zend_compact stuff
auto_sources(HPHP_IDLS "*.idl.json" "RECURSE" "${HPHP_HOME}/hphp/system/idl")
foreach (zext calendar ftp gettext mongo oauth yaml)
	list(REMOVE_ITEM HPHP_IDLS "${HPHP_HOME}/hphp/system/idl/${zext}.idl.json")
endforeach ()

# Generate ext_hhvm_infotabs.h based on ext_hhvm files being generated
file(WRITE ${INFOTABS_HEADER} "")
foreach (header ${CXX_HEADERS})
	file(APPEND ${INFOTABS_HEADER} "#include \"${header}\"\n")
endforeach()

# Generate ext_hhvm_infotabs.cpp
add_custom_command(OUTPUT ${INFOTABS_SOURCE}
                   DEPENDS ${CXX_HEADERS} gen-infotabs
                   COMMAND "${HPHP_HOME}/hphp/tools/bootstrap/gen-infotabs"
                   ARGS ${INFOTABS_SOURCE} ${HPHP_IDLS})
list(APPEND CXX_SOURCES ${INFOTABS_SOURCE})


#
## Actual library
#

ADD_LIBRARY(ext_hhvm_static STATIC ${CXX_SOURCES})
target_link_libraries(ext_hhvm_static hphp_runtime_static)
SET_TARGET_PROPERTIES(ext_hhvm_static PROPERTIES OUTPUT_NAME "ext_hhvm")
SET_TARGET_PROPERTIES(ext_hhvm_static PROPERTIES PREFIX "lib")
SET_TARGET_PROPERTIES(ext_hhvm_static PROPERTIES CLEAN_DIRECT_OUTPUT 1)

