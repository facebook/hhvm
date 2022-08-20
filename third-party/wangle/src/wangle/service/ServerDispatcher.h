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

#include <wangle/channel/Handler.h>
#include <wangle/service/Service.h>

namespace wangle {

/**
 * Dispatch requests from pipeline one at a time synchronously.
 * Concurrent requests are queued in the pipeline.
 */
template <typename Req, typename Resp = Req>
class SerialServerDispatcher : public HandlerAdapter<Req, Resp> {
 public:
  typedef typename HandlerAdapter<Req, Resp>::Context Context;

  explicit SerialServerDispatcher(Service<Req, Resp>* service)
      : service_(service) {}

  void read(Context* ctx, Req in) override {
    auto resp = (*service_)(std::move(in)).get();
    ctx->fireWrite(std::move(resp));
  }

 private:
  Service<Req, Resp>* service_;
};

/**
 * Dispatch requests from pipeline as they come in.
 * Responses are queued until they can be sent in order.
 */
template <typename Req, typename Resp = Req>
class PipelinedServerDispatcher : public HandlerAdapter<Req, Resp> {
 public:
  typedef typename HandlerAdapter<Req, Resp>::Context Context;

  explicit PipelinedServerDispatcher(Service<Req, Resp>* service)
      : service_(service) {}

  void read(Context*, Req in) override {
    auto requestId = requestId_++;
    (*service_)(std::move(in)).then([requestId, this](Resp& resp) {
      responses_[requestId] = resp;
      sendResponses();
    });
  }

  void sendResponses() {
    auto search = responses_.find(lastWrittenId_ + 1);
    while (search != responses_.end()) {
      Resp resp = std::move(search->second);
      responses_.erase(search->first);
      this->getContext()->fireWrite(std::move(resp));
      lastWrittenId_++;
      search = responses_.find(lastWrittenId_ + 1);
    }
  }

 private:
  Service<Req, Resp>* service_;
  uint32_t requestId_{1};
  std::unordered_map<uint32_t, Resp> responses_;
  uint32_t lastWrittenId_{0};
};

/**
 * Dispatch requests from pipeline as they come in.  Concurrent
 * requests are assumed to have sequence id's that are taken care of
 * by the pipeline.  Unlike a multiplexed client dispatcher, a
 * multiplexed server dispatcher needs no state, and the sequence id's
 * can just be copied from the request to the response in the pipeline.
 */
template <typename Req, typename Resp = Req>
class MultiplexServerDispatcher : public HandlerAdapter<Req, Resp> {
 public:
  typedef typename HandlerAdapter<Req, Resp>::Context Context;

  explicit MultiplexServerDispatcher(Service<Req, Resp>* service)
      : service_(service) {}

  void read(Context* ctx, Req in) override {
    (*service_)(std::move(in)).thenValue([ctx](Resp resp) {
      ctx->fireWrite(std::move(resp));
    });
  }

 private:
  Service<Req, Resp>* service_;
};

} // namespace wangle
