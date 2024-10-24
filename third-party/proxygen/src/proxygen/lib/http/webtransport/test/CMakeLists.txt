# Copyright (c) Meta Platforms, Inc. and affiliates.
# All rights reserved.
#
# This source code is licensed under the BSD-style license found in the
# LICENSE file in the root directory of this source tree.

if(NOT BUILD_TESTS)
    return()
endif()

proxygen_add_test(TARGET WebTransportAPITest
  SOURCES
    WebTransportAPITest.cpp
  DEPENDS
    proxygen
    testmain
)

proxygen_add_test(TARGET QuicWebTransportTest
  SOURCES
    QuicWebTransportTest.cpp
  DEPENDS
    quicwebtransport
    testmain
)
