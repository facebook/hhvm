# Copyright (c) Meta Platforms, Inc. and affiliates.
# All rights reserved.
#
# This source code is licensed under the BSD-style license found in the
# LICENSE file in the root directory of this source tree.

proxygen_add_test(TARGET DeviousBatonTests
  SOURCES
    DeviousBatonTests.cpp
  DEPENDS
    proxygendeviousbaton
    testmain
)
