# Copyright (c) Meta Platforms, Inc. and affiliates.
# All rights reserved.
#
# This source code is licensed under the BSD-style license found in the
# LICENSE file in the root directory of this source tree.

if(NOT BUILD_TESTS)
    return()
endif()

proxygen_add_test(TARGET TransportTests
  SOURCES
      H3DatagramAsyncSocketTest.cpp
    DEPENDS
      codectestutils
      sessiontestutils
      proxygen
      testmain
      mvfst::mvfst_codec_types
      mvfst::mvfst_dsr_types
      mvfst::mvfst_state_machine
)
