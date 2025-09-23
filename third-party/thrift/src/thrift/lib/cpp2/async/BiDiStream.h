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

#include <folly/Function.h>
#include <folly/coro/AsyncGenerator.h>
#include <folly/coro/Task.h>

namespace apache::thrift {

template <typename InputElement, typename OutputElement>
struct StreamTransformation {
  using InputType = folly::coro::AsyncGenerator<InputElement&&>;
  using OutputType = folly::coro::AsyncGenerator<OutputElement&&>;
  using Func = folly::Function<OutputType(InputType)>;

  Func func;
};

template <typename Response, typename InputElement, typename OutputElement>
struct ResponseAndStreamTransformation {
  Response response;
  StreamTransformation<InputElement, OutputElement> transform;
};

} // namespace apache::thrift
