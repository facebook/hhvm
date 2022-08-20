/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <proxygen/lib/http/codec/compress/experimental/simulator/HPACKQueue.h>
#include <proxygen/lib/http/codec/compress/test/TestStreamingCallback.h>
#include <proxygen/lib/http/codec/compress/test/TestUtil.h>

#include <folly/portability/GTest.h>

using namespace folly;
using namespace proxygen::compress;
using namespace proxygen;
using namespace proxygen::hpack;

namespace {
uint64_t bufLen(const std::unique_ptr<IOBuf>& buf) {
  if (buf) {
    return buf->computeChainDataLength();
  }
  return 0;
}

} // namespace

class HPACKQueueTests : public testing::TestWithParam<int> {
 public:
  HPACKQueueTests() : queue(std::make_unique<HPACKQueue>(server)) {
  }

 protected:
  HPACKCodec client{TransportDirection::UPSTREAM};
  HPACKCodec server{TransportDirection::DOWNSTREAM};
  std::unique_ptr<HPACKQueue> queue;
};

TEST_F(HPACKQueueTests, QueueInline) {
  std::vector<Header> req = basicHeaders();
  TestStreamingCallback cb;

  for (int i = 0; i < 3; i++) {
    std::unique_ptr<IOBuf> encodedReq = client.encode(req);
    auto len = bufLen(encodedReq);
    cb.reset();
    queue->enqueueHeaderBlock(i, std::move(encodedReq), len, &cb, false);
    auto result = cb.getResult();
    EXPECT_TRUE(!result.hasError());
    EXPECT_EQ(result->headers.size(), 12);
  }
}

TEST_F(HPACKQueueTests, QueueReorder) {
  std::vector<Header> req = basicHeaders();
  std::vector<std::pair<std::unique_ptr<IOBuf>, TestStreamingCallback>> data;

  data.reserve(4);
  for (int i = 0; i < 4; i++) {
    data.emplace_back(client.encode(req), TestStreamingCallback());
  }

  std::vector<int> insertOrder{1, 3, 2, 0};
  for (auto i : insertOrder) {
    auto& encodedReq = data[i].first;
    auto len = bufLen(encodedReq);
    queue->enqueueHeaderBlock(
        i, std::move(encodedReq), len, &data[i].second, false);
  }
  for (auto& d : data) {
    auto result = d.second.getResult();
    EXPECT_TRUE(!result.hasError());
    EXPECT_EQ(result->headers.size(), 12);
  }
  EXPECT_EQ(queue->getHolBlockCount(), 3);
}

TEST_F(HPACKQueueTests, QueueReorderOoo) {
  std::vector<Header> req = basicHeaders();
  std::vector<std::pair<std::unique_ptr<IOBuf>, TestStreamingCallback>> data;

  data.reserve(4);
  for (int i = 0; i < 4; i++) {
    data.emplace_back(client.encode(req), TestStreamingCallback());
  }

  std::vector<int> insertOrder{0, 3, 2, 1};
  for (auto i : insertOrder) {
    auto& encodedReq = data[i].first;
    auto len = bufLen(encodedReq);
    // Allow idx 3 to be decoded out of order
    queue->enqueueHeaderBlock(
        i, std::move(encodedReq), len, &data[i].second, i == 3);
  }
  for (auto& d : data) {
    auto result = d.second.getResult();
    EXPECT_TRUE(!result.hasError());
    EXPECT_EQ(result->headers.size(), 12);
  }
  EXPECT_EQ(queue->getHolBlockCount(), 1);
}

TEST_F(HPACKQueueTests, QueueError) {
  std::vector<Header> req = basicHeaders();
  TestStreamingCallback cb;

  bool expectOk = true;
  // ok, dup, ok, lower
  for (auto i : std::vector<int>({0, 0, 1, 0, 3, 3, 2})) {
    std::unique_ptr<IOBuf> encodedReq = client.encode(req);
    auto len = bufLen(encodedReq);
    cb.reset();
    queue->enqueueHeaderBlock(i, std::move(encodedReq), len, &cb, true);
    auto result = cb.getResult();
    if (expectOk) {
      EXPECT_TRUE(!result.hasError());
      EXPECT_EQ(result->headers.size(), 12);
    } else {
      EXPECT_TRUE(result.hasError());
      EXPECT_EQ(result.error(), HPACK::DecodeError::BAD_SEQUENCE_NUMBER);
    }
    expectOk = !expectOk;
  }
}

TEST_P(HPACKQueueTests, QueueDeleted) {
  std::vector<Header> req = basicHeaders();
  std::vector<std::pair<std::unique_ptr<IOBuf>, TestStreamingCallback>> data;

  for (int i = 0; i < 4; i++) {
    data.emplace_back(client.encode(req), TestStreamingCallback());
    if (i == GetParam()) {
      data.back().second.headersCompleteCb = [&] { queue.reset(); };
    }
  }

  std::vector<int> insertOrder{0, 3, 2, 1};
  for (auto i : insertOrder) {
    auto& encodedReq = data[i].first;
    auto len = bufLen(encodedReq);

    // Allow idx 3 to be decoded out of order
    queue->enqueueHeaderBlock(
        i, std::move(encodedReq), len, &data[i].second, i == 3);
    if (!queue) {
      break;
    }
  }
}

INSTANTIATE_TEST_SUITE_P(Queue, HPACKQueueTests, ::testing::Values(0, 1, 2, 3));
