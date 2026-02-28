/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <proxygen/lib/http/session/HTTPTransaction.h>
#include <proxygen/lib/utils/ConditionalGate.h>

#include "proxygen/lib/http/coro/HTTPSourceHolder.h"
#include "proxygen/lib/http/coro/HTTPStreamSource.h"

namespace proxygen::coro {
/*
 * This class creates an adaptor for an HTTPTransaction to simplify migrations
 * to the Coro API.
 *
 * It behaves as a transaction handler but drives an ingress stream source that
 * can be used with a coroutine handler.
 *
 * An egress source can be set to drive the egress of the transaction.
 */
class HTTPTransactionAdaptorSource
    : public HTTPTransactionHandler
    , public HTTPStreamSource::Callback {
 public:
  static HTTPTransactionAdaptorSource* create(folly::EventBase* evb);
  ~HTTPTransactionAdaptorSource() override;

  HTTPSource* getIngressSource();

  void setEgressSource(HTTPSourceHolder egressSource);
  folly::CancellationToken getCancelToken();

 private:
  explicit HTTPTransactionAdaptorSource(folly::EventBase* evb);

  // HTTPTransactionHandler callback.
  void setTransaction(HTTPTransaction* txn) noexcept override;
  void detachTransaction() noexcept override;
  void onHeadersComplete(std::unique_ptr<HTTPMessage> msg) noexcept override;
  void onBody(std::unique_ptr<folly::IOBuf> chain) noexcept override;
  void onTrailers(std::unique_ptr<HTTPHeaders> trailers) noexcept override;
  void onEOM() noexcept override;
  void onUpgrade(UpgradeProtocol protocol) noexcept override;
  void onError(const HTTPException& error) noexcept override;
  void onEgressPaused() noexcept override;
  void onEgressResumed() noexcept override;

  bool hasTransaction() const;

  void windowOpen(HTTPCodec::StreamID /*id*/) override;
  void sourceComplete(HTTPCodec::StreamID /*id*/,
                      folly::Optional<HTTPError> /*error*/) override;

  void startEgressLoop();
  folly::coro::Task<void> egressLoop();
  void requestCancellation();

  folly::EventBase* evb_;
  folly::CancellationSource cancelSource_;
  HTTPTransaction* txn_{nullptr};

  HTTPStreamSource ingressSource_;
  HTTPStreamSource::FlowControlState windowState_{
      HTTPStreamSource::FlowControlState::OPEN};

  HTTPSourceHolder egressSource_;
  TimedBaton egressResumed_;

  enum class Event { EgressComplete, IngressComplete };
  ConditionalGate<Event, 2> gate_;
};

} // namespace proxygen::coro
