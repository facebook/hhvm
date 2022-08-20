/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <algorithm>
#include <folly/Benchmark.h>
#include <proxygen/lib/http/HTTPCommonHeaders.h>
#include <proxygen/lib/http/HTTPHeaders.h>

using namespace proxygen;

// buck build @mode/opt proxygen/lib/http/test:http_headers_benchmark
// ./buck-out/gen/proxygen/lib/http/test/http_headers_benchmark -bm_min_iters
// 100
// ============================================================================
// proxygen/lib/http/test/HTTPHeadersBenchmark.cpp relative  time/iter  iters/s
// ============================================================================
// HTTPCommonHeadersHash                                        3.50us  285.82K
// HTTPCommonHeadersGetHeaderCodeFromTableCommonHeaderName    161.70ns    6.18M
// memchr                                                       1.02us  976.02K
// stdFind                                                      5.59us  178.94K
// ============================================================================

namespace {

std::vector<HTTPHeaderCode> getTestHeaderCodes() {
  std::vector<HTTPHeaderCode> testHeaderCodes;
  for (uint64_t j = HTTPHeaderCodeCommonOffset;
       j < HTTPCommonHeaders::num_codes;
       ++j) {
    testHeaderCodes.push_back(static_cast<HTTPHeaderCode>(j));
  }
  return testHeaderCodes;
}

std::vector<const std::string*> getTestHeaderStrings() {
  std::vector<const std::string*> testHeaderStrings;
  for (uint64_t j = HTTPHeaderCodeCommonOffset;
       j < HTTPCommonHeaders::num_codes;
       ++j) {
    testHeaderStrings.push_back(
        HTTPCommonHeaders::getPointerToName(static_cast<HTTPHeaderCode>(j)));
  }
  return testHeaderStrings;
}

static const std::string* testHeaderNames =
    HTTPCommonHeaders::getPointerToName(HTTP_HEADER_NONE);

static const std::vector<HTTPHeaderCode> testHeaderCodes = getTestHeaderCodes();

static const std::vector<const std::string*> testHeaderStrings =
    getTestHeaderStrings();

} // namespace

void HTTPCommonHeadersHashBench(int iters) {
  for (int i = 0; i < iters; ++i) {
    for (auto const& testHeaderString : testHeaderStrings) {
      HTTPCommonHeaders::hash(*testHeaderString);
    }
  }
}

void HTTPCommonHeadersGetHeaderCodeFromTableCommonHeaderNameBench(int iters) {
  for (int i = 0; i < iters; ++i) {
    for (uint64_t j = HTTPHeaderCodeCommonOffset;
         j < HTTPCommonHeaders::num_codes;
         ++j) {
      HTTPCommonHeaders::getCodeFromTableName(
          &testHeaderNames[j], HTTPCommonHeaderTableType::TABLE_CAMELCASE);
    }
  }
}

BENCHMARK(HTTPCommonHeadersHash, iters) {
  HTTPCommonHeadersHashBench(iters);
}

BENCHMARK(HTTPCommonHeadersGetHeaderCodeFromTableCommonHeaderName, iters) {
  HTTPCommonHeadersGetHeaderCodeFromTableCommonHeaderNameBench(iters);
}

void memchrBench(int iters) {
  for (int i = 0; i < iters; ++i) {
    for (uint64_t j = HTTPHeaderCodeCommonOffset;
         j < HTTPCommonHeaders::num_codes;
         ++j) {
      CHECK(memchr((void*)testHeaderCodes.data(),
                   static_cast<HTTPHeaderCode>(j),
                   testHeaderCodes.size()) != nullptr);
    }
  }
}

void stdFindBench(int iters) {
  for (int i = 0; i < iters; ++i) {
    for (uint64_t j = HTTPHeaderCodeCommonOffset;
         j < HTTPCommonHeaders::num_codes;
         ++j) {
      auto address =
          HTTPCommonHeaders::getPointerToName(static_cast<HTTPHeaderCode>(j));
      CHECK(std::find(testHeaderStrings.begin(),
                      testHeaderStrings.end(),
                      address) != testHeaderStrings.end());
    }
  }
}

BENCHMARK(memchr, iters) {
  memchrBench(iters);
}

BENCHMARK(stdFind, iters) {
  stdFindBench(iters);
}

void addCodeBench(int nHeaders, int hdrSize, int iters) {
  std::string value(hdrSize, 'a');
  for (int i = 0; i < iters; ++i) {
    HTTPHeaders headers;
    for (int j = 0; j < nHeaders; ++j) {
      headers.add(HTTP_HEADER_HOST, value);
    }
  }
}

BENCHMARK(addCode4_headers_8_length, iters) {
  addCodeBench(4, 8, iters);
}

BENCHMARK(addCode4_headers_32_length, iters) {
  addCodeBench(4, 32, iters);
}

BENCHMARK(addCode16_headers_8_length, iters) {
  addCodeBench(16, 8, iters);
}

BENCHMARK(addCode16_headers_32_length, iters) {
  addCodeBench(16, 32, iters);
}

BENCHMARK(addCode24_headers_8_length, iters) {
  addCodeBench(24, 8, iters);
}

BENCHMARK(addCode24_headers_32_length, iters) {
  addCodeBench(24, 32, iters);
}

int main(int argc, char** argv) {
  gflags::ParseCommandLineFlags(&argc, &argv, true);
  folly::runBenchmarks();
  return 0;
}
