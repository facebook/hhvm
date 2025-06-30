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

#include <thrift/lib/cpp2/security/extensions/Types.h>

namespace apache::thrift {

class ThriftParametersContext {
 public:
  folly::Range<const CompressionAlgorithm*> getSupportedCompressionAlgorithms()
      const {
    return supportedCompressionAlgos_;
  }

  void setUseStopTLS(bool useStopTLS) { useStopTLS_ = useStopTLS; }

  bool getUseStopTLS() const { return useStopTLS_; }

  void setUseStopTLSV2(bool useStopTLSV2) { useStopTLSV2_ = useStopTLSV2; }

  bool getUseStopTLSV2() const { return useStopTLSV2_; }

 private:
  static constexpr std::array<CompressionAlgorithm, 2>
      supportedCompressionAlgos_{{
          CompressionAlgorithm::ZSTD,
          CompressionAlgorithm::ZLIB,
      }};
  bool useStopTLS_{false};
  bool useStopTLSV2_{false};
};
} // namespace apache::thrift
