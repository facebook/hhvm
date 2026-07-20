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

#include <thrift/lib/cpp2/transcode/Codec.h>
#include <thrift/lib/cpp2/transcode/TranscodeErrc.h>

#include <folly/Expected.h>

#include <string>

namespace apache::thrift::transcode {

/**
 * A TranscodePlan is a Command tree describing how to transcode from one
 * wire format to another. The root can be any Command variant — a struct,
 * list, map, or even a bare scalar.
 *
 * Build a plan by constructing Commands directly, or use CodecFactory to
 * produce source/target codecs and fuse them with fuseCodecs().
 */
struct TranscodePlan {
  std::string name;
  Command root;
  WireProtocol sourceProtocol = WireProtocol::Unknown;
  WireProtocol targetProtocol = WireProtocol::Unknown;

  // If true, kernel writes to a struct pointer (3rd arg) instead of output
  // buffer. Kernel signature: int32_t(const uint8_t* in, size_t inLen, void*)
  bool structTarget = false;

  // If true, skip all ensure_write checks. Use only when the caller guarantees
  // the output buffer is large enough (e.g. pre-allocated benchmarks).
  // This eliminates all bounds checking overhead from the generated code.
  bool unsafeSkipEnsureWrite = false;

  // If true, emit read-side bounds checks (readPos vs readEnd) on the
  // generated kernel's inline fixed/length-prefixed reads. The check is
  // hoisted out of fixed-element container loops (one check per container
  // instead of one per element). Mirrors the bounds guarantees of fbthrift's
  // protocol readers. Default off preserves the original (unchecked) codegen.
  bool checkReadBounds = false;

  // Force-link the intrinsics' LLVM bitcode into the kernel module before
  // optimization so the JIT can INLINE the JSON/text primitives (otherwise
  // opaque extern-C calls). Normally this is auto-enabled when the plan uses
  // text/JSON ops; set this to force it on even otherwise (testing).
  bool inlineIntrinsicsBitcode = false;

  // Disable bitcode inlining even if the plan uses text/JSON ops. Escape hatch
  // for benchmarking the non-inlined baseline.
  bool disableBitcodeInlining = false;
};

folly::Expected<TranscodePlan, CompileError> fuseCodecs(
    const Codec& source, const Codec& target);

/**
 * Fuse a source StructOp (read-side) with a target StructOp (write-side)
 * into a single StructOp that reads from source and writes to target.
 *
 * Fields are matched by field ID. For each matched field:
 *   - readFn comes from the source
 *   - writeFn comes from the target
 *   - coercion is inferred from value kinds
 *
 * Fields only in source → skipped at runtime (via skipField intrinsic).
 * Fields only in target → left unset.
 */
folly::Expected<Command, CompileError> fuseStructOps(
    const StructOp& source, const StructOp& target);

/**
 * Fuse two scalar ops: take readFn from source, writeFn from target,
 * infer coercion from value kinds.
 */
folly::Expected<ScalarOp, CompileError> fuseScalarOps(
    const ScalarOp& source, const ScalarOp& target);

} // namespace apache::thrift::transcode
