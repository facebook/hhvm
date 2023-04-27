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

#include <fizz/record/Extensions.h>
#include <thrift/lib/thrift/gen-cpp2/RpcMetadata_types.h>

namespace apache {
namespace thrift {

struct ThriftParametersExt {
  NegotiationParameters params;
};

fizz::Extension encodeThriftExtension(const ThriftParametersExt& params);

folly::Optional<ThriftParametersExt> getThriftExtension(
    const std::vector<fizz::Extension>& extensions);

} // namespace thrift
} // namespace apache
