/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
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

#include <folly/futures/Future.h>
#include <wangle/channel/Handler.h>

namespace wangle {

class EventBaseHandler : public OutboundBytesToBytesHandler {
 public:
  folly::Future<folly::Unit> write(
      Context* ctx,
      std::unique_ptr<folly::IOBuf> buf) override {
    folly::Future<folly::Unit> retval;
    DCHECK(ctx->getTransport());
    DCHECK(ctx->getTransport()->getEventBase());
    ctx->getTransport()
        ->getEventBase()
        ->runImmediatelyOrRunInEventBaseThreadAndWait(
            [&]() { retval = ctx->fireWrite(std::move(buf)); });
    return retval;
  }

  folly::Future<folly::Unit> close(Context* ctx) override {
    DCHECK(ctx->getTransport());
    DCHECK(ctx->getTransport()->getEventBase());
    folly::Future<folly::Unit> retval;
    ctx->getTransport()
        ->getEventBase()
        ->runImmediatelyOrRunInEventBaseThreadAndWait(
            [&]() { retval = ctx->fireClose(); });
    return retval;
  }
};

} // namespace wangle
