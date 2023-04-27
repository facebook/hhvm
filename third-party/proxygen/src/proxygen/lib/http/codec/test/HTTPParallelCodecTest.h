/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once
#include <folly/portability/GTest.h>
#include <proxygen/lib/http/codec/HTTPParallelCodec.h>
#include <proxygen/lib/http/codec/test/TestUtils.h>
#include <proxygen/lib/utils/Logging.h>

class HTTPParallelCodecTest : public testing::Test {
 public:
  HTTPParallelCodecTest(proxygen::HTTPParallelCodec& upstreamCodec,
                        proxygen::HTTPParallelCodec& downstreamCodec)
      : upstreamCodec_(upstreamCodec), downstreamCodec_(downstreamCodec) {
  }

  void SetUp() override {
    downstreamCodec_.setCallback(&callbacks_);
    upstreamCodec_.setCallback(&callbacks_);
    // Most tests are downstream tests, so generate the upstream conn preface
    // by default
    upstreamCodec_.generateConnectionPreface(output_);
  }

  void SetUpUpstreamTest() {
    output_.move();
    downstreamCodec_.generateConnectionPreface(output_); // no-op
    downstreamCodec_.generateSettings(output_);
  }

  bool parse(std::function<void(folly::IOBuf*)> hackIngress =
                 std::function<void(folly::IOBuf*)>()) {
    return parseImpl(downstreamCodec_, hackIngress);
  }

  bool parseUpstream(std::function<void(folly::IOBuf*)> hackIngress =
                         std::function<void(folly::IOBuf*)>()) {
    return parseImpl(upstreamCodec_, hackIngress);
  }

  /*
   * hackIngress is used to keep the codec's strict checks while having
   * separate checks for tests
   */
  bool parseImpl(proxygen::HTTPParallelCodec& codec,
                 std::function<void(folly::IOBuf*)> hackIngress) {
    dumpToFile(codec.getTransportDirection() ==
               proxygen::TransportDirection::UPSTREAM);
    auto ingress = output_.move();
    if (hackIngress) {
      hackIngress(ingress.get());
    }
    size_t parsed = codec.onIngress(*ingress);
    return (parsed == ingress->computeChainDataLength());
  }

  /*
   * dumpToFile dumps binary frames to files ("/tmp/http2_*.bin"),
   * allowing debugging individual frames.
   * @note: assign true to dump_ to turn on dumpToFile
   */
  void dumpToFile(bool isUpstream = false) {
    if (!dump_) {
      return;
    }
    auto endpoint = isUpstream ? "client" : "server";
    auto filename = folly::to<std::string>(
        "/tmp/parallel_", endpoint, "_", testInfo_->name(), ".bin");
    proxygen::dumpBinToFile(filename, output_.front());
  }

 protected:
  proxygen::FakeHTTPCodecCallback callbacks_;
  proxygen::HTTPParallelCodec& upstreamCodec_;
  proxygen::HTTPParallelCodec& downstreamCodec_;
  folly::IOBufQueue output_{folly::IOBufQueue::cacheChainLength()};
  const testing::TestInfo* testInfo_{
      testing::UnitTest::GetInstance()->current_test_info()};
  bool dump_{false};
};
