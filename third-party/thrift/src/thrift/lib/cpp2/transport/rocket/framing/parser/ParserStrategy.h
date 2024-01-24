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

#include <folly/io/IOBuf.h>
#include <folly/io/IOBufQueue.h>

namespace apache {
namespace thrift {
namespace rocket {

// C++20 concept
// template <typename T>
// concept ParserStrategyConcept = requires(T t, void** bufReturn, size_t*
// lenReturn, std::unique_ptr<folly::IOBuf> buf) {
//     { t.getReadBuffer(bufReturn, lenReturn) } -> std::same_as<void>;
//     { t.readDataAvailable(lenReturn) } -> std::same_as<void>;
//     { t.readBufferAvailable(std::move(buf)) } -> std::same_as<void>;
// };
template <class T, template <typename...> class Strategy, typename... Args>
class ParserStrategy : private Strategy<T, Args...> {
 public:
  using Strategy<T, Args...>::Strategy;

  void getReadBuffer(void** bufReturn, size_t* lenReturn) {
    Strategy<T, Args...>::getReadBuffer(bufReturn, lenReturn);
  }

  void readDataAvailable(size_t len) {
    Strategy<T, Args...>::readDataAvailable(len);
  }

  void readBufferAvailable(std::unique_ptr<folly::IOBuf> buf) {
    Strategy<T, Args...>::readBufferAvailable(std::move(buf));
  }
};

} // namespace rocket
} // namespace thrift
} // namespace apache
