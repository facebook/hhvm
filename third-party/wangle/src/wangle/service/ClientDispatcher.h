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

template <typename Pipeline, typename Req, typename Resp = Req>
class ClientDispatcherBase : public HandlerAdapter<Resp, Req>,
                             public Service<Req, Resp> {
 public:
  typedef typename HandlerAdapter<Resp, Req>::Context Context;

  ~ClientDispatcherBase() override {
    if (pipeline_) {
      try {
        pipeline_->remove(this).finalize();
      } catch (const std::invalid_argument&) {
        // not in pipeline; this is fine
      }
    }
  }

  void setPipeline(Pipeline* pipeline) {
    try {
      pipeline->template remove<ClientDispatcherBase>();
    } catch (const std::invalid_argument&) {
      // no existing dispatcher; this is fine
    }
    pipeline_ = pipeline;
    pipeline_->addBack(this);
    pipeline_->finalize();
  }

  folly::Future<folly::Unit> close() override {
    return HandlerAdapter<Resp, Req>::close(this->getContext());
  }

  folly::Future<folly::Unit> close(Context* ctx) override {
    return HandlerAdapter<Resp, Req>::close(ctx);
  }

 protected:
  Pipeline* pipeline_{nullptr};
};

/**
 * Dispatch a request, satisfying Promise `p` with the response;
 * the returned Future is satisfied when the response is received:
 * only one request is allowed at a time.
 */
template <typename Pipeline, typename Req, typename Resp = Req>
class SerialClientDispatcher
    : public ClientDispatcherBase<Pipeline, Req, Resp> {
 public:
  typedef typename HandlerAdapter<Resp, Req>::Context Context;

  void read(Context*, Resp in) override {
    DCHECK(p_);
    p_->setValue(std::move(in));
    p_ = folly::none;
  }

  folly::Future<Resp> operator()(Req arg) override {
    CHECK(!p_);
    DCHECK(this->pipeline_);

    p_ = folly::Promise<Resp>();
    auto f = p_->getFuture();
    this->pipeline_->write(std::move(arg));
    return f;
  }

 private:
  folly::Optional<folly::Promise<Resp>> p_;
};

/**
 * Dispatch a request, satisfying Promise `p` with the response;
 * the returned Future is satisfied when the response is received.
 * A deque of promises/futures are mantained for pipelining.
 */
template <typename Pipeline, typename Req, typename Resp = Req>
class PipelinedClientDispatcher
    : public ClientDispatcherBase<Pipeline, Req, Resp> {
 public:
  typedef typename HandlerAdapter<Resp, Req>::Context Context;

  void read(Context*, Resp in) override {
    DCHECK(p_.size() >= 1);
    auto p = std::move(p_.front());
    p_.pop_front();
    p.setValue(std::move(in));
  }

  folly::Future<Resp> operator()(Req arg) override {
    DCHECK(this->pipeline_);

    folly::Promise<Resp> p;
    auto f = p.getFuture();
    p_.push_back(std::move(p));
    this->pipeline_->write(std::move(arg));
    return f;
  }

 private:
  std::deque<folly::Promise<Resp>> p_;
};

/*
 * A full out-of-order request/response client would require some sort
 * of sequence id on the wire.  Currently this is left up to
 * individual protocol writers to implement.
 */

} // namespace wangle
