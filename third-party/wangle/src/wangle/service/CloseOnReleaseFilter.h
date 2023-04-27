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

#include <wangle/service/Service.h>

namespace wangle {

/**
 * A service that rejects all requests after its 'close' method has
 * been invoked.
 */
template <typename Req, typename Resp = Req>
class CloseOnReleaseFilter : public ServiceFilter<Req, Resp> {
 public:
  explicit CloseOnReleaseFilter(std::shared_ptr<Service<Req, Resp>> service)
      : ServiceFilter<Req, Resp>(service) {}

  folly::Future<Resp> operator()(Req req) override {
    if (!released) {
      return (*this->service_)(std::move(req));
    } else {
      return folly::makeFuture<Resp>(
          folly::make_exception_wrapper<std::runtime_error>("Service Closed"));
    }
  }

  folly::Future<folly::Unit> close() override {
    if (!released.exchange(true)) {
      return this->service_->close();
    } else {
      return folly::makeFuture();
    }
  }

 private:
  std::atomic<bool> released{false};
};

} // namespace wangle
