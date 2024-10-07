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

#include <thrift/lib/cpp2/transport/core/ThriftRequest.h>

namespace apache::thrift {

class Cpp2ConnContext;

namespace server {

// TODO(sazonovk): Should we move it to apache::thrift namespace?
struct PreprocessParams {
  PreprocessParams(
      const transport::THeader::StringToStringMap& headersIn,
      const std::string& methodIn,
      const Cpp2ConnContext& connContextIn,
      const ThriftRequestCore* request = nullptr)
      : headers(headersIn),
        method(methodIn),
        connContext(connContextIn),
        request_(request) {}
  const transport::THeader::StringToStringMap& headers;
  const std::string& method;
  const Cpp2ConnContext& connContext;

  const std::string* clientId() const {
    if (request_ && request_->getTHeader().clientId()) {
      return &*request_->getTHeader().clientId();
    }
    return folly::get_ptr(headers, transport::THeader::kClientId);
  }

  const std::string* tenantId() const {
    if (request_ && request_->getTHeader().tenantId()) {
      return &*request_->getTHeader().tenantId();
    }
    return folly::get_ptr(headers, transport::THeader::kTenantId);
  }

  const std::string* getServiceTraceMeta() const {
    if (request_ && request_->getTHeader().serviceTraceMeta()) {
      return &*request_->getTHeader().serviceTraceMeta();
    }
    return folly::get_ptr(headers, transport::THeader::kServiceTraceMeta);
  }

 private:
  const ThriftRequestCore* request_;
};

} // namespace server
} // namespace apache::thrift
