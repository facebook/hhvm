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

#include <memory>
#include <stdexcept>
#include <utility>

#include <folly/io/IOBuf.h>
#include <thrift/lib/cpp2/transport/rocket/compression/CustomCompressor.h>
#include <thrift/lib/cpp2/transport/rocket/payload/CustomCompressionPayloadSerializerStrategy.h>
#include <thrift/lib/cpp2/transport/rocket/payload/DefaultPayloadSerializerStrategy.h>
#include <thrift/lib/cpp2/transport/rocket/payload/PayloadSerializer.h>

namespace apache::thrift::rocket::test {

/**
 * Stands in for a codec that is unusable at runtime, e.g.
 * folly::compression::getCodec() throwing std::invalid_argument because the
 * binary was built without that codec.
 */
class ThrowingCompressor : public CustomCompressor {
 public:
  std::unique_ptr<folly::IOBuf> compressBuffer(
      std::unique_ptr<folly::IOBuf>&&) override {
    throw std::runtime_error("error during compress");
  }

  std::unique_ptr<folly::IOBuf> uncompressBuffer(
      std::unique_ptr<folly::IOBuf>&& buffer) override {
    return std::move(buffer);
  }
};

/**
 * Owns a PayloadSerializer whose pack() throws for payloads whose metadata sets
 * compression() to CompressionAlgorithm::CUSTOM, and serializes normally
 * otherwise. This makes failure injection precise: no toggling is needed, and
 * packCompact() (used to serialize the StreamRpcError sent in response to the
 * failure) never compresses, so it cannot re-enter the throwing compressor.
 */
class ThrowingPayloadSerializer {
 public:
  ThrowingPayloadSerializer() : serializer_(make()) {}

  PayloadSerializer::Ptr getNonOwningPtr() {
    return serializer_.getNonOwningPtr();
  }

 private:
  static PayloadSerializer make() {
    CustomCompressionPayloadSerializerStrategyOptions options;
    options.compressor = std::make_shared<ThrowingCompressor>();
    return PayloadSerializer::make<CustomCompressionPayloadSerializerStrategy<
        DefaultPayloadSerializerStrategy>>(options);
  }

  PayloadSerializer serializer_;
};

} // namespace apache::thrift::rocket::test
