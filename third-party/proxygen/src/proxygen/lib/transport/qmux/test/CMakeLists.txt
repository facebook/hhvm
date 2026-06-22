# Copyright (c) Meta Platforms, Inc. and affiliates.
# All rights reserved.
#
# This source code is licensed under the BSD-style license found in the
# LICENSE file in the root directory of this source tree.

if(NOT BUILD_TESTS)
    return()
endif()

# These tests link granular proxygen library targets rather than the
# monolithic `proxygen` target.

proxygen_add_test(TARGET QmuxFramerTest
  SOURCES
    QmuxFramerTest.cpp
  DEPENDS
    proxygen_transport_qmux_qmux_framer
    mvfst::mvfst_codec_types
    mvfst::mvfst_folly_utils
    testmain
)

proxygen_add_test(TARGET QmuxCodecTest
  SOURCES
    QmuxCodecTest.cpp
  DEPENDS
    proxygen_transport_qmux_qmux_codec
    proxygen_http_codec_webtransport_webtransport_framer
    proxygen_http_webtransport_wt_stream_manager
    proxygen_http_webtransport_wt_util
    mvfst::mvfst_codec_types
    mvfst::mvfst_priority_http_priority_queue
    testmain
)

proxygen_add_test(TARGET QmuxSessionTest
  SOURCES
    QmuxSessionTest.cpp
  DEPENDS
    proxygen_transport_qmux_qmux_session
    proxygen_transport_qmux_qmux_codec
    proxygen_transport_qmux_qmux_framer
    proxygen_http_codec_webtransport_webtransport_framer
    proxygen_http_webtransport_wt_stream_manager
    proxygen_http_webtransport_wt_util
    TestCoroTransport
    testmain
)

proxygen_add_test(TARGET QmuxConnectorTest
  SOURCES
    QmuxConnectorTest.cpp
  DEPENDS
    proxygen_transport_qmux_qmux_connector
    proxygen_transport_qmux_qmux_framer
    TestCoroTransport
    testmain
)
