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

#include <folly/Indestructible.h>
#include <thrift/conformance/cpp2/AnyRegistry.h>
#include <thrift/conformance/cpp2/ThriftTypeInfo.h>
#include <thrift/lib/thrift/gen-cpp2/protocol_detail_types.h>

namespace apache::thrift::protocol::detail {

template <class Base>
const char* ObjectWrapper<Base>::__fbthrift_thrift_uri() {
  static const folly::Indestructible<std::string> ret = uri<Base>();
  return ret->c_str();
}

template <class Base>
const char* ValueWrapper<Base>::__fbthrift_thrift_uri() {
  static const folly::Indestructible<std::string> ret = uri<Base>();
  return ret->c_str();
}

template const char* ObjectWrapper<detail::Object>::__fbthrift_thrift_uri();
template const char* ValueWrapper<detail::Value>::__fbthrift_thrift_uri();
} // namespace apache::thrift::protocol::detail
