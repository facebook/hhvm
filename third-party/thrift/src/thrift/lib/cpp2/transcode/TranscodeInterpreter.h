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

#include <thrift/lib/cpp2/transcode/Intrinsics.h>
#include <thrift/lib/cpp2/transcode/TranscodePlan.h>

#include <folly/Expected.h>
#include <folly/io/IOBuf.h>

#include <memory>

namespace apache::thrift::transcode {

/**
 * A non-JIT, tree-walking interpreter over the same Command tree that
 * TranscodeKernel compiles. It is a complete, standalone execution engine for
 * transcode plans, usable wherever the LLVM dependency isn't wanted. It also
 * serves as the performance baseline that isolates what the LLVM JIT actually
 * buys us by holding everything else equal.
 *
 * Crucially, the interpreter executes the *same* TranscodePlan and calls the
 * *same* extern "C" intrinsics (Intrinsics.cpp) that the JIT kernel calls. The
 * only differences between this and a compiled kernel are therefore:
 *
 *   1. Per-node dispatch: this walks the Command variant at runtime
 *      (std::visit + pointer-chasing) instead of straight-line specialized IR.
 *   2. Primitive inlining: the JIT inlines the hot binary primitives (varint,
 *      fixed-width, field headers) as IR, whereas the interpreter calls them as
 *      out-of-line extern "C" functions.
 *
 * So `JIT_time / interpreter_time` measures the *combined* benefit of
 * specialization + inlining over a reasonable dynamic interpreter — which is
 * the number that actually justifies (or doesn't) the LLVM dependency. It does
 * NOT measure "no DOM allocation"; both the interpreter and the JIT are
 * single-pass and tree-free, so that win is shared and is NOT what this
 * baseline is for. (The protocol::Object / DynamicValue benches measure that.)
 *
 * Scope: this port currently supports wire↔wire transcoding between binary
 * protocols (Thrift Compact, Thrift Binary, Protobuf Binary) for scalars,
 * lists, sets, maps, and nested structs. The folly::dynamic endpoints
 * (transcodeToDynamic / transcodeFromDynamic) are deferred.
 */
class TranscodeInterpreter {
 public:
  explicit TranscodeInterpreter(TranscodePlan plan);

  /**
   * Transcode input → output, allocating the output buffer via malloc/realloc.
   * Mirrors TranscodeKernel::transcode so benchmarks compare like-for-like.
   */
  folly::Expected<std::unique_ptr<folly::IOBuf>, TranscodeError> transcode(
      const folly::IOBuf& input) const;

  /**
   * Transcode into a caller-provided buffer. No allocation; fails if too small.
   * Returns the number of bytes written. Mirrors
   * TranscodeKernel::transcodeInto.
   */
  folly::Expected<size_t, TranscodeError> transcodeInto(
      const folly::IOBuf& input, uint8_t* output, size_t outputCapacity) const;

  const TranscodePlan& plan() const { return plan_; }

 private:
  TranscodePlan plan_;
};

} // namespace apache::thrift::transcode
