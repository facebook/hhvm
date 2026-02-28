/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <fizz/client/PskCache.h>
#include <fizz/protocol/Actions.h>
#include <fizz/protocol/Params.h>
#include <fizz/util/Variant.h>
#include <folly/CPortability.h>

namespace fizz {
namespace client {

class State;

/**
 * A lambda that should be invoked on State so that modification can be applied.
 */
using MutateState = folly::Function<void(State&)>;

/**
 * Reports that the early data cipher is available.
 *
 * After receiving ReportEarlyHandshakeSuccess, the application may write data
 * with EarlyAppWrite. The application is responsible for not exceeding
 * maxEarlyDataSize.
 */
struct ReportEarlyHandshakeSuccess {
  uint32_t maxEarlyDataSize{0};
};

/**
 * Reports that the full handshake has completed successfully. If
 * earlyDataAccepted is true, the server accepted early data that was sent.
 * If it is false, and early data was attempted, all the early data was rejected
 * and was lost. The application is responsible for handling the loss of early
 * data.
 */
struct ReportHandshakeSuccess {
  bool earlyDataAccepted{false};
};

/**
 * Reports that an early app write was attempted when early data was already
 * rejected. This action will not be received when early data is sent on the
 * transport and later rejected, only when the early data is not even sent on
 * the transport.
 *
 * The application is responsible for invoking the failed write's write
 * callback.
 */
struct ReportEarlyWriteFailed {
  EarlyAppWrite write;
};

/**
 * Reports that there is a new psk sent by the server to cache on the client.
 *
 * The application is responsible for caching the new psk.
 */
struct NewCachedPsk {
  CachedPsk psk;
};

/**
 * New ech retry config available. This action is emitted whenever an ECH retry
 * config is received from the server's encrypted extensions.
 */
struct ECHRetryAvailable {
  /**
   * The original SNI that was used in the `Connect` event, prior to any
   * modifications due to ECH.
   *
   * It is present here in order to associate this `ECHRetryAvailable`
   * action with a connection attempt.
   *
   * An empty string indicates that no SNI was sent, but the peer responded
   * with a set of ECHConfigs regardless
   */
  std::string sni;
  /**
   * A list of ECHConfigs sent by the peer. It is intended to indicate
   * the set of acceptable ECHConfigs to use the next time the local
   * sender intends to open a TLS connection to `sni`.
   */
  std::vector<ech::ParsedECHConfig> configs;
};

#define FIZZ_CLIENT_ACTIONS(F, ...)           \
  F(DeliverAppData, __VA_ARGS__)              \
  F(WriteToSocket, __VA_ARGS__)               \
  F(ReportHandshakeSuccess, __VA_ARGS__)      \
  F(ReportEarlyHandshakeSuccess, __VA_ARGS__) \
  F(ReportEarlyWriteFailed, __VA_ARGS__)      \
  F(ReportError, __VA_ARGS__)                 \
  F(EndOfData, __VA_ARGS__)                   \
  F(MutateState, __VA_ARGS__)                 \
  F(WaitForData, __VA_ARGS__)                 \
  F(NewCachedPsk, __VA_ARGS__)                \
  F(SecretAvailable, __VA_ARGS__)             \
  F(ECHRetryAvailable, __VA_ARGS__)

FIZZ_DECLARE_VARIANT_TYPE(Action, FIZZ_CLIENT_ACTIONS)

#if FOLLY_MOBILE
using Actions = std::vector<Action>;
#else
using Actions = folly::small_vector<Action, 4>;
#endif

namespace detail {

template <typename... Args>
FOLLY_ERASE Actions actions(Args&&... act) {
  Actions acts;
  fizz::detail::addAction(acts, std::forward<Args>(act)...);
  return acts;
}
} // namespace detail
} // namespace client
} // namespace fizz
