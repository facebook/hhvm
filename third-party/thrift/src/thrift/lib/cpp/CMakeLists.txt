# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.


add_library(
  thrift-core

  Thrift.cpp
  ../cpp2/FieldRef.cpp
)
target_include_directories(
  thrift-core
  INTERFACE
    $<INSTALL_INTERFACE:${INCLUDE_INSTALL_DIR}>
)
target_link_libraries(
  thrift-core
  PUBLIC
    fmt::fmt
    Folly::folly
)

add_library(
  concurrency

  concurrency/InitThreadFactory.cpp
  concurrency/PosixThreadFactory.cpp
  concurrency/ThreadManager.cpp
  concurrency/TimerManager.cpp
  concurrency/Util.cpp
)
target_link_libraries(
  concurrency
  PUBLIC
    Folly::folly
    ${LIBGFLAGS_LIBRARY}
    ${GLOG_LIBRARIES}
)

add_library(
  transport

  transport/TTransportException.cpp
  transport/TFDTransport.cpp
  transport/THttpTransport.cpp
  transport/THttpClient.cpp
  transport/THttpServer.cpp
  transport/TSocket.cpp
  transport/TBufferTransports.cpp
  transport/THeader.cpp
  transport/TZlibTransport.cpp
  ../cpp2/transport/rocket/framing/Util.cpp
  ../cpp2/Flags.cpp
  util/PausableTimer.cpp
  util/THttpParser.cpp
  util/VarintUtils.cpp
)
add_dependencies(
  transport
  rpcmetadata
)
target_link_libraries(
  transport
  PUBLIC
    concurrency
    thrift-core
    rpcmetadata
    Folly::folly
    ZLIB::ZLIB
    ${OPENSSL_LIBRARIES}
    ${ZSTD_LIBRARIES}
)

add_library(
  async

  ContextStack.cpp
  EventHandlerBase.cpp
  async/TBinaryAsyncChannel.cpp
  async/TFramedAsyncChannel.cpp
  async/THttpAsyncChannel.cpp
  async/TUnframedAsyncChannel.cpp
  server/TServerObserver.cpp
  ../cpp2/detail/EventHandlerRuntime.cpp
)
if (
  CMAKE_CXX_COMPILER_ID MATCHES "Clang" OR
  CMAKE_CXX_COMPILER_ID STREQUAL "GNU"
)
  target_compile_options(
    async
    PUBLIC
      $<$<COMPILE_LANGUAGE:CXX>:-fsized-deallocation>
  )
endif()
target_link_libraries(
  async
  PUBLIC
    concurrency
    transport
    Boost::boost
    Folly::folly
    wangle::wangle
    ${GLOG_LIBRARIES}
    ${OPENSSL_LIBRARIES}
)

add_library(
  thrift
  INTERFACE
)
target_link_libraries(
  thrift
  INTERFACE
    async
    concurrency
    thriftprotocol
    transport
    Folly::folly
    ${GLOG_LIBRARIES}
)

set(THRIFT1_HEADER_DIRS
  async
  concurrency
  protocol
  server
  transport
)
foreach(dir ${THRIFT1_HEADER_DIRS})
  install(DIRECTORY ${dir} DESTINATION ${INCLUDE_INSTALL_DIR}/thrift/lib/cpp
          FILES_MATCHING PATTERN "*.h")
  install(DIRECTORY ${dir} DESTINATION ${INCLUDE_INSTALL_DIR}/thrift/lib/cpp
          FILES_MATCHING PATTERN "*.tcc")
endforeach()

if (BUILD_SHARED_LIBS)
  # all but thrift since it's an interface
  set_target_properties(thrift-core concurrency transport async
    PROPERTIES VERSION ${PACKAGE_VERSION}
    )
endif()

install(
  TARGETS
    thrift-core
    concurrency
    transport
    async
    thrift
  EXPORT fbthrift-exports
  DESTINATION ${LIB_INSTALL_DIR}
)
