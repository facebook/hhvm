/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <boost/variant.hpp>

#include <fizz/protocol/Actions.h>
#include <fizz/util/Variant.h>
#include <folly/Optional.h>
#include <folly/futures/Future.h>
#include <folly/small_vector.h>

namespace fizz {
namespace server {

class State;

/**
 * A lambda that should be invoked on State so that modification can be applied.
 */
using MutateState = folly::Function<void(State&)>;

struct AttemptVersionFallback {
  /**
   * The ClientHello received with a TLS version lower than TLSv1.3.
   */
  std::unique_ptr<folly::IOBuf> clientHello;

  /**
   * If set, the SNI that was conveyed in the ClientHello.
   * Helps downstream implementations to set right context
   * for processing the ClientHello.
   */
  folly::Optional<std::string> sni;
};

/**
 * Reports that early data was received and accepted. Application data delivered
 * after ReportEarlyHandshakeSuccess but before ReportHandshakeSuccess was
 * received using the early cipher.
 */
struct ReportEarlyHandshakeSuccess {};

/**
 * Reports that the full handshake has completed successfully.
 */
struct ReportHandshakeSuccess {};

#define FIZZ_SERVER_ACTIONS(F, ...)           \
  F(DeliverAppData, __VA_ARGS__)              \
  F(WriteToSocket, __VA_ARGS__)               \
  F(ReportHandshakeSuccess, __VA_ARGS__)      \
  F(ReportEarlyHandshakeSuccess, __VA_ARGS__) \
  F(ReportError, __VA_ARGS__)                 \
  F(EndOfData, __VA_ARGS__)                   \
  F(MutateState, __VA_ARGS__)                 \
  F(WaitForData, __VA_ARGS__)                 \
  F(AttemptVersionFallback, __VA_ARGS__)      \
  F(SecretAvailable, __VA_ARGS__)

FIZZ_DECLARE_VARIANT_TYPE(Action, FIZZ_SERVER_ACTIONS)

using Actions = folly::small_vector<Action, 4>;
using AsyncActions = boost::variant<Actions, folly::SemiFuture<Actions>>;

namespace detail {

template <typename... Args>
Actions actions(Args&&... act) {
  Actions acts;
  fizz::detail::addAction(acts, std::forward<Args>(act)...);
  return acts;
}
} // namespace detail
} // namespace server
} // namespace fizz
