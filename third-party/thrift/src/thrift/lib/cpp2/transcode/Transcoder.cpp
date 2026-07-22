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

bool hasJsonMapSourceAndTarget(const Command& cmd) {
  if (const auto* mp = std::get_if<MapOp>(&cmd)) {
    if (mp->readFraming == ContainerFraming::Json &&
        mp->writeFraming == ContainerFraming::Json) {
      return true;
    }
    return (mp->key != nullptr && hasJsonMapSourceAndTarget(*mp->key)) ||
        (mp->value != nullptr && hasJsonMapSourceAndTarget(*mp->value));
  }
  if (const auto* st = std::get_if<StructOp>(&cmd)) {
    for (const auto& field : st->fields) {
      if (field.command != nullptr &&
          hasJsonMapSourceAndTarget(*field.command)) {
        return true;
      }
    }
    return false;
  }
  if (const auto* sq = std::get_if<SeqOp>(&cmd)) {
    return sq->element != nullptr && hasJsonMapSourceAndTarget(*sq->element);
  }
  return false;
}

std::optional<std::string> missingProtocolReason(const TranscodePlan& plan) {
  if (plan.sourceProtocol == WireProtocol::Unknown ||
      plan.targetProtocol == WireProtocol::Unknown) {
    return "transcode plan protocol metadata is missing";
  }
  return std::nullopt;
}

std::optional<std::string> unsupportedProtocolReason(
    const TranscodePlan& plan) {
  const bool json = plan.sourceProtocol == WireProtocol::Json ||
      plan.targetProtocol == WireProtocol::Json;
  const bool protobuf = plan.sourceProtocol == WireProtocol::ProtobufBinary ||
      plan.targetProtocol == WireProtocol::ProtobufBinary;
  if (json && protobuf) {
    return "JSON/Protobuf protocol support is still in development; pass "
           "UnsupportedPlanPolicy::"
           "AllowExperimentalProtocols to opt in";
  }
  if (json) {
    return "JSON protocol support is still in development; pass "
           "UnsupportedPlanPolicy::AllowExperimentalProtocols to opt in";
  }
  if (protobuf) {
    return "Protobuf protocol support is still in development; "
           "pass UnsupportedPlanPolicy::AllowExperimentalProtocols to opt in";
  }
  return std::nullopt;
}

// Returns a reason string when the interpreter cannot run this plan, otherwise
// nullopt.
std::optional<std::string> interpreterSupports(const TranscodePlan& plan) {
  if (plan.structTarget) {
    return "interpreter does not support struct-memory targets; use Engine::Jit";
  }
  if (plan.sourceProtocol == WireProtocol::Json &&
      plan.targetProtocol == WireProtocol::Json) {
    return "interpreter does not yet support JSON-to-JSON plans; use Engine::Jit";
  }
  if (hasJsonMapSourceAndTarget(plan.root)) {
    return "interpreter does not support JSON-to-JSON map transcodes";
  }
  return std::nullopt;
}

} // namespace

folly::Expected<std::unique_ptr<ITranscoder>, CompileError> makeTranscoder(
    TranscodePlan plan, Engine engine, TranscoderOptions options) {
  if (engine == Engine::Jit) {
    return folly::makeUnexpected(CompileError{"JIT engine not linked"});
  }
  if (engine != Engine::Interpreter) {
    return folly::makeUnexpected(CompileError{"unknown engine"});
  }
  if (auto reason = missingProtocolReason(plan)) {
    return folly::makeUnexpected(CompileError{std::move(*reason)});
  }
  if (options.unsupportedPlanPolicy == UnsupportedPlanPolicy::Reject) {
    if (auto reason = unsupportedProtocolReason(plan)) {
      return folly::makeUnexpected(CompileError{std::move(*reason)});
    }
  }
  if (auto reason = interpreterSupports(plan)) {
    return folly::makeUnexpected(CompileError{std::move(*reason)});
  }
  return std::make_unique<InterpretedTranscoder>(std::move(plan));
}

void registerJitTranscoderFactory(JitTranscoderFactory factory) {
  jitFactory().store(factory, std::memory_order_release);
}

} // namespace apache::thrift::transcode
