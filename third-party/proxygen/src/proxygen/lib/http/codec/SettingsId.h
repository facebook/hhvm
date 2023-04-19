/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <cstdint>
#include <utility>

namespace proxygen {

// Will never be valid HTTP/2 which only has 16 bits
#define SPDY_SETTINGS_MASK (1 << 16)
#define HQ_SETTINGS_MASK (1ull << 32)

enum class SettingsId : uint64_t {
  // From HTTP/2
  HEADER_TABLE_SIZE = 1,
  ENABLE_PUSH = 2,
  MAX_CONCURRENT_STREAMS = 3,
  INITIAL_WINDOW_SIZE = 4,
  MAX_FRAME_SIZE = 5,
  MAX_HEADER_LIST_SIZE = 6,

  ENABLE_CONNECT_PROTOCOL = 8,

  THRIFT_CHANNEL_ID_DEPRECATED = 100,

  // 0xf000 and 0xffff being reserved for Experimental Use
  ENABLE_EX_HEADERS = 0xfbfb,
  THRIFT_CHANNEL_ID = 0xf100,

  // For secondary authentication in HTTP/2
  SETTINGS_HTTP_CERT_AUTH = 0xff00,

  // From SPDY, mostly unused
  _SPDY_UPLOAD_BANDWIDTH = SPDY_SETTINGS_MASK | 1,
  _SPDY_DOWNLOAD_BANDWIDTH = SPDY_SETTINGS_MASK | 2,
  _SPDY_ROUND_TRIP_TIME = SPDY_SETTINGS_MASK | 3,
  //  MAX_CONCURRENT_STREAMS = 4,
  _SPDY_CURRENT_CWND = SPDY_SETTINGS_MASK | 5,
  _SPDY_DOWNLOAD_RETRANS_RATE = SPDY_SETTINGS_MASK | 6,
  //  INITIAL_WINDOW_SIZE = 7,
  _SPDY_CLIENT_CERTIFICATE_VECTOR_SIZE = SPDY_SETTINGS_MASK | 8,

  // From HQ
  //_HQ_HEADER_TABLE_SIZE = HQ_SETTINGS_MASK | 1, -- use HEADER_TABLE_SIZE
  //_HQ_MAX_HEADER_LIST_SIZE = HQ_SETTINGS_MASK | 6, -- use MAX_HEADER_LIST_SIZE
  _HQ_QPACK_BLOCKED_STREAMS = HQ_SETTINGS_MASK | 7,
  _HQ_DATAGRAM = HQ_SETTINGS_MASK | 0x0276,
  _HQ_DATAGRAM_DRAFT_8 = HQ_SETTINGS_MASK | 0xffd277,
  _HQ_DATAGRAM_RFC = HQ_SETTINGS_MASK | 0x33,
  ENABLE_WEBTRANSPORT = 0x2b603742,
  WEBTRANSPORT_MAX_SESSIONS = 0x2b603743
};

using SettingPair = std::pair<SettingsId, uint32_t>;

} // namespace proxygen
