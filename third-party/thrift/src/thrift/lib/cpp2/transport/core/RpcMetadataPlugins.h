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

#include <folly/dynamic.h>
#include <folly/io/IOBuf.h>
#include <thrift/lib/cpp2/PluggableFunction.h>

namespace apache {
namespace thrift {

class RpcOptions;

namespace detail {

THRIFT_PLUGGABLE_FUNC_DECLARE(
    std::unique_ptr<folly::IOBuf>,
    makeFrameworkMetadata,
    const RpcOptions&,
    folly::dynamic& logMessages);

THRIFT_PLUGGABLE_FUNC_DECLARE(const std::string&, getFrameworkMetadataHttpKey);

THRIFT_PLUGGABLE_FUNC_DECLARE(
    void, ingestFrameworkMetadataFromResponse, std::unique_ptr<folly::IOBuf>&&);

} // namespace detail
} // namespace thrift
} // namespace apache
