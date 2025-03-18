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

#include <thrift/lib/cpp2/protocol/JSONProtocolCommon.h>

#include <cstddef>
#include <string>

#include <folly/Benchmark.h>
#include <folly/Range.h>
#include <folly/Traits.h>
#include <folly/functional/traits.h>
#include <folly/init/Init.h>
#include <folly/lang/Keep.h>

using namespace std;

using namespace folly;
using namespace folly::io;
using namespace apache::thrift;

extern "C" FOLLY_KEEP size_t
check_thrift_json_read_whitespace(JSONProtocolReaderCommon& reader) {
  struct Reader : JSONProtocolReaderCommon {
    using JSONProtocolReaderCommon::readWhitespace;
  };
  return static_cast<Reader&>(reader).readWhitespace();
}

namespace {

template <typename F>
FOLLY_NOINLINE void reading(size_t iters, Cursor& cur, F f) {
  using member_sig = decltype(&F::operator());
  using sig = typename folly::member_pointer_traits<member_sig>::member_type;
  using traits = folly::function_traits<sig>;
  using arg_type = typename traits::template arguments<folly::tag_t>;
  while (iters--) {
    folly::remove_cvref_t<folly::type_list_element_t<0, arg_type>> reader;
    reader.setInput(cur);
    compiler_must_not_elide(reader); // for baseline
    folly::remove_cvref_t<folly::type_list_element_t<1, arg_type>> out;
    f(reader, out);
  }
}

void baseline(size_t iters) {
  struct Reader : JSONProtocolReaderCommon {
    using JSONProtocolReaderCommon::readWhitespace;
  };
  BenchmarkSuspender bs;
  IOBuf buf;
  Cursor cur(&buf);
  (void)cur.peekBytes();
  bs.dismissing([&] { //
    reading(iters, cur, [&](Reader&, int&) {});
  });
}

void read_whitespace(size_t iters, size_t spaces, bool more, bool endl) {
  struct Reader : JSONProtocolReaderCommon {
    using JSONProtocolReaderCommon::readWhitespace;
  };
  BenchmarkSuspender bs;
  auto str = endl ? std::string("\n") : std::string();
  str += std::string(spaces, ' ');
  str += more ? std::string(16, 'x') : std::string();
  IOBuf buf(IOBuf::WRAP_BUFFER, ByteRange(StringPiece(str)));
  Cursor cur(&buf);
  (void)cur.peekBytes();
  bs.dismissing([&] {
    reading(iters, cur, [&](Reader& r, int&) { r.readWhitespace(); });
  });
}

} // namespace

BENCHMARK_NAMED_PARAM(baseline, baseline)

BENCHMARK_DRAW_LINE();

BENCHMARK_NAMED_PARAM(read_whitespace, ws_edge_00, 0x00, false, false)
BENCHMARK_NAMED_PARAM(read_whitespace, ws_edge_01, 0x01, false, false)
BENCHMARK_NAMED_PARAM(read_whitespace, ws_edge_02, 0x02, false, false)
BENCHMARK_NAMED_PARAM(read_whitespace, ws_edge_03, 0x03, false, false)
BENCHMARK_NAMED_PARAM(read_whitespace, ws_edge_04, 0x04, false, false)
BENCHMARK_NAMED_PARAM(read_whitespace, ws_edge_05, 0x05, false, false)
BENCHMARK_NAMED_PARAM(read_whitespace, ws_edge_06, 0x06, false, false)
BENCHMARK_NAMED_PARAM(read_whitespace, ws_edge_07, 0x07, false, false)
BENCHMARK_NAMED_PARAM(read_whitespace, ws_edge_08, 0x08, false, false)
BENCHMARK_NAMED_PARAM(read_whitespace, ws_edge_09, 0x09, false, false)
BENCHMARK_NAMED_PARAM(read_whitespace, ws_edge_0a, 0x0a, false, false)
BENCHMARK_NAMED_PARAM(read_whitespace, ws_edge_0b, 0x0b, false, false)
BENCHMARK_NAMED_PARAM(read_whitespace, ws_edge_0c, 0x0c, false, false)
BENCHMARK_NAMED_PARAM(read_whitespace, ws_edge_0d, 0x0d, false, false)
BENCHMARK_NAMED_PARAM(read_whitespace, ws_edge_0e, 0x0e, false, false)
BENCHMARK_NAMED_PARAM(read_whitespace, ws_edge_0f, 0x0f, false, false)
BENCHMARK_NAMED_PARAM(read_whitespace, ws_edge_10, 0x10, false, false)

