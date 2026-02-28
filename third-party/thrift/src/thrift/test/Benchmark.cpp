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

#include <thrift/lib/cpp2/protocol/BinaryProtocol.h>
#include <thrift/lib/cpp2/protocol/CompactProtocol.h>
#include <thrift/test/gen-cpp2/DebugProtoTest_types.h>

#include <folly/Benchmark.h>
#include <folly/compression/Compression.h>
#include <folly/init/Init.h>

using namespace std;
using namespace apache::thrift;
using namespace apache::thrift::test;

OneOfEach ooe;
ManyOfOneOfEach many;

template <typename Reader, typename Obj>
void runTestWrite(
    const Obj& obj, int iters, bool compress, bool single_alloc = false) {
  Reader prot;
  size_t bufSize = obj.serializedSizeZC(&prot);
  folly::IOBufQueue queue;
  if (single_alloc) {
    auto largeBuf = folly::IOBuf::create(bufSize);
    queue.append(std::move(largeBuf));
  }

  for (int i = 0; i < iters; i++) {
    queue.clearAndTryReuseLargestBuffer();
    prot.setOutput(&queue, bufSize);
    obj.write(&prot);
    auto buf = queue.move();
    if (compress) {
      buf = folly::compression::getCodec(folly::compression::CodecType::ZSTD)
                ->compress(buf.get());
    }
    queue.append(std::move(buf));
  }
}

BENCHMARK(LargeWrite_Compact, iters) {
  runTestWrite<CompactProtocolWriter>(many, iters, false);
}

BENCHMARK(LargeWrite_Compact_Compressed, iters) {
  runTestWrite<CompactProtocolWriter>(many, iters, true);
}

BENCHMARK(LargeWrite_Binary, iters) {
  runTestWrite<BinaryProtocolWriter>(many, iters, false);
}

BENCHMARK(LargeWrite_Binary_Compressed, iters) {
  runTestWrite<BinaryProtocolWriter>(many, iters, true);
}

BENCHMARK(LargeWrite_Compact_Prealloc, iters) {
  runTestWrite<CompactProtocolWriter>(many, iters, false, true);
}

BENCHMARK(LargeWrite_Compact_Compressed_Prealloc, iters) {
  runTestWrite<CompactProtocolWriter>(many, iters, true, true);
}

BENCHMARK(LargeWrite_Binary_Prealloc, iters) {
  runTestWrite<BinaryProtocolWriter>(many, iters, false, true);
}

BENCHMARK(LargeWrite_Binary_Compressed_Prealloc, iters) {
  runTestWrite<BinaryProtocolWriter>(many, iters, true, true);
}

BENCHMARK(SmallWrite_Compact, iters) {
  runTestWrite<CompactProtocolWriter>(ooe, iters, false);
}

BENCHMARK(SmallWrite_Compact_Compressed, iters) {
  runTestWrite<CompactProtocolWriter>(ooe, iters, true);
}

BENCHMARK(SmallWrite_Binary, iters) {
  runTestWrite<BinaryProtocolWriter>(ooe, iters, false);
}

BENCHMARK(SmallWrite_Binary_Compressed, iters) {
  runTestWrite<BinaryProtocolWriter>(ooe, iters, true);
}

template <typename Reader, typename Writer>
void runTestRead(int iters) {
  OneOfEach ooe2;
  Reader prot;
  std::unique_ptr<folly::IOBuf> buf;

  {
    folly::BenchmarkSuspender susp;
    folly::IOBufQueue queue;
    Writer writer;
    writer.setOutput(&queue);
    ooe.write(&writer);
    buf = queue.move();
  }

  for (int i = 0; i < iters; i++) {
    prot.setInput(buf.get());
    ooe2.read(&prot);
  }
}

BENCHMARK(runTestRead_BinaryProtocolReader, iters) {
  runTestRead<BinaryProtocolReader, BinaryProtocolWriter>(iters);
}

int main(int argc, char** argv) {
  folly::Init init(&argc, &argv);

  *ooe.im_true() = true;
  *ooe.im_false() = false;
  *ooe.a_bite() = 0xd6;
  *ooe.integer16() = 27000;
  *ooe.integer32() = 1 << 24;
  *ooe.integer64() = (uint64_t)6000 * 1000 * 1000;
  *ooe.double_precision() = M_PI;
  *ooe.some_characters() = "JSON THIS! \"\1";
  *ooe.zomg_unicode() = "\xd7\n\a\t";
  *ooe.base64() = "\1\2\3\255";
  ooe.string_string_map()["one"] = "two";
  ooe.string_string_hash_map()["three"] = "four";
  *ooe.float_precision() = (float)12.345;
  ooe.rank_map()[567419810] = (float)0.211184;
  ooe.rank_map()[507959914] = (float)0.080382;

  many.structs().ensure().insert(many.structs()->end(), 10'000, ooe);

  folly::runBenchmarks();

  return 0;
}
