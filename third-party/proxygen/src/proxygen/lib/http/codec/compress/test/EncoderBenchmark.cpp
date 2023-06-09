/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <folly/Benchmark.h>
#include <folly/String.h>
#include <iostream>
#include <proxygen/lib/http/HTTPHeaders.h>
#include <proxygen/lib/http/codec/compress/HPACKEncoder.h>
#include <string>

using namespace proxygen;

const HTTPHeaders& getHeaders() {
  static bool initHeaders = true;
  static HTTPHeaders headers;
  if (initHeaders) {
    initHeaders = false;
    for (std::string line; std::getline(std::cin, line, '\n');) {

      std::vector<folly::StringPiece> pieces;
      folly::split(' ', line, pieces);
      CHECK_EQ(pieces[3][0], 'n');
      std::string name(pieces[3].begin() + 2, pieces[3].size() - 2);
      auto valueOff = pieces[3].end() + 1 - line.data();
      folly::StringPiece valuePiece(line.data() + valueOff,
                                    line.size() - valueOff);
      CHECK_EQ(valuePiece[0], 'v');
      std::string value(valuePiece.begin() + 2, valuePiece.size() - 2);
      headers.add(name, value);
    }
  }
  return headers;
}

void encoderBenchmark(uint32_t iters, bool huffman, uint32_t tableSize) {
  const HTTPHeaders* headers = nullptr;
  BENCHMARK_SUSPEND {
    headers = &getHeaders();
  }
  uint64_t compressed = 0;
  for (auto i = 0u; i < iters; i++) {
    HPACKEncoder encoder(huffman, tableSize);
    folly::IOBufQueue writeBuf{folly::IOBufQueue::cacheChainLength()};
    bool first = true;
    headers->forEachWithCode([&](HTTPHeaderCode code,
                                 const std::string& name,
                                 const std::string& value) {
      if (code == HTTP_HEADER_COLON_METHOD) {
        if (first) {
          first = false;
        } else {
          encoder.completeEncode();
          compressed += writeBuf.chainLength();
          writeBuf.move();
        }
        encoder.startEncode(writeBuf);
      }
      if (code == HTTP_HEADER_OTHER) {
        encoder.encodeHeader(name, value);
      } else {
        encoder.encodeHeader(code, value);
      }
    });

    compressed += writeBuf.chainLength();
    encoder.completeEncode();
  }
  LOG(INFO) << "compressed=" << compressed / iters;
}

BENCHMARK(EncoderHuffman4096, iters) {
  encoderBenchmark(iters, true, 4096);
}

BENCHMARK(EncoderNoHuffman4096, iters) {
  encoderBenchmark(iters, false, 4096);
}

BENCHMARK(EncoderHuffman0, iters) {
  encoderBenchmark(iters, true, 0);
}

BENCHMARK(EncoderNoHuffman0, iters) {
  encoderBenchmark(iters, false, 0);
}

int main(int argc, char** argv) {
  gflags::ParseCommandLineFlags(&argc, &argv, true);
  google::InitGoogleLogging(argv[0]);
  folly::runBenchmarks();
  return 0;
}
