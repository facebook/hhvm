/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <folly/Benchmark.h>
#include <folly/Range.h>
#include <proxygen/lib/http/codec/compress/test/TestStreamingCallback.h>
#include <proxygen/lib/http/codec/compress/test/TestUtil.h>

#include <algorithm>

using namespace std;
using namespace folly;
using namespace proxygen;
using proxygen::HPACKHeader;

unique_ptr<IOBuf> encode(vector<HPACKHeader>& headers, HPACKEncoder& encoder) {
  return encoder.encode(headers);
}

void encodeDecode(vector<HPACKHeader>& headers,
                  HPACKEncoder& encoder,
                  HPACKDecoder& decoder) {
  unique_ptr<IOBuf> encoded = encode(headers, encoder);
  CHECK(encoded);
  TestStreamingCallback cb;
  folly::io::Cursor c(encoded.get());
  decoder.decodeStreaming(c, c.totalLength(), &cb);
  CHECK(!cb.hasError());
}

vector<HPACKHeader> getHeaders() {
  vector<HPACKHeader> headers;
  headers.emplace_back(":authority", "www.facebook.com");
  headers.emplace_back(":method", "GET");
  headers.emplace_back(":path", "/graphql");
  headers.emplace_back(":scheme", "https");
  headers.emplace_back(
      "user-agent",
      "Mozilla/5.0 (Macintosh; Intel Mac OS X 10_12_6) AppleWebKit/537.36 "
      "(KHTML, like Gecko) Chrome/60.0.3100.0 Safari/537.36");
  headers.emplace_back("accept-encoding", "gzip, deflate, br");
  headers.emplace_back("accept-language", "en-US,en;q=0.8");
  headers.emplace_back(
      "accept",
      "text/html,application/xhtml+xml,application/xml;q=0.9,image/"
      "webp,image/apng,*/*;q=0.8");
  return headers;
}

namespace {
static vector<HPACKHeader> headers = getHeaders();
}

void encodeBench(int reencodes, int iters) {
  for (int i = 0; i < iters; i++) {
    HPACKEncoder encoder(true);
    encode(headers, encoder);
    for (int j = 0; j < reencodes; j++) {
      encode(headers, encoder);
    }
  }
}

void encodeDecodeBench(int reencodes, int iters) {
  for (int i = 0; i < iters; i++) {
    HPACKEncoder encoder(true);
    HPACKDecoder decoder;
    encodeDecode(headers, encoder, decoder);
    for (int j = 0; j < reencodes; j++) {
      encodeDecode(headers, encoder, decoder);
    }
  }
}

BENCHMARK(Encode, iters) {
  encodeBench(0, iters);
}

BENCHMARK(Encode1, iters) {
  encodeBench(1, iters);
}

BENCHMARK(Encode2, iters) {
  encodeBench(2, iters);
}

BENCHMARK(EncodeDecode, iters) {
  encodeDecodeBench(0, iters);
}

BENCHMARK(EncodeDecode1, iters) {
  encodeDecodeBench(1, iters);
}

BENCHMARK(EncodeDecode2, iters) {
  encodeDecodeBench(2, iters);
}

int main(int argc, char** argv) {
  gflags::ParseCommandLineFlags(&argc, &argv, true);
  folly::runBenchmarks();
  return 0;
}
