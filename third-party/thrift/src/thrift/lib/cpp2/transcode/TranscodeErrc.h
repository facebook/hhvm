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

#include <cstddef>
#include <cstdint>
#include <string>
#include <string_view>

namespace apache::thrift::transcode {

// Error taxonomy shared by both engines.
//
// DEVNOTE: The numeric values are latched into the transcode cursor by the
// interpreter's intrinsics and by the JIT kernel, so they are a stable ABI:
// keep them in sync with the intrinsics (and the bitcode twin) whenever they
// change.
enum class TranscodeErrc : int32_t {
  Ok = 0,
  Truncated = 1, // input ended mid-value
  Malformed = 2, // wire or JSON bytes violate the grammar
  Overflow = 3, // value out of range for a narrow target type
  LimitExceeded = 4, // a resource limit (depth/size/count) was hit
  Unsupported = 5, // the plan or engine cannot express this conversion
  SchemaMismatch = 6, // a wire type byte disagrees with the schema
  DuplicateArgument = 7, // a request argument arrived from two sources
  MissingArgument = 8, // a required request argument was not provided
  Oom = 9, // output allocation failed
  Internal = 10, // invariant violation (a bug), not attacker-triggerable
};

// Stable, human-readable name for a code, for logs, metrics, and diagnostics.
std::string_view toString(TranscodeErrc code) noexcept;

// A runtime transcoding failure. Returned via folly::Expected on the fallible
// paths; input-derived failures are never thrown.
struct TranscodeError {
  TranscodeErrc code = TranscodeErrc::Ok;
  std::string message;
  size_t inputOffset = 0; // best-effort byte offset into the input
};

// A plan or kernel compilation failure (a schema or codegen problem, distinct
// from a runtime input error).
struct CompileError {
  std::string message;
};

} // namespace apache::thrift::transcode
