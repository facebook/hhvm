/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include "proxygen/lib/http/coro/HTTPStreamSource.h"

namespace proxygen::coro {

class HTTPStreamSourceHolder
    : public HTTPStreamSource::Callback
    , public std::enable_shared_from_this<HTTPStreamSourceHolder> {
 public:
  using Ptr = std::shared_ptr<HTTPStreamSourceHolder>;
  static Ptr make(folly::EventBase* evb,
                  folly::Optional<HTTPCodec::StreamID> id = folly::none,
                  uint32_t egressBufferSize = 65535) {
    return std::shared_ptr<HTTPStreamSourceHolder>(
        new HTTPStreamSourceHolder(evb, id, egressBufferSize));
  }

  ~HTTPStreamSourceHolder() override = default;

  void start() {
    libRef_ = shared_from_this();
  }

  void sourceComplete(HTTPCodec::StreamID /*id*/,
                      folly::Optional<HTTPError> /*error*/) override {
    /**
     * ::sourceComplete is guaranteed to run inside HTTPStreamSource evb, so we
     * can safely destruct here.
     *
     * WARNING: this is class is not thread safe and should not be used by
     * different consumer and producer threads; setting sourceComplete_ flag and
     * destructing source_ are not atomic operations.
     */
    source_.reset();
    libRef_.reset();
  }

  HTTPStreamSource* get() {
    return source_.has_value() ? &source_.value() : nullptr;
  }

 private:
  HTTPStreamSourceHolder(folly::EventBase* evb,
                         folly::Optional<HTTPCodec::StreamID> id,
                         uint32_t egressBufferSize)
      : source_(std::in_place, evb, id, this, egressBufferSize) {
  }
  std::optional<HTTPStreamSource> source_;
  std::shared_ptr<HTTPStreamSourceHolder> libRef_;
};

} // namespace proxygen::coro
