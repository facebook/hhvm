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

#include <thrift/lib/cpp2/transport/core/RpcMetadataPlugins.h>

namespace apache::thrift::detail {

THRIFT_PLUGGABLE_FUNC_REGISTER(
    std::unique_ptr<folly::IOBuf>,
    makeFrameworkMetadata,
    const RpcOptions&,
    folly::dynamic&) {
  return nullptr;
}

THRIFT_PLUGGABLE_FUNC_REGISTER(
    const std::string&, getFrameworkMetadataHttpKey) {
  static const std::string ret("thrift_fmhk");
  return ret;
}

THRIFT_PLUGGABLE_FUNC_REGISTER(
    void,
    ingestFrameworkMetadataFromResponse,
    std::unique_ptr<folly::IOBuf>&&) {}
} // namespace apache::thrift::detail
