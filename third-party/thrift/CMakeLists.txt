set(THRIFT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src/thrift")

set(CXX_SOURCES)

auto_sources(files "*.cpp" "RECURSE" "${THRIFT_DIR}")

HHVM_REMOVE_MATCHES_FROM_LISTS(files MATCHES
  "/demo/"
  "/perf/"
  "/test/"
  "/tests/"
  "/compiler/"
  "/server/"
  "/lib/d/"
  "/lib/node/"
  "/lib/cpp2/transport/core/testutil/"
  "/lib/cpp2/transport/.*RoutingHandler.*"
  "/lib/py/"
  "/lib/php/"
  "/testutil/"
  "/tutorial/"
  "/example/"
  "Test.cpp"
  "/neutronium/"
  "/kerberos/"
)

list(REMOVE_ITEM files
  ${THRIFT_DIR}/contrib/thrift_dump.cpp
  ${THRIFT_DIR}/lib/cpp/transport/HDFS.cpp
  ${THRIFT_DIR}/lib/cpp/transport/THDFSFileTransport.cpp
  ${THRIFT_DIR}/lib/cpp/transport/THeaderTransport.cpp
  ${THRIFT_DIR}/lib/cpp/transport/TMemPagedTransport.cpp
  ${THRIFT_DIR}/lib/cpp/transport/TMemPagedFactory.cpp
  ${THRIFT_DIR}/lib/cpp/protocol/TSimpleJSONProtocol.cpp
  ${THRIFT_DIR}/lib/cpp/concurrency/NumaThreadManager.cpp
  ${THRIFT_DIR}/lib/cpp/util/TThreadedServerCreator.cpp
  ${THRIFT_DIR}/lib/cpp/util/TNonblockingServerCreator.cpp
)

if (NOT OPENSSL_HAVE_ALPN)
  HHVM_REMOVE_MATCHES_FROM_LISTS(files MATCHES
    "/lib/cpp2/transport/rsocket/"
    "/lib/cpp2/transport/rocket/"
    ".*/RSocket.*"
    ".*/Rocket.*"
  )
endif()

list(APPEND CXX_SOURCES ${files})

auto_sources(files "*.cpp" "RECURSE" "${CMAKE_CURRENT_SOURCE_DIR}/gen")
list(APPEND CXX_SOURCES ${files})
auto_sources(files "*.tcc" "RECURSE" "${CMAKE_CURRENT_SOURCE_DIR}/gen")
list(APPEND CXX_SOURCES ${files})

include_directories("${CMAKE_CURRENT_SOURCE_DIR}/..")
include_directories("${CMAKE_CURRENT_SOURCE_DIR}")
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/gen")
include_directories("${TP_DIR}/fmt/fmt/include")
include_directories("${TP_DIR}/fizz/src")
include_directories("${TP_DIR}/rsocket-cpp/src")
include_directories("${TP_DIR}/fatal")
include_directories("${TP_DIR}/proxygen/src")
add_library(hphp_thrift STATIC ${CXX_SOURCES})

target_link_libraries(hphp_thrift wangle boost fmt fizz rsocket_cpp hphp_proxygen
                                         ${LIBGLOG_LIBRARY}
                                         ${LIBPTHREAD_LIBRARIES})
