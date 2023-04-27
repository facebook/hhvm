# Copyright (c) Meta Platforms, Inc. and affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

add_fb_python_unittest(bser_js
  SOURCES
    test_bser.py

  DEPENDS
    integration_lib

  WORKING_DIRECTORY
    ${CMAKE_BINARY_DIR}

  ENV
    "YARN_PATH=${YARN}"
    "NODE_BIN=${NODE}"
    "WATCHMAN_SRC_DIR=${CMAKE_SOURCE_DIR}/watchman"
)
