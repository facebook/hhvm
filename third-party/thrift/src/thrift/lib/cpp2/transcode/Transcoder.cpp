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

#include <thrift/lib/cpp2/transcode/Transcoder.h>

#include <thrift/lib/cpp2/transcode/Codec.h>
#include <thrift/lib/cpp2/transcode/TranscodeInterpreter.h>

#include <atomic>
#include <optional>
#include <string>
#include <utility>
#include <variant>

namespace apache::thrift::transcode {

namespace {

class InterpretedTranscoder final : public ITranscoder {
 public:
  explicit InterpretedTranscoder(TranscodePlan plan)
      : interp_(std::move(plan)) {}

  folly::Expected<std::unique_ptr<folly::IOBuf>, TranscodeError> transcode(
      const folly::IOBuf& input) const override {
    return interp_.transcode(input);
  }

  folly::Expected<size_t, TranscodeError> transcodeInto(
      const folly::IOBuf& input, uint8_t* out, size_t cap) const override {
    return interp_.transcodeInto(input, out, cap);
  }

  Engine engine() const override { return Engine::Interpreter; }

 private:
  TranscodeInterpreter interp_;
};

std::atomic<JitTranscoderFactory>& jitFactory() {
  static std::atomic<JitTranscoderFactory> factory{nullptr};
  return factory;
}

// TODO(D3): replace this string/enum sniffing with the WireProtocol enum once
// it lands.
bool structReadsJson(const StructOp& op) {
  return op.fieldIdent == FieldIdent::ByName;
}

// Returns a reason string when the interpreter cannot run this plan, otherwise
// nullopt.
std::optional<std::string> interpreterSupports(const TranscodePlan& plan) {
  if (plan.structTarget) {
    return "interpreter does not support struct-memory targets; use Engine::Jit";
  }

  bool jsonSource = false;
  if (const auto* st = std::get_if<StructOp>(&plan.root)) {
    jsonSource = structReadsJson(*st);
  } else if (const auto* sq = std::get_if<SeqOp>(&plan.root)) {
    jsonSource = sq->readFraming == ContainerFraming::Json;
  } else if (const auto* mp = std::get_if<MapOp>(&plan.root)) {
    jsonSource = mp->readFraming == ContainerFraming::Json;
  }
  if (jsonSource) {
    return "interpreter does not support a JSON source; use Engine::Jit";
  }
  return std::nullopt;
}

} // namespace

folly::Expected<std::unique_ptr<ITranscoder>, CompileError> makeTranscoder(
    TranscodePlan plan, Engine engine) {
  switch (engine) {
    case Engine::Interpreter:
      if (auto reason = interpreterSupports(plan)) {
        return folly::makeUnexpected(CompileError{std::move(*reason)});
      }
      return std::make_unique<InterpretedTranscoder>(std::move(plan));
    case Engine::Jit:
      if (auto factory = jitFactory().load(std::memory_order_acquire)) {
        return factory(std::move(plan));
      }
      return folly::makeUnexpected(CompileError{"JIT engine not linked"});
  }
  return folly::makeUnexpected(CompileError{"unknown engine"});
}

void registerJitTranscoderFactory(JitTranscoderFactory factory) {
  jitFactory().store(factory, std::memory_order_release);
}

} // namespace apache::thrift::transcode
