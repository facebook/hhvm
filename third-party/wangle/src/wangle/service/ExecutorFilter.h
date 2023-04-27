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
 * A service that runs all requests through an executor.
 */
template <typename Req, typename Resp = Req>
class ExecutorFilter : public ServiceFilter<Req, Resp> {
 public:
  explicit ExecutorFilter(
      std::shared_ptr<folly::Executor> exe,
      std::shared_ptr<Service<Req, Resp>> service)
      : ServiceFilter<Req, Resp>(service), exe_(exe) {}

  folly::Future<Resp> operator()(Req req) override {
    return via(exe_.get())
        .thenValue([req = std::move(req), this](auto&&) mutable {
          return (*this->service_)(std::move(req));
        });
  }

 private:
  std::shared_ptr<folly::Executor> exe_;
};

} // namespace wangle
