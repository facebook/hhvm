set(PROXYGEN_LIB_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src/proxygen/lib")
set(PROXYGEN_HTTPSERVER_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src/proxygen/httpserver")
set(PROXYGEN_EXTERNAL_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src/proxygen/external")

# ported over from src/CMakeLists.txt
set(PROXYGEN_FBCODE_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/src)
set(PROXYGEN_GENERATED_ROOT ${CMAKE_CURRENT_BINARY_DIR}/generated)
file(MAKE_DIRECTORY ${PROXYGEN_GENERATED_ROOT})

find_program(PROXYGEN_PYTHON NAMES python3 python)
if(NOT PROXYGEN_PYTHON)
  message(WARNING "failed to identify python command, assuming 'python'")
  set(PROXYGEN_PYTHON "python")
endif()

# ported over from src/proxygen/lib/CMakeLists.txt
file(MAKE_DIRECTORY ${PROXYGEN_GENERATED_ROOT}/proxygen/lib/utils)
add_custom_command(
  OUTPUT
    ${PROXYGEN_GENERATED_ROOT}/proxygen/lib/utils/TraceEventType.h
    ${PROXYGEN_GENERATED_ROOT}/proxygen/lib/utils/TraceEventType.cpp
    ${PROXYGEN_GENERATED_ROOT}/proxygen/lib/utils/TraceFieldType.h
    ${PROXYGEN_GENERATED_ROOT}/proxygen/lib/utils/TraceFieldType.cpp
  COMMAND
    ${PROXYGEN_PYTHON}
    ${PROXYGEN_LIB_DIR}/utils/gen_trace_event_constants.py
    --output_type=cpp
    --input_files=samples/TraceEventType.txt,samples/TraceFieldType.txt
    --output_scope=proxygen
    --header_path=proxygen/lib/utils
    --install_dir=${PROXYGEN_GENERATED_ROOT}/proxygen/lib/utils
    --fbcode_dir=${PROXYGEN_FBCODE_ROOT}
  WORKING_DIRECTORY
    ${PROXYGEN_LIB_DIR}/utils/
  COMMENT "Generating TraceEventType and TraceFieldType"
)

set(CXX_SOURCES)
auto_sources(files "*.cpp" "RECURSE" "${PROXYGEN_LIB_DIR}")
auto_sources(serverfiles "*.cpp" "RECURSE" "${PROXYGEN_HTTPSERVER_DIR}")
list(APPEND files ${serverfiles})
HHVM_REMOVE_MATCHES_FROM_LISTS(
  files
  MATCHES
  "/samples/"
  "/test/"
  "/tests/"
  "/http/codec/compress/experimental/simulator/"
  # Quic
  "/HQ"
  "Quic"
)
list(APPEND CXX_SOURCES ${files})
list(APPEND CXX_SOURCES "${PROXYGEN_EXTERNAL_DIR}/http_parser/http_parser_cpp.cpp")
list(APPEND CXX_SOURCES "${PROXYGEN_LIB_DIR}/http/HTTPCommonHeaders.cpp")
list(APPEND CXX_SOURCES "${PROXYGEN_LIB_DIR}/http/HTTPCommonHeaders.h")
list(APPEND CXX_SOURCES
  ${PROXYGEN_GENERATED_ROOT}/proxygen/lib/utils/TraceEventType.cpp
  ${PROXYGEN_GENERATED_ROOT}/proxygen/lib/utils/TraceFieldType.cpp)

FIND_PROGRAM(GPERF_EXECUTABLE NAMES gperf)
IF(NOT GPERF_EXECUTABLE)
  MESSAGE(FATAL_ERROR "HHVM requires gperf for proxygen")
ENDIF()

add_custom_command(
  OUTPUT ${PROXYGEN_LIB_DIR}/http/HTTPCommonHeaders.h
  OUTPUT ${PROXYGEN_LIB_DIR}/http/HTTPCommonHeaders.cpp
  COMMAND ${PROXYGEN_LIB_DIR}/http/gen_HTTPCommonHeaders.sh "${PROXYGEN_LIB_DIR}/http/HTTPCommonHeaders.txt" "${CMAKE_CURRENT_SOURCE_DIR}/src" "${PROXYGEN_LIB_DIR}/http"
  DEPENDS ${PROXYGEN_LIB_DIR}/http/HTTPCommonHeaders.txt ${PROXYGEN_LIB_DIR}/http/gen_HTTPCommonHeaders.sh
)

add_definitions(-DNO_LIB_GFLAGS)

include_directories("${CMAKE_CURRENT_SOURCE_DIR}/..")
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/src")
include_directories("${TP_DIR}/fizz/src")
include_directories("${TP_DIR}/fb303/src")
if (NOT MSVC)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-unused-variable")
endif()
add_library(hphp_proxygen STATIC ${CXX_SOURCES})
target_include_directories(hphp_proxygen PUBLIC ${PROXYGEN_GENERATED_ROOT})

target_link_libraries(hphp_proxygen wangle zstd boost
  fizz libsodium
  ${LIBGLOG_LIBRARY} ${LIBPTHREAD_LIBRARIES})
