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

#include <thrift/lib/cpp2/transcode/IntrinsicsCommon.h>

#include <folly/Benchmark.h>
#include <folly/Portability.h>
#include <folly/init/Init.h>

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <type_traits>
#include <utility>

namespace apache::thrift::transcode {
namespace {

constexpr size_t kBufferSize = 4096;
constexpr size_t kRawBytesSize = 16;

using CallbackReturn = decltype(std::declval<TranscodeExtendFn>()(
    std::declval<const TranscodeExtendRequest*>(),
    std::declval<TranscodeExtendResult*>(),
    std::declval<void*>()));

template <class T = CallbackReturn>
T callbackOk() {
  if constexpr (std::is_same_v<T, bool>) {
    return true;
  } else {
    return T::Ok;
  }
}

template <class F>
void runWithTailroom(size_t iters, size_t maxBytesPerIter, F&& f) {
  std::array<uint8_t, kBufferSize> buffer{};
  TranscodeCursor cursor{};
  thrift_transcode_cursor_init(
      &cursor,
      {nullptr, nullptr},
      {buffer.data(), buffer.data() + buffer.size()},
      nullptr,
      nullptr,
      nullptr);

  const size_t maxItersPerBatch = buffer.size() / maxBytesPerIter;
  while (iters > 0) {
    const size_t batch = std::min(iters, maxItersPerBatch);
    cursor.writePos = cursor.writeSegmentBegin;
    for (size_t i = 0; i < batch; ++i) {
      f(cursor);
    }
    iters -= batch;
  }

  folly::doNotOptimizeAway(cursor.writePos);
  folly::doNotOptimizeAway(buffer.data());
}

struct RotatingOutput {
  std::array<std::array<uint8_t, 1>, 8> segments{};
  size_t next = 1;
};

CallbackReturn rotateSegment(
    const TranscodeExtendRequest* /*request*/,
    TranscodeExtendResult* result,
    void* userData) {
  auto& output = *static_cast<RotatingOutput*>(userData);
  auto& segment = output.segments[output.next++ % output.segments.size()];
  result->kind = TranscodeExtendKind::NewSegment;
  result->segment = {segment.data(), segment.data() + segment.size()};
  return callbackOk();
}

CallbackReturn reuseTailroomOnFlush(
    const TranscodeFlushRequest* request,
    TranscodeExtendResult* result,
    void* /*userData*/) {
  result->kind = TranscodeExtendKind::NewSegment;
  result->segment = {request->flushPoint, request->segment.end};
  return callbackOk();
}

FOLLY_NOINLINE void benchmarkEnsureWriteOne(TranscodeCursor* cursor) {
  thrift_transcode_cursor_ensure_write(cursor, 1);
  *cursor->writePos++ = 0x5a;
}

FOLLY_NOINLINE void benchmarkWriteByte(TranscodeCursor* cursor) {
  thrift_transcode_write_byte_checked(cursor, 0x5a);
}

FOLLY_NOINLINE void benchmarkWriteFixed32BE(TranscodeCursor* cursor) {
  thrift_transcode_write_fixed32_be_checked(cursor, 0x12345678);
}

FOLLY_NOINLINE void benchmarkWriteVarint(TranscodeCursor* cursor) {
  thrift_transcode_write_unsigned_varint(cursor, 0x1234);
}

FOLLY_NOINLINE void benchmarkWriteRawBytes(
    TranscodeCursor* cursor, const uint8_t* data, size_t len) {
  thrift_transcode_write_raw_bytes_checked(cursor, data, len);
}

FOLLY_NOINLINE void benchmarkFlush(TranscodeCursor* cursor) {
  *cursor->writePos++ = 0x5a;
  thrift_transcode_cursor_flush(cursor, 1);
}

BENCHMARK(CursorEnsureWriteHasTailroom, iters) {
  runWithTailroom(iters, 1, [](TranscodeCursor& cursor) {
    benchmarkEnsureWriteOne(&cursor);
  });
}

BENCHMARK(CursorWriteByteIntrinsicHasTailroom, iters) {
  runWithTailroom(
      iters, 1, [](TranscodeCursor& cursor) { benchmarkWriteByte(&cursor); });
}

BENCHMARK(CursorWriteFixed32BEIntrinsicHasTailroom, iters) {
  runWithTailroom(iters, 4, [](TranscodeCursor& cursor) {
    benchmarkWriteFixed32BE(&cursor);
  });
}

BENCHMARK(CursorWriteVarintIntrinsicHasTailroom, iters) {
  runWithTailroom(iters, 10, [](TranscodeCursor& cursor) {
    benchmarkWriteVarint(&cursor);
  });
}

BENCHMARK(CursorWriteRawBytesIntrinsicHasTailroom, iters) {
  const std::array<uint8_t, kRawBytesSize> data{
      0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15};
  runWithTailroom(iters, kRawBytesSize, [&](TranscodeCursor& cursor) {
    benchmarkWriteRawBytes(&cursor, data.data(), data.size());
  });
}

BENCHMARK(CursorWriteByteRotatingSegments, iters) {
  RotatingOutput output;
  TranscodeCursor cursor{};
  auto& first = output.segments[0];
  thrift_transcode_cursor_init(
      &cursor,
      {nullptr, nullptr},
      {first.data(), first.data() + first.size()},
      rotateSegment,
      nullptr,
      &output);

  for (size_t i = 0; i < iters; ++i) {
    benchmarkWriteByte(&cursor);
  }

  folly::doNotOptimizeAway(cursor.writePos);
  folly::doNotOptimizeAway(output.segments.data());
}

BENCHMARK(CursorFlushNoCallback, iters) {
  std::array<uint8_t, kBufferSize> buffer{};
  while (iters > 0) {
    TranscodeCursor cursor{};
    thrift_transcode_cursor_init(
        &cursor,
        {nullptr, nullptr},
        {buffer.data(), buffer.data() + buffer.size()},
        nullptr,
        nullptr,
        nullptr);
    const size_t batch = std::min<size_t>(iters, buffer.size() - 1);
    for (size_t i = 0; i < batch; ++i) {
      benchmarkFlush(&cursor);
    }
    iters -= batch;
  }
  folly::doNotOptimizeAway(buffer.data());
}

BENCHMARK(CursorFlushReuseTailroom, iters) {
  std::array<uint8_t, kBufferSize> buffer{};
  while (iters > 0) {
    TranscodeCursor cursor{};
    thrift_transcode_cursor_init(
        &cursor,
        {nullptr, nullptr},
        {buffer.data(), buffer.data() + buffer.size()},
        nullptr,
        reuseTailroomOnFlush,
        nullptr);
    const size_t batch = std::min<size_t>(iters, buffer.size() - 1);
    for (size_t i = 0; i < batch; ++i) {
      benchmarkFlush(&cursor);
    }
    iters -= batch;
  }
  folly::doNotOptimizeAway(buffer.data());
}

} // namespace
} // namespace apache::thrift::transcode

int main(int argc, char** argv) {
  folly::Init init(&argc, &argv);
  folly::runBenchmarks();
  return 0;
}
