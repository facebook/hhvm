# Copyright (c) Meta Platforms, Inc. and affiliates.
# All rights reserved.
#
# This source code is licensed under the BSD-style license found in the
# LICENSE file in the root directory of this source tree.

if(NOT BUILD_TESTS)
    return()
endif()

add_library(hpacktestutils TestUtil.cpp)
target_include_directories(
    hpacktestutils PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    ${LIBGMOCK_INCLUDE_DIR}
    ${LIBGTEST_INCLUDE_DIR}
)
target_compile_options(
    hpacktestutils PRIVATE
    ${_PROXYGEN_COMMON_COMPILE_OPTIONS}
)
target_link_libraries(hpacktestutils PUBLIC proxygen)

proxygen_add_test(TARGET HPACKTests
  SOURCES
    HeaderPieceTests.cpp
    HeaderTableTests.cpp
    HTTPArchive.cpp
    HPACKBufferTests.cpp
    HPACKCodecTests.cpp
    HPACKContextTests.cpp
    HPACKHeaderTests.cpp
    HuffmanTests.cpp
    LoggingTests.cpp
    QPACKCodecTests.cpp
    QPACKContextTests.cpp
    QPACKHeaderTableTests.cpp
    RFCExamplesTests.cpp
  DEPENDS
    hpacktestutils
    proxygen
    testmain
)
