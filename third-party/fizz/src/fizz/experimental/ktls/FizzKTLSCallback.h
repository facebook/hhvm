/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#pragma once

#include <fizz/experimental/ktls/AsyncKTLSSocket.h>
#include <fizz/protocol/KeyScheduler.h>
#include <folly/Function.h>

namespace fizz {

class KTLSCallbackImpl : public AsyncKTLSSocket::TLSCallback {
 public:
  using TicketHandler =
      folly::Function<void(KeyScheduler& ks, NewSessionTicket nst)>;
  KTLSCallbackImpl(
      std::unique_ptr<KeyScheduler> keyScheduler,
      TicketHandler&& ticketHandler)
      : keyScheduler_(std::move(keyScheduler)),
        ticketHandler_(std::move(ticketHandler)) {}

  void receivedNewSessionTicket(
      AsyncKTLSSocket* sock,
      fizz::NewSessionTicket ticket) override;

 private:
  std::unique_ptr<KeyScheduler> keyScheduler_;
  TicketHandler ticketHandler_;
};

} // namespace fizz
