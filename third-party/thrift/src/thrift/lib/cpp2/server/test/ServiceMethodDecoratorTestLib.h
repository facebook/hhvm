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
#include <string>

namespace apache::thrift::test {

struct AdaptedEcho {
  std::string adaptedText;
};

template <typename T>
struct EchoRequestAdapter {
  static AdaptedEcho fromThrift(const T& echoRequest) {
    return {*echoRequest.text()};
  }
  static T toThrift(const AdaptedEcho& adaptedEcho) {
    T echoRequest;
    echoRequest.text() = adaptedEcho.adaptedText;
    return echoRequest;
  }
};

struct AdaptedEchoResponse {
  std::string adaptedResponseText;
};

template <typename T>
struct EchoResponseAdapter {
  static AdaptedEchoResponse fromThrift(const T& echoRequest) {
    return {*echoRequest.text()};
  }
  static T toThrift(const AdaptedEchoResponse& adaptedEcho) {
    T echoRequest;
    echoRequest.text() = adaptedEcho.adaptedResponseText;
    return echoRequest;
  }
};

} // namespace apache::thrift::test
