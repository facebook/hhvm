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

#include <thrift/lib/cpp/transport/THeader.h>
#include <thrift/lib/cpp2/async/RequestChannel.h>
#include <thrift/lib/thrift/gen-cpp2/RpcMetadata_types.h>

namespace apache::thrift {

/**
 * HeaderChannel manages persistent headers.
 */
class HeaderChannel {
 public:
  void addRpcOptionHeaders(
      apache::thrift::transport::THeader* header, const RpcOptions& rpcOptions);

  void setPersistentHeader(const std::string& key, const std::string& value) {
    persistentWriteHeaders_[key] = value;
  }

  void setDesiredCompressionConfig(CompressionConfig compressionConfig) {
    compressionConfig_ = compressionConfig;
  }

  void setLoggingContext(LoggingContext loggingContext) {
    loggingContext_ = std::move(loggingContext);
  }

  const transport::THeader::StringToStringMap& getPersistentWriteHeaders()
      const {
    return persistentWriteHeaders_;
  }

  void preprocessHeader(apache::thrift::transport::THeader* header) const;

 protected:
  virtual ~HeaderChannel() = default;

  virtual bool clientSupportHeader() { return true; }

 private:
  // Map to use for persistent headers
  transport::THeader::StringToStringMap persistentWriteHeaders_;
  folly::Optional<CompressionConfig> compressionConfig_;
  std::optional<LoggingContext> loggingContext_;
};
} // namespace apache::thrift
