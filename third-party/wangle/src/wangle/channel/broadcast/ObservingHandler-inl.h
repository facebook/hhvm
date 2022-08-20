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

namespace wangle {

template <typename T, typename R, typename P>
ObservingHandler<T, R, P>::ObservingHandler(
    const R& routingData,
    BroadcastPool<T, R, P>* broadcastPool)
    : routingData_(routingData), broadcastPool_(CHECK_NOTNULL(broadcastPool)) {}

template <typename T, typename R, typename P>
ObservingHandler<T, R, P>::~ObservingHandler() {
  if (broadcastHandler_) {
    auto broadcastHandler = broadcastHandler_;
    broadcastHandler_ = nullptr;
    broadcastHandler->unsubscribe(subscriptionId_);
  }

  if (deleted_) {
    *deleted_ = true;
  }
}

template <typename T, typename R, typename P>
void ObservingHandler<T, R, P>::transportActive(Context* ctx) {
  if (broadcastHandler_) {
    // Already connected
    return;
  }

  // Pause ingress until the remote connection is established and
  // broadcast handler is ready
  auto pipeline = dynamic_cast<ObservingPipeline<T>*>(ctx->getPipeline());
  CHECK(pipeline);
  pipeline->transportInactive();

  auto deleted = deleted_;
  broadcastPool_->getHandler(routingData_)
      .thenValue(
          [this, pipeline, deleted](BroadcastHandler<T, R>* broadcastHandler) {
            if (*deleted) {
              return;
            }

            broadcastHandler_ = broadcastHandler;
            subscriptionId_ = broadcastHandler_->subscribe(this);
            VLOG(10) << "Subscribed to a broadcast";

            // Resume ingress
            pipeline->transportActive();
          })
      .thenError(
          folly::tag_t<std::exception>{},
          [this, ctx, deleted](const std::exception& ex) {
            if (*deleted) {
              return;
            }

            LOG(ERROR) << "Error subscribing to a broadcast: " << ex.what();
            this->close(ctx);
          });
}

template <typename T, typename R, typename P>
void ObservingHandler<T, R, P>::readEOF(Context* ctx) {
  this->close(ctx);
}

template <typename T, typename R, typename P>
void ObservingHandler<T, R, P>::readException(
    Context* ctx,
    folly::exception_wrapper ex) {
  LOG(ERROR) << "Error on read: " << exceptionStr(ex);
  this->close(ctx);
}

template <typename T, typename R, typename P>
void ObservingHandler<T, R, P>::onNext(const T& data) {
  auto ctx = this->getContext();
  auto deleted = deleted_;
  this->write(ctx, data).thenError(
      folly::tag_t<std::exception>{},
      [this, ctx, deleted](const std::exception& ex) {
        if (*deleted) {
          return;
        }

        LOG(ERROR) << "Error on write: " << ex.what();
        this->close(ctx);
      });
}

template <typename T, typename R, typename P>
void ObservingHandler<T, R, P>::onError(folly::exception_wrapper ex) {
  LOG(ERROR) << "Error observing a broadcast: " << exceptionStr(ex);

  // broadcastHandler_ will clear its subscribers and delete itself
  broadcastHandler_ = nullptr;
  this->close(this->getContext());
}

template <typename T, typename R, typename P>
void ObservingHandler<T, R, P>::onCompleted() {
  // broadcastHandler_ will clear its subscribers and delete itself
  broadcastHandler_ = nullptr;
  this->close(this->getContext());
}

template <typename T, typename R, typename P>
R& ObservingHandler<T, R, P>::routingData() {
  return routingData_;
}

} // namespace wangle
