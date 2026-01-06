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

#include <vector>

#include <folly/Indestructible.h>
#include <thrift/lib/cpp2/util/SchemaToMetadata.h>
#include <thrift/lib/thrift/gen-cpp2/metadata_types.h>

namespace apache::thrift {

template <class>
class ServiceHandler;

namespace detail::md {

using ThriftMetadata = ::apache::thrift::metadata::ThriftMetadata;
using ThriftServiceContext = ::apache::thrift::metadata::ThriftServiceContext;
using ThriftServiceContextRef =
    ::apache::thrift::metadata::ThriftServiceContextRef;
using ThriftService = ::apache::thrift::metadata::ThriftService;
using ThriftServiceMetadataResponse =
    ::apache::thrift::metadata::ThriftServiceMetadataResponse;

template <typename T>
class EnumMetadata {
 public:
  static void gen(ThriftMetadata& metadata) { genEnumMetadata<T>(metadata); }
};

template <typename T>
class StructMetadata {
 public:
  static const metadata::ThriftStruct& gen(ThriftMetadata& metadata) {
    return genStructMetadata<T>(metadata).metadata;
  }
};

template <typename T>
class ExceptionMetadata {
 public:
  static void gen(ThriftMetadata& metadata) {
    genExceptionMetadata<T>(metadata);
  }
};

template <typename T>
class ServiceMetadata {
  static_assert(folly::always_false<T>);
};

template <typename Tag>
class ServiceMetadata<::apache::thrift::ServiceHandler<Tag>> {
 public:
  static void gen(ThriftServiceMetadataResponse& response) {
    genServiceMetadataResponse<Tag>(response);
  }
};

} // namespace detail::md

/**
 * Get ThriftMetadata of given thrift structure. If no_metadata option is
 * enabled, return empty data.
 *
 * @tparam T thrift structure
 *
 * @return ThriftStruct (https://git.io/JJQpW)
 */
template <class T>
[[deprecated("Use `SchemaRegistry::get().getNode<T>()` instead")]]
const metadata::ThriftStruct& get_struct_metadata() {
  static const folly::Indestructible<metadata::ThriftStruct> data = [] {
    detail::md::ThriftMetadata meta;
    return detail::md::StructMetadata<T>::gen(meta);
  }();
  return *data;
}

} // namespace apache::thrift
