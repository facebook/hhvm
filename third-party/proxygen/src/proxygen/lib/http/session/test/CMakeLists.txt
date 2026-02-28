# Copyright (c) Meta Platforms, Inc. and affiliates.
# All rights reserved.
#
# This source code is licensed under the BSD-style license found in the
# LICENSE file in the root directory of this source tree.

# Install MockQuicSocketDriver.h for downstream test targets
install(FILES MockQuicSocketDriver.h DESTINATION include/proxygen/lib/http/session/test)

if(NOT BUILD_TESTS)
    return()
endif()

# Header-only library for MockQuicSocketDriver
add_library(mock_quic_socket_driver INTERFACE)
target_include_directories(mock_quic_socket_driver INTERFACE
    $<BUILD_INTERFACE:${PROXYGEN_FBCODE_ROOT}>
    $<INSTALL_INTERFACE:include>
)
target_link_libraries(mock_quic_socket_driver INTERFACE
    mvfst::mvfst_api_test_mocks
    mvfst::mvfst_common_events_folly_eventbase
    Folly::folly
)

add_library(sessiontestutils TestUtils.cpp)
target_include_directories(
    sessiontestutils PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    ${LIBGMOCK_INCLUDE_DIR}
    ${LIBGTEST_INCLUDE_DIR}
)
target_compile_options(
    sessiontestutils PRIVATE
    ${_PROXYGEN_COMMON_COMPILE_OPTIONS}
)
target_link_libraries(sessiontestutils PUBLIC proxygen)


proxygen_add_test(TARGET SessionTests
  SOURCES
    ByteEventTrackerTest.cpp
    DownstreamTransactionTest.cpp
    HTTPDownstreamSessionTest.cpp
    HTTPSessionAcceptorTest.cpp
    HTTPUpstreamSessionTest.cpp
    MockCodecDownstreamTest.cpp
    HTTP2PriorityQueueTest.cpp
    HTTPDefaultSessionCodecFactoryTest.cpp
    HTTPTransactionSMTest.cpp
    HTTPTransactionTest.cpp
    HTTPTransactionWebTransportTest.cpp
  DEPENDS
    codectestutils
    sessiontestutils
    testtransport
    proxygen
    testmain
)

proxygen_add_test(TARGET HQSessionTests
  SOURCES
    HQByteEventTrackerTest.cpp
    HQDownstreamSessionTest.cpp
    HQSessionMocksTest.cpp
    HQSessionTestCommon.cpp
    HQStreamBaseTest.cpp
    HQStreamDispatcherTest.cpp
    HQUpstreamSessionTest.cpp
  DEPENDS
    codectestutils
    sessiontestutils
    testtransport
    proxygen
    testmain
    mvfst::mvfst_codec_types
    mvfst::mvfst_state_machine
)
