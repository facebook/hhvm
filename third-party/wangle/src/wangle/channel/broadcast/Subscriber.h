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

#include <folly/ExceptionWrapper.h>

namespace wangle {

/**
 * Subscriber interface for listening to a stream.
 */
template <typename T, typename R>
class Subscriber {
 public:
  virtual ~Subscriber() = default;

  virtual void onNext(const T&) = 0;
  virtual void onError(folly::exception_wrapper ex) = 0;
  virtual void onCompleted() = 0;
  virtual R& routingData() = 0;
};

} // namespace wangle
