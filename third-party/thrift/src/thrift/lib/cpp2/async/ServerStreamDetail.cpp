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

#include <thrift/lib/cpp2/async/ServerStreamDetail.h>

#include <folly/ExceptionString.h>
#include <folly/compression/Compression.h>
#include <folly/logging/xlog.h>
#include <thrift/lib/cpp2/transport/rocket/compression/CompressionAlgorithmSelector.h>
#include <thrift/lib/cpp2/transport/rocket/compression/CompressionManager.h>

namespace apache::thrift::detail {

std::optional<StreamCompressionContext> makeCompressionContext(
    const CompressionConfig& config) {
  const auto& codecConfig = config.codecConfig();
  if (!codecConfig) {
    return std::nullopt;
  }
  auto algorithm = rocket::CompressionManager().fromCodecConfig(*codecConfig);
  if (algorithm == CompressionAlgorithm::NONE ||
      algorithm == CompressionAlgorithm::CUSTOM) {
    return std::nullopt;
  }
  try {
    auto [codecType, level] =
        rocket::CompressionAlgorithmSelector::toCodecTypeAndLevel(algorithm);
    auto codec = folly::compression::getCodec(codecType, level);
    auto sizeLimit =
        static_cast<size_t>(config.compressionSizeLimit().value_or(0));
    return StreamCompressionContext{algorithm, std::move(codec), sizeLimit};
  } catch (...) {
    XLOG(ERR) << "Failed to create compression codec: "
              << folly::exceptionStr(folly::current_exception());
    return std::nullopt;
  }
}

void compressStreamItem(
    StreamPayload& sp,
    const StreamCompressionContext& ctx,
    size_t payloadSize) {
  if (!sp.payload || sp.payload->empty()) {
    return;
  }
  if (payloadSize <= ctx.sizeLimit) {
    return;
  }
  try {
    auto compressed = ctx.codec->compress(sp.payload.get());
    sp.payload = std::move(compressed);
    sp.metadata.compression() = ctx.algorithm;
  } catch (...) {
    XLOG(ERR) << "CPU-thread stream item compression failed: "
              << folly::exceptionStr(folly::current_exception());
  }
}

} // namespace apache::thrift::detail
