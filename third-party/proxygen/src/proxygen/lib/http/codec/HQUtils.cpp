/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <proxygen/lib/http/codec/HQUtils.h>

#include <folly/Overload.h>

namespace proxygen { namespace hq {

const quic::StreamId kSessionStreamId = std::numeric_limits<uint64_t>::max();

// Ingress Sett.hngs Default Values
const uint64_t kDefaultIngressHeaderTableSize = 0;
const uint64_t kDefaultIngressNumPlaceHolders = 0;
const uint64_t kDefaultIngressMaxHeaderListSize = 1u << 17;
const uint64_t kDefaultIngressQpackBlockedStream = 0;

// Egress Settings Default Values
const uint64_t kDefaultEgressHeaderTableSize = 4096;
const uint64_t kDefaultEgressNumPlaceHolders = 16;
const uint64_t kDefaultEgressMaxHeaderListSize = 1u << 17;
const uint64_t kDefaultEgressQpackBlockedStream = 100;

proxygen::ErrorCode hqToHttpErrorCode(HTTP3::ErrorCode err) {
  switch (err) {
    case HTTP3::ErrorCode::HTTP_NO_ERROR:
      return ErrorCode::NO_ERROR;
    case HTTP3::ErrorCode::HTTP_REQUEST_REJECTED:
      return ErrorCode::REFUSED_STREAM;
    case HTTP3::ErrorCode::HTTP_INTERNAL_ERROR:
      return ErrorCode::INTERNAL_ERROR;
    case HTTP3::ErrorCode::HTTP_REQUEST_CANCELLED:
      return ErrorCode::CANCEL;
    case HTTP3::ErrorCode::HTTP_CONNECT_ERROR:
      return ErrorCode::CONNECT_ERROR;
    case HTTP3::ErrorCode::HTTP_EXCESSIVE_LOAD:
      return ErrorCode::ENHANCE_YOUR_CALM;
    case HTTP3::ErrorCode::HTTP_VERSION_FALLBACK:
      return ErrorCode::INTERNAL_ERROR;
    case HTTP3::ErrorCode::HTTP_CLOSED_CRITICAL_STREAM:
    case HTTP3::ErrorCode::HTTP_MISSING_SETTINGS:
    case HTTP3::ErrorCode::HTTP_FRAME_UNEXPECTED:
    case HTTP3::ErrorCode::HTTP_STREAM_CREATION_ERROR:
    case HTTP3::ErrorCode::HTTP_FRAME_ERROR:
    case HTTP3::ErrorCode::HTTP_ID_ERROR:
    case HTTP3::ErrorCode::HTTP_SETTINGS_ERROR:
    case HTTP3::ErrorCode::HTTP_INCOMPLETE_REQUEST:
    case HTTP3::ErrorCode::HTTP_MESSAGE_ERROR:
      return ErrorCode::PROTOCOL_ERROR;
    default:
      return ErrorCode::INTERNAL_ERROR;
  }
}

ProxygenError toProxygenError(quic::QuicErrorCode error, bool fromPeer) {
  switch (error.type()) {
    case quic::QuicErrorCode::Type::ApplicationErrorCode: {
      auto h3error = HTTP3::ErrorCode(*error.asApplicationErrorCode());
      if (h3error == HTTP3::HTTP_NO_ERROR) {
        return kErrorNone;
      } else if (isQPACKError(h3error)) {
        return kErrorBadDecompress;
      } else if (fromPeer) {
        return kErrorConnectionReset;
      } else {
        return kErrorConnection;
      }
    }
    case quic::QuicErrorCode::Type::LocalErrorCode:
      return kErrorShutdown;
    case quic::QuicErrorCode::Type::TransportErrorCode:
      return kErrorConnectionReset;
  }
  folly::assume_unreachable();
}

folly::Optional<hq::SettingId> httpToHqSettingsId(proxygen::SettingsId id) {
  switch (id) {
    case proxygen::SettingsId::HEADER_TABLE_SIZE:
      return hq::SettingId::HEADER_TABLE_SIZE;
    case proxygen::SettingsId::MAX_HEADER_LIST_SIZE:
      return hq::SettingId::MAX_HEADER_LIST_SIZE;
    case proxygen::SettingsId::ENABLE_CONNECT_PROTOCOL:
      return hq::SettingId::ENABLE_CONNECT_PROTOCOL;
    case proxygen::SettingsId::_HQ_QPACK_BLOCKED_STREAMS:
      return hq::SettingId::QPACK_BLOCKED_STREAMS;
    case proxygen::SettingsId::_HQ_DATAGRAM:
      return hq::SettingId::H3_DATAGRAM;
    case proxygen::SettingsId::_HQ_DATAGRAM_DRAFT_8:
      return hq::SettingId::H3_DATAGRAM_DRAFT_8;
    case proxygen::SettingsId::_HQ_DATAGRAM_RFC:
      return hq::SettingId::H3_DATAGRAM_RFC;
    case proxygen::SettingsId::ENABLE_WEBTRANSPORT:
      return hq::SettingId::ENABLE_WEBTRANSPORT;
    case proxygen::SettingsId::WEBTRANSPORT_MAX_SESSIONS:
      return hq::SettingId::WEBTRANSPORT_MAX_SESSIONS;
    default:
      return folly::none; // this setting has no meaning in HQ
  }
  return folly::none;
}

folly::Optional<proxygen::SettingsId> hqToHttpSettingsId(hq::SettingId id) {
  switch (id) {
    case hq::SettingId::HEADER_TABLE_SIZE:
      return proxygen::SettingsId::HEADER_TABLE_SIZE;
    case hq::SettingId::MAX_HEADER_LIST_SIZE:
      return proxygen::SettingsId::MAX_HEADER_LIST_SIZE;
    case hq::SettingId::ENABLE_CONNECT_PROTOCOL:
      return proxygen::SettingsId::ENABLE_CONNECT_PROTOCOL;
    case hq::SettingId::QPACK_BLOCKED_STREAMS:
      return proxygen::SettingsId::_HQ_QPACK_BLOCKED_STREAMS;
    case hq::SettingId::H3_DATAGRAM:
      return proxygen::SettingsId::_HQ_DATAGRAM;
    case hq::SettingId::H3_DATAGRAM_DRAFT_8:
      return proxygen::SettingsId::_HQ_DATAGRAM_DRAFT_8;
    case hq::SettingId::H3_DATAGRAM_RFC:
      return proxygen::SettingsId::_HQ_DATAGRAM_RFC;
    case hq::SettingId::ENABLE_WEBTRANSPORT:
      return proxygen::SettingsId::ENABLE_WEBTRANSPORT;
    case hq::SettingId::WEBTRANSPORT_MAX_SESSIONS:
      return proxygen::SettingsId::WEBTRANSPORT_MAX_SESSIONS;
  }
  return folly::none;
}

}} // namespace proxygen::hq
