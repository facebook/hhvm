# Copyright (c) Meta Platforms, Inc. and affiliates.
# All rights reserved.
#
# This source code is licensed under the BSD-style license found in the
# LICENSE file in the root directory of this source tree.

proxygen_add_library(proxygen_hq_devious_baton
  SRCS
    DeviousBaton.cpp
  DEPS
    mvfst::mvfst_codec_types
    mvfst::mvfst_folly_utils
  EXPORTED_DEPS
    proxygen_http_message
    proxygen_webtransport
    Folly::folly_io_iobuf
)