BENCHMARK_DRAW_LINE();

BENCHMARK_NAMED_PARAM(read_whitespace, ws_long_00, 0x00, true, false)
BENCHMARK_NAMED_PARAM(read_whitespace, ws_long_01, 0x01, true, false)
BENCHMARK_NAMED_PARAM(read_whitespace, ws_long_02, 0x02, true, false)
BENCHMARK_NAMED_PARAM(read_whitespace, ws_long_03, 0x03, true, false)
BENCHMARK_NAMED_PARAM(read_whitespace, ws_long_04, 0x04, true, false)
BENCHMARK_NAMED_PARAM(read_whitespace, ws_long_05, 0x05, true, false)
BENCHMARK_NAMED_PARAM(read_whitespace, ws_long_06, 0x06, true, false)
BENCHMARK_NAMED_PARAM(read_whitespace, ws_long_07, 0x07, true, false)
BENCHMARK_NAMED_PARAM(read_whitespace, ws_long_08, 0x08, true, false)
BENCHMARK_NAMED_PARAM(read_whitespace, ws_long_09, 0x09, true, false)
BENCHMARK_NAMED_PARAM(read_whitespace, ws_long_0a, 0x0a, true, false)
BENCHMARK_NAMED_PARAM(read_whitespace, ws_long_0b, 0x0b, true, false)
BENCHMARK_NAMED_PARAM(read_whitespace, ws_long_0c, 0x0c, true, false)
BENCHMARK_NAMED_PARAM(read_whitespace, ws_long_0d, 0x0d, true, false)
BENCHMARK_NAMED_PARAM(read_whitespace, ws_long_0e, 0x0e, true, false)
BENCHMARK_NAMED_PARAM(read_whitespace, ws_long_0f, 0x0f, true, false)
BENCHMARK_NAMED_PARAM(read_whitespace, ws_long_10, 0x10, true, false)

BENCHMARK_DRAW_LINE();

BENCHMARK_NAMED_PARAM(read_whitespace, ws_endl_00, 0x00, true, true)
BENCHMARK_NAMED_PARAM(read_whitespace, ws_endl_01, 0x01, true, true)
BENCHMARK_NAMED_PARAM(read_whitespace, ws_endl_02, 0x02, true, true)
BENCHMARK_NAMED_PARAM(read_whitespace, ws_endl_03, 0x03, true, true)
BENCHMARK_NAMED_PARAM(read_whitespace, ws_endl_04, 0x04, true, true)
BENCHMARK_NAMED_PARAM(read_whitespace, ws_endl_05, 0x05, true, true)
BENCHMARK_NAMED_PARAM(read_whitespace, ws_endl_06, 0x06, true, true)
BENCHMARK_NAMED_PARAM(read_whitespace, ws_endl_07, 0x07, true, true)
BENCHMARK_NAMED_PARAM(read_whitespace, ws_endl_08, 0x08, true, true)
BENCHMARK_NAMED_PARAM(read_whitespace, ws_endl_09, 0x09, true, true)
BENCHMARK_NAMED_PARAM(read_whitespace, ws_endl_0a, 0x0a, true, true)
BENCHMARK_NAMED_PARAM(read_whitespace, ws_endl_0b, 0x0b, true, true)
BENCHMARK_NAMED_PARAM(read_whitespace, ws_endl_0c, 0x0c, true, true)
BENCHMARK_NAMED_PARAM(read_whitespace, ws_endl_0d, 0x0d, true, true)
BENCHMARK_NAMED_PARAM(read_whitespace, ws_endl_0e, 0x0e, true, true)
BENCHMARK_NAMED_PARAM(read_whitespace, ws_endl_0f, 0x0f, true, true)
BENCHMARK_NAMED_PARAM(read_whitespace, ws_endl_10, 0x10, true, true)

int main(int argc, char** argv) {
  folly::Init init(&argc, &argv);
  folly::runBenchmarks();
  return 0;
}
