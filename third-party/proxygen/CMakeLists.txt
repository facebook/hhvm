set(PROXYGEN_LIB_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src/proxygen/lib")
set(PROXYGEN_HTTPSERVER_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src/proxygen/httpserver")
set(PROXYGEN_EXTERNAL_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src/proxygen/external")

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
  "/lib/utils/(Null)?TraceEvent"
  "/http/codec/compress/experimental/simulator/"
  # Quic
  "/HQ"
  "Quic"
  "/http/HTTP3ErrorCode.cpp"
  "/http/codec/UnframedBodyOffsetTracker.cpp"
)
list(APPEND CXX_SOURCES ${files})
list(APPEND CXX_SOURCES "${PROXYGEN_EXTERNAL_DIR}/http_parser/http_parser_cpp.cpp")
list(APPEND CXX_SOURCES "${PROXYGEN_LIB_DIR}/http/HTTPCommonHeaders.cpp")
list(APPEND CXX_SOURCES "${PROXYGEN_LIB_DIR}/http/HTTPCommonHeaders.h")

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

add_dependencies(hphp_proxygen libsodiumMaybeBuild)
target_include_directories(hphp_proxygen PUBLIC "${LIBSODIUM_INCLUDE_DIRS}")

target_link_libraries(hphp_proxygen wangle zstd boost brotli_dec brotli_enc
  fizz libsodium
  ${LIBGLOG_LIBRARY} ${LIBPTHREAD_LIBRARIES})
