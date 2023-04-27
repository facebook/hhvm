/*
 *  Copyright (c) 2019-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */
#pragma once

#include <fizz/protocol/Types.h>
#include <fizz/protocol/clock/SystemClock.h>
#include <fizz/server/ResumptionState.h>

namespace fizz {
namespace server {

class TicketPolicy {
 public:
  TicketPolicy() : clock_(std::make_shared<SystemClock>()) {}

  ~TicketPolicy() = default;

  /**
   * These two settings control the ticket's validity period. The handshake
   * validity refers to how long a ticket is considered valid from the initial
   * full handshake that authenticated it. This time carries over when a new
   * ticket is issued on a resumed connection. In practice, this means a full
   * handshake will be forced when a ticket's handshake is considered stale.
   *
   * A given ticket's ticket_lifetime is the remaining handshake validity
   * period, capped at the configured ticket validity.
   */

  void setHandshakeValidity(std::chrono::seconds validity) {
    handshakeValidity_ = validity;
  }

  void setTicketValidity(std::chrono::seconds validity) {
    ticketValidity_ = validity;
  }

  void setClock(std::shared_ptr<Clock> clock) {
    clock_ = std::move(clock);
  }

  std::chrono::seconds remainingValidity(
      const ResumptionState& resState) const {
    // If handshake is in future, validity computed from resState will be
    // longer than the actual validity period. Ticket shouldn't be reused after
    // the ticketValidity_ period either.
    return std::min(
        {getValidity(resState), handshakeValidity_, ticketValidity_});
  }

  bool shouldAccept(ResumptionState& resState) const {
    return getValidity(resState) > std::chrono::system_clock::duration::zero();
  }

 private:
  std::chrono::seconds getValidity(const ResumptionState& resState) const {
    return std::chrono::duration_cast<std::chrono::seconds>(
        resState.handshakeTime + handshakeValidity_ - clock_->getCurrentTime());
  }

  std::chrono::seconds ticketValidity_{std::chrono::hours(1)};
  std::chrono::seconds handshakeValidity_{std::chrono::hours(72)};
  std::shared_ptr<Clock> clock_;
};
} // namespace server
} // namespace fizz
