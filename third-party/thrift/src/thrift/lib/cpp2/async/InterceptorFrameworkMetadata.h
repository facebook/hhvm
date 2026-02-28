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
#include <thrift/lib/cpp2/PluggableFunction.h>
#include <thrift/lib/cpp2/async/RpcOptions.h>
#include <thrift/lib/cpp2/util/TypeErasedValue.h>

namespace apache::thrift {

/**
 * This corresponds to closed-source data in RpcMetadata.thrift's
 * frameworkMetadata fields. The actual type store in this buffer can be
 * customized by providing an implementation of
 * initializeInterceptorFrameworkMetadataStorage() and
 * serializeFrameworkMetadata().
 */
using InterceptorFrameworkMetadataStorage =
    util::TypeErasedValue<704, alignof(std::max_align_t)>;

namespace detail {

THRIFT_PLUGGABLE_FUNC_DECLARE(
    InterceptorFrameworkMetadataStorage,
    initializeInterceptorFrameworkMetadataStorage);

THRIFT_PLUGGABLE_FUNC_DECLARE(
    void,
    postProcessFrameworkMetadata,
    InterceptorFrameworkMetadataStorage& storage,
    const RpcOptions& rpcOptions);

THRIFT_PLUGGABLE_FUNC_DECLARE(
    std::unique_ptr<folly::IOBuf>,
    serializeFrameworkMetadata,
    InterceptorFrameworkMetadataStorage&& storage);

THRIFT_PLUGGABLE_FUNC_DECLARE(
    InterceptorFrameworkMetadataStorage,
    deserializeFrameworkMetadata,
    const folly::IOBuf& buf);

} // namespace detail

} // namespace apache::thrift
