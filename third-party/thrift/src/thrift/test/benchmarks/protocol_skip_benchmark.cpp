/*
 * Copyright (c) Facebook, Inc. and its affiliates.
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

#include <string_view>
#include <folly/Benchmark.h>
#include <folly/Memory.h>
#include <folly/Random.h>
#include <folly/Traits.h>
#include <folly/container/Foreach.h>
#include <folly/init/Init.h>
#include <folly/lang/Pretty.h>
#include <thrift/lib/cpp2/protocol/Serializer.h>
#include <thrift/test/testset/gen-cpp2/testset_types.h>

namespace apache::thrift::test {
namespace {

const size_t kContainerSize = 1000;
const size_t kIterBaseCount = 1000;

template <class T>
void randomize(T& s) {
  s = static_cast<T>(folly::Random::rand64());
}

template <template <class...> class C, class T>
void randomize(C<T>& s) {
  s.resize(kContainerSize);
  std::for_each(s.begin(), s.end(), randomize<T>);
}

template <class Reader, class Struct>
void add_benchmark(
    std::string_view prefix,
    std::string_view suffix,
    const std::vector<folly::IOBuf>& bufs) {
  std::string structName = folly::pretty_name<Struct>();
  structName = structName.substr(structName.rfind("::") + 2);
  const char* readerName =
      std::is_base_of_v<CompactProtocolReader, Reader> ? "Compact" : "Binary";

  folly::addBenchmark(
      __FILE__,
      fmt::format("{}{}_{}_{}", prefix, readerName, structName, suffix),
      [bufs] {
        folly::BenchmarkSuspender susp;
        for (auto&& buf : bufs) {
          std::string name;
          TType fieldType;
          int16_t fieldId;
          Reader reader;
          reader.setInput(&buf);
          reader.readStructBegin(name);
          reader.readFieldBegin(name, fieldType, fieldId);
          susp.dismiss();
          apache::thrift::skip(reader, fieldType);
          susp.rehire();
        }
        return bufs.size();
      });
}

template <class... T>
using type_list = std::tuple<folly::tag_t<T>...>;

template <class Reader, class Struct>
void add_benchmarks(folly::tag_t<Reader>, folly::tag_t<Struct>) {
  std::vector<folly::IOBuf> bufs(kIterBaseCount);
  for (auto&& buf : bufs) {
    Struct obj;
    randomize(obj.field_1_ref().ensure());
    folly::IOBufQueue q;
    Serializer<Reader, typename Reader::ProtocolWriter>::serialize(obj, &q);
    buf = q.moveAsValue();
    buf.coalesce();
  }

  struct FastReader : Reader {
    // If we use Reader directly, instead of FastReader, compiler will perform
    // inline expansion differently, which produces different result
    static size_t fixedSizeInContainer(TType t) {
      return Reader::fixedSizeInContainer(t);
    }
  };
  struct SlowReader : Reader {
    static size_t fixedSizeInContainer(TType) { return 0; }
  };

  add_benchmark<FastReader, Struct>("", "skip", bufs);
  add_benchmark<SlowReader, Struct>("%", "slow_skip", bufs);
}

void addReflectionBenchmarks() {
  type_list<CompactProtocolReader, BinaryProtocolReader> readerTypes;
  type_list<
      testset::struct_bool,
      testset::struct_byte,
      testset::struct_float,
      testset::struct_double,
      testset::struct_string,
      testset::struct_i16,
      testset::struct_i32,
      testset::struct_i64,
      testset::struct_list_float,
      testset::struct_list_double,
      testset::struct_list_i16,
      testset::struct_list_i32,
      testset::struct_list_i64>
      structTypes;
  folly::for_each(readerTypes, [&](auto readerType) {
    folly::for_each(structTypes, [&](auto structType) {
      add_benchmarks(readerType, structType);
    });
  });
}

} // namespace
} // namespace apache::thrift::test

int main(int argc, char** argv) {
  folly::init(&argc, &argv);
  apache::thrift::test::addReflectionBenchmarks();
  folly::runBenchmarks();
}
