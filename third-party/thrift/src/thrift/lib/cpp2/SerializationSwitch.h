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

#include <folly/Utility.h>
#include <thrift/lib/cpp2/Thrift.h>
#include <thrift/lib/cpp2/protocol/Cpp2Ops.h>

namespace apache {
namespace thrift {
namespace detail {

template <typename Protocol, typename Args>
uint32_t deserializeRequestBody(Protocol* prot, Args* args) {
  auto xferStart = prot->getCursorPosition();
  ::apache::thrift::Cpp2Ops<Args>::read(prot, args);
  return folly::to_narrow(prot->getCursorPosition() - xferStart);
}

template <typename Protocol, typename Args>
uint32_t serializeResponseBody(Protocol* prot, const Args* args) {
  return ::apache::thrift::Cpp2Ops<Args>::write(prot, args);
}

template <typename Protocol, typename Args>
uint32_t serializedResponseBodySizeZC(Protocol* prot, const Args* args) {
  return ::apache::thrift::Cpp2Ops<Args>::serializedSizeZC(prot, args);
}

template <typename Protocol, typename Args>
uint32_t deserializeExceptionBody(Protocol* prot, Args* args) {
  return args->read(prot);
}

template <typename Protocol, typename Args>
uint32_t serializeExceptionBody(Protocol* prot, const Args* args) {
  return args->write(prot);
}

template <typename Protocol, typename Args>
uint32_t serializedExceptionBodySizeZC(Protocol* prot, const Args* args) {
  return args->serializedSizeZC(prot);
}
} // namespace detail
} // namespace thrift
} // namespace apache
