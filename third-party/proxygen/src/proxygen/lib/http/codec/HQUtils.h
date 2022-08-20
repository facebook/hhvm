/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <deque>
#include <folly/Optional.h>
#include <proxygen/lib/http/HTTP3ErrorCode.h>
#include <proxygen/lib/http/HTTPException.h>
#include <proxygen/lib/http/codec/ErrorCode.h>
#include <proxygen/lib/http/codec/HQFramer.h>
#include <proxygen/lib/http/codec/HTTPSettings.h>
#include <quic/codec/Types.h>

namespace proxygen { namespace hq {

// stream ID used for session-level callbacks from the codec
extern const quic::StreamId kSessionStreamId;

// Ingress Settings Default Values
extern const uint64_t kDefaultIngressHeaderTableSize;
extern const uint64_t kDefaultIngressNumPlaceHolders;
extern const uint64_t kDefaultIngressMaxHeaderListSize;
extern const uint64_t kDefaultIngressQpackBlockedStream;

// Settings Default Values
extern const uint64_t kDefaultEgressHeaderTableSize;
extern const uint64_t kDefaultEgressNumPlaceHolders;
extern const uint64_t kDefaultEgressMaxHeaderListSize;
extern const uint64_t kDefaultEgressQpackBlockedStream;

// The maximum client initiated bidirectional stream id in a quic varint
constexpr uint64_t kMaxClientBidiStreamId = quic::kEightByteLimit - 3;
// The maximum server initiated push id in a quic varint
constexpr uint64_t kMaxPushId = quic::kEightByteLimit - 1;

proxygen::ErrorCode hqToHttpErrorCode(HTTP3::ErrorCode err);

/**
 * Conver a quic error to the appropriate proxygen error.
 *
 * Our convention is:
 *  Application error from peer -> kErrorConnectionReset
 *  Application error generated locally -> kErrorConnection
 *  Transport error (must be from peer?) -> kErrorConnectionReset
 *  Local error -> kErrorShutdown
 */
ProxygenError toProxygenError(quic::QuicErrorCode error, bool fromPeer = false);

folly::Optional<hq::SettingId> httpToHqSettingsId(proxygen::SettingsId id);

folly::Optional<proxygen::SettingsId> hqToHttpSettingsId(hq::SettingId id);

}} // namespace proxygen::hq
