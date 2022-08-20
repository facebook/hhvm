/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <fizz/server/ResumptionState.h>
#include <folly/Optional.h>
#include <folly/futures/Future.h>
#include <folly/io/IOBuf.h>

namespace fizz {
namespace server {

/**
 * Interface for turning PSKs into ResumptionState, and vice versa.
 */
class TicketCipher {
 public:
  virtual ~TicketCipher() = default;

  /**
   * Returns an opaque PSK for ResumptionState, and its validity time.
   */
  virtual folly::SemiFuture<
      folly::Optional<std::pair<Buf, std::chrono::seconds>>>
  encrypt(ResumptionState resState) const = 0;

  /**
   * Returns the ResumptionState for an opaque PSK, and the type of PSK
   * (resumption or external).
   *
   * Returns Rejected if the PSK is not recognized or not valid.
   */
  virtual folly::SemiFuture<
      std::pair<PskType, folly::Optional<ResumptionState>>>
  decrypt(std::unique_ptr<folly::IOBuf> encryptedTicket) const = 0;
};
} // namespace server
} // namespace fizz
