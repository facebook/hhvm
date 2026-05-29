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

#include <thrift/lib/cpp2/logging/ThriftConnectionLog.h>

#include <gtest/gtest.h>

namespace apache::thrift {

namespace {

class RecordingCounters : public IThriftServerCounters {
 public:
  int streamSubscribes = 0;
  int sinkSubscribes = 0;
  int biDiSubscribes = 0;

  void onStreamSubscribe(std::string_view /*methodName*/) override {
    ++streamSubscribes;
  }
  void onSinkSubscribe(std::string_view /*methodName*/) override {
    ++sinkSubscribes;
  }
  void onBiDiSubscribe(std::string_view /*methodName*/) override {
    ++biDiSubscribes;
  }
};

} // namespace

class ThriftConnectionLogTest : public ::testing::Test {
 protected:
  RecordingCounters counters_;
};

TEST_F(ThriftConnectionLogTest, CreateStreamLog) {
  ThriftConnectionLog connLog(&counters_, nullptr);
  auto streamLog = connLog.createStreamLog("myStream");
  ASSERT_NE(streamLog, nullptr);

  streamLog->log(detail::StreamSubscribeEvent{});
  EXPECT_EQ(counters_.streamSubscribes, 1);
}

TEST_F(ThriftConnectionLogTest, CreateSinkLog) {
  ThriftConnectionLog connLog(&counters_, nullptr);
  auto sinkLog = connLog.createSinkLog("mySink");
  ASSERT_NE(sinkLog, nullptr);

  sinkLog->log(detail::SinkSubscribeEvent{});
  EXPECT_EQ(counters_.sinkSubscribes, 1);
}

TEST_F(ThriftConnectionLogTest, MultipleLogsShareBackend) {
  ThriftConnectionLog connLog(&counters_, nullptr);
  auto log1 = connLog.createStreamLog("method1");
  auto log2 = connLog.createStreamLog("method2");

  log1->log(detail::StreamSubscribeEvent{});
  log2->log(detail::StreamSubscribeEvent{});
  EXPECT_EQ(counters_.streamSubscribes, 2);
}

TEST_F(ThriftConnectionLogTest, CreateBiDiLog) {
  ThriftConnectionLog connLog(&counters_, nullptr);
  auto biDiLog = connLog.createBiDiLog("myBiDi");
  ASSERT_NE(biDiLog, nullptr);

  biDiLog->log(detail::BiDiSubscribeEvent{});
  EXPECT_EQ(counters_.biDiSubscribes, 1);
}

TEST_F(ThriftConnectionLogTest, NullBackendsReturnsNullptr) {
  ThriftConnectionLog connLog(nullptr, nullptr);
  EXPECT_EQ(connLog.createStreamLog("method"), nullptr);
  EXPECT_EQ(connLog.createSinkLog("method"), nullptr);
  EXPECT_EQ(connLog.createBiDiLog("method"), nullptr);
}

} // namespace apache::thrift
