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

#include <thrift/lib/cpp2/transcode/TranscodeErrc.h>
#include <thrift/lib/cpp2/transcode/TranscodePlan.h>

#include <folly/Expected.h>
#include <folly/io/IOBuf.h>

#include <cstdint>
#include <memory>

namespace apache::thrift::transcode {

enum class Engine : uint8_t { Interpreter, Jit };

enum class UnsupportedPlanPolicy : uint8_t {
  Reject,
  AllowExperimentalProtocols,
};

struct TranscoderOptions {
  UnsupportedPlanPolicy unsupportedPlanPolicy = UnsupportedPlanPolicy::Reject;
};

// Immutable, reentrant transcoder for one fused TranscodePlan; safe to share
// across threads.
class ITranscoder {
 public:
  virtual ~ITranscoder() = default;
  virtual folly::Expected<std::unique_ptr<folly::IOBuf>, TranscodeError>
  transcode(const folly::IOBuf& input) const = 0;
  virtual folly::Expected<size_t, TranscodeError> transcodeInto(
      const folly::IOBuf& input, uint8_t* out, size_t cap) const = 0;
  virtual Engine engine() const = 0;
};

folly::Expected<std::unique_ptr<ITranscoder>, CompileError> makeTranscoder(
    TranscodePlan plan, Engine engine, TranscoderOptions options = {});

// JIT arm registration (populated by the :jit target later, via a link_whole
// static registrar). Declared here so :jit can register without depending on
// the interpreter.
using JitTranscoderFactory =
    folly::Expected<std::unique_ptr<ITranscoder>, CompileError> (*)(
        TranscodePlan);
void registerJitTranscoderFactory(JitTranscoderFactory factory);

} // namespace apache::thrift::transcode
