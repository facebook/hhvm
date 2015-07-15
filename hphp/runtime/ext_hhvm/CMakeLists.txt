set(files)
set(CXX_SOURCES)
set(C_SOURCES)
set(CXX_HEADERS)
set(EXT_HHVM_ARCH "x64")

exec_program("uname -m" OUTPUT_VARIABLE UNAME_M)
if (UNAME_M STREQUAL "aarch64")
  set(EXT_HHVM_ARCH "arm")
endif()

macro(EXT_HHVM_FILE SOURCES HEADERS REL EHHVM_ARCH SOURCE_REL BUILD_TARGET)
  set(SOURCE_ABS "${CMAKE_CURRENT_SOURCE_DIR}/${SOURCE_REL}")
  set(TARGET_ABS "${CMAKE_CURRENT_BINARY_DIR}/${SOURCE_REL}")
  set(f_SRC "${SOURCE_ABS}/${REL}")
  set(f_OBJ "${TARGET_ABS}/CMakeFiles/${BUILD_TARGET}.dir/${REL}")
  list(APPEND ${SOURCES} "${f_OBJ}.ext_hhvm.cpp")
  list(APPEND ${HEADERS} "${f_OBJ}.ext_hhvm.h")

  file(MAKE_DIRECTORY "${TARGET_ABS}/CMakeFiles/${BUILD_TARGET}.dir")
  add_custom_command(
    OUTPUT "${f_OBJ}.ext_hhvm.cpp" "${f_OBJ}.ext_hhvm.h"
    DEPENDS ${f_SRC} gen-ext-hhvm
    COMMAND gen-ext-hhvm
    ARGS ${EHHVM_ARCH} "${f_OBJ}.ext_hhvm.h" "${f_OBJ}.ext_hhvm.cpp" "${f_SRC}"
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/../../tools/bootstrap"
    COMMENT "Generating ext_hhvm wrapper for ${SOURCE_REL}/${REL}")
endmacro()

macro(EXT_HHVM_DIR SOURCES HEADERS SOURCE_REL BUILD_TARGET)
  string(LENGTH "${CMAKE_CURRENT_SOURCE_DIR}/${SOURCE_REL}" HPHP_DIR_LEN)
  auto_sources(files "*.idl.json" "RECURSE"
               "${CMAKE_CURRENT_SOURCE_DIR}/${SOURCE_REL}")
  foreach (f ${files})
    # This makes cpp files in the root path into a bogus lookup
    # But that's okay since we want it to fail anyhow
    string(REGEX REPLACE "../../system/idl/([^/]+)(/.*)?$"
                         "../../system/idl/\\1/config.cmake" DIR_CONFIG ${f})
    if (NOT EXISTS ${DIR_CONFIG} AND
                    NOT ${f} MATCHES "(.ext_hhvm.cpp|/(old-)?tests?/)")
      string(LENGTH ${f} f_LEN)
      math(EXPR f_REL_LEN "${f_LEN} - ${HPHP_DIR_LEN} - 1")
      math(EXPR f_REL_OFF "${HPHP_DIR_LEN} + 1")
      string(SUBSTRING ${f} ${f_REL_OFF} ${f_REL_LEN} f_REL)
      EXT_HHVM_FILE(${SOURCES} ${HEADERS} ${f_REL} ${EXT_HHVM_ARCH}
                    ${SOURCE_REL} ${BUILD_TARGET})
    endif()
  endforeach()
endmacro()

# Generate ext_hhvm stubs for system/idl/*.idl.json
EXT_HHVM_DIR(CXX_SOURCES CXX_HEADERS "../../system/idl" "hphp_runtime_ext")

#
## ext_hhvm_infotabs
#

set(ROOT_GEN_DIR "${CMAKE_CURRENT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/generated")
set(GENERATED_DIR "${ROOT_GEN_DIR}/hphp/runtime/ext_hhvm/")
include_directories(BEFORE "${ROOT_GEN_DIR}")
set(INFOTABS_HEADER "${GENERATED_DIR}/ext_hhvm_infotabs.h")
set(INFOTABS_SOURCE "${GENERATED_DIR}/ext_hhvm_infotabs.cpp")

auto_sources(HPHP_IDLS "*.idl.json" "RECURSE"
             "${CMAKE_CURRENT_SOURCE_DIR}/../../system/idl")

# Generate ext_hhvm_infotabs.h based on ext_hhvm files being generated
file(WRITE ${INFOTABS_HEADER} "")
foreach (header ${CXX_HEADERS})
  file(APPEND ${INFOTABS_HEADER} "#include \"${header}\"\n")
endforeach()

# Generate ext_hhvm_infotabs.cpp
add_custom_command(
  OUTPUT ${INFOTABS_SOURCE}
  DEPENDS ${CXX_HEADERS} gen-infotabs
  COMMAND gen-infotabs
  ARGS ${INFOTABS_SOURCE} ${INFOTABS_HEADER} ${HPHP_IDLS})
list(APPEND CXX_SOURCES ${INFOTABS_SOURCE})

#
## Actual library
#

ADD_LIBRARY(ext_hhvm_static STATIC ${CXX_SOURCES})
target_link_libraries(ext_hhvm_static hphp_runtime_ext)
SET_TARGET_PROPERTIES(ext_hhvm_static PROPERTIES OUTPUT_NAME "ext_hhvm")
SET_TARGET_PROPERTIES(ext_hhvm_static PROPERTIES PREFIX "lib")
SET_TARGET_PROPERTIES(ext_hhvm_static PROPERTIES CLEAN_DIRECT_OUTPUT 1)
if (ENABLE_COTIRE)
  cotire(ext_hhvm_static)
endif()
