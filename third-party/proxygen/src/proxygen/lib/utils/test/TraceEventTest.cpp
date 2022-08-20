/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <proxygen/lib/utils/TraceEvent.h>

#include <proxygen/lib/utils/Exception.h>
#include <proxygen/lib/utils/TraceEventType.h>
#include <proxygen/lib/utils/TraceFieldType.h>

#include <folly/portability/GMock.h>
#include <folly/portability/GTest.h>

#include <string>
#include <vector>

using namespace proxygen;

TEST(TraceEventTest, IntegralDataIntegralValue) {
  TraceEvent traceEvent((TraceEventType::TotalRequest));

  int64_t data(13);
  traceEvent.addMeta(TraceFieldType::Protocol, data);

  ASSERT_EQ(data,
            traceEvent.getTraceFieldDataAs<int64_t>(TraceFieldType::Protocol));
}

TEST(TraceEventTest, IntegralDataStringValue) {
  TraceEvent traceEvent((TraceEventType::TotalRequest));

  int64_t intData(13);
  traceEvent.addMeta(TraceFieldType::Protocol, intData);

  std::string strData(std::to_string(intData));

  ASSERT_EQ(
      strData,
      traceEvent.getTraceFieldDataAs<std::string>(TraceFieldType::Protocol));
}

TEST(TraceEventTest, IntegralDataVectorValue) {
  TraceEvent traceEvent((TraceEventType::TotalRequest));

  int64_t data(13);
  traceEvent.addMeta(TraceFieldType::Protocol, data);

  ASSERT_THROW(traceEvent.getTraceFieldDataAs<std::vector<std::string>>(
                   TraceFieldType::Protocol),
               Exception);
}

TEST(TraceEventTest, StringDataIntegralValueConvertible) {
  TraceEvent traceEvent((TraceEventType::TotalRequest));

  int64_t intData(13);
  std::string strData(std::to_string(intData));
  traceEvent.addMeta(TraceFieldType::Protocol, strData);

  ASSERT_EQ(intData,
            traceEvent.getTraceFieldDataAs<int64_t>(TraceFieldType::Protocol));
}

TEST(TraceEventTest, StringDataIntegralValueNonConvertible) {
  TraceEvent traceEvent((TraceEventType::TotalRequest));

  std::string data("Abc");
  traceEvent.addMeta(TraceFieldType::Protocol, data);

  ASSERT_ANY_THROW(
      traceEvent.getTraceFieldDataAs<int64_t>(TraceFieldType::Protocol));
}

TEST(TraceEventTest, StringDataStringValue) {
  TraceEvent traceEvent((TraceEventType::TotalRequest));

  std::string data("Abc");
  traceEvent.addMeta(TraceFieldType::Protocol, data);

  ASSERT_EQ(
      data,
      traceEvent.getTraceFieldDataAs<std::string>(TraceFieldType::Protocol));
}

TEST(TraceEventTest, StringDataVectorValue) {
  TraceEvent traceEvent((TraceEventType::TotalRequest));

  std::string data("Abc");
  traceEvent.addMeta(TraceFieldType::Protocol, data);

  ASSERT_THROW(traceEvent.getTraceFieldDataAs<std::vector<std::string>>(
                   TraceFieldType::Protocol),
               Exception);
}

TEST(TraceEventTest, VectorDataIntegralValue) {
  TraceEvent traceEvent((TraceEventType::TotalRequest));

  std::vector<std::string> data;
  data.push_back("Abc");
  data.push_back("Hij");
  data.push_back("Xyz");
  traceEvent.addMeta(TraceFieldType::Protocol, data);

  ASSERT_THROW(
      traceEvent.getTraceFieldDataAs<int64_t>(TraceFieldType::Protocol),
      Exception);
}

TEST(TraceEventTest, VectorDataStringValue) {
  TraceEvent traceEvent((TraceEventType::TotalRequest));

  std::vector<std::string> data;
  data.push_back("A");
  data.push_back("B");
  data.push_back("C");
  traceEvent.addMeta(TraceFieldType::Protocol, data);

  ASSERT_EQ(
      "[\"A\",\"B\",\"C\"]",
      traceEvent.getTraceFieldDataAs<std::string>(TraceFieldType::Protocol));
}

TEST(TraceEventTest, VectorDataVectorValue) {
  TraceEvent traceEvent((TraceEventType::TotalRequest));

  std::vector<std::string> data;
  data.push_back("A");
  data.push_back("B");
  data.push_back("C");
  traceEvent.addMeta(TraceFieldType::Protocol, data);

  std::vector<std::string> extractedData(
      traceEvent.getTraceFieldDataAs<std::vector<std::string>>(
          TraceFieldType::Protocol));

  EXPECT_THAT(extractedData, testing::ContainerEq(data));
}

TEST(TraceEventTest, IteratorValueTypeCheckInteger) {
  TraceEvent traceEvent((TraceEventType::TotalRequest));
  int64_t intData(13);
  traceEvent.addMeta(TraceFieldType::Protocol, intData);

  auto itr = traceEvent.getMetaDataItr();
  ASSERT_TRUE(itr.isValid());
  ASSERT_EQ(TraceFieldType::Protocol, itr.getKey());
  ASSERT_EQ(typeid(int64_t), itr.type());

  itr.next();
  ASSERT_FALSE(itr.isValid());
}

TEST(TraceEventTest, IteratorValueTypeCheckString) {
  TraceEvent traceEvent((TraceEventType::TotalRequest));
  std::string strData("abc");
  traceEvent.addMeta(TraceFieldType::Protocol, strData);

  auto itr = traceEvent.getMetaDataItr();
  ASSERT_TRUE(itr.isValid());
  ASSERT_EQ(TraceFieldType::Protocol, itr.getKey());
  ASSERT_EQ(typeid(std::string), itr.type());

  itr.next();
  ASSERT_FALSE(itr.isValid());
}

TEST(TraceEventTest, IteratorValueTypeCheckStringArray) {
  TraceEvent traceEvent((TraceEventType::TotalRequest));
  std::vector<std::string> arrData;
  arrData.push_back("A");
  arrData.push_back("B");
  arrData.push_back("C");
  traceEvent.addMeta(TraceFieldType::Protocol, arrData);

  auto itr = traceEvent.getMetaDataItr();
  ASSERT_TRUE(itr.isValid());
  ASSERT_EQ(TraceFieldType::Protocol, itr.getKey());
  ASSERT_EQ(typeid(std::vector<std::string>), itr.type());

  itr.next();
  ASSERT_FALSE(itr.isValid());
}

// To
TEST(TraceEventTest, IntegerValueToString) {
  TraceEvent traceEvent(TraceEventType::TotalRequest, 1);
  traceEvent.start(TimePoint(std::chrono::milliseconds(100)));
  traceEvent.end(TimePoint(std::chrono::milliseconds(200)));
  int64_t intData(13);
  traceEvent.addMeta(TraceFieldType::Protocol, intData);

  std::ostringstream out;
  out << "TraceEvent(";
  out << "type='TotalRequest', ";
  out << "id='" << traceEvent.getID() << "', ";
  out << "parentID='1', ";
  out << "start='100', ";
  out << "end='200', ";
  out << "metaData='{protocol: 13, }')";

  ASSERT_EQ(out.str(), traceEvent.toString());
}

TEST(TraceEventTest, StringValueToString) {
  TraceEvent traceEvent(TraceEventType::TotalRequest, 1);
  traceEvent.start(TimePoint(std::chrono::milliseconds(100)));
  traceEvent.end(TimePoint(std::chrono::milliseconds(200)));
  std::string strData("abc");
  traceEvent.addMeta(TraceFieldType::Protocol, strData);

  std::ostringstream out;
  out << "TraceEvent(";
  out << "type='TotalRequest', ";
  out << "id='" << traceEvent.getID() << "', ";
  out << "parentID='1', ";
  out << "start='100', ";
  out << "end='200', ";
  out << "metaData='{protocol: abc, }')";

  ASSERT_EQ(out.str(), traceEvent.toString());
}

TEST(TraceEventTest, StringArrayValueToString) {
  TraceEvent traceEvent(TraceEventType::TotalRequest, 1);
  traceEvent.start(TimePoint(std::chrono::milliseconds(100)));
  traceEvent.end(TimePoint(std::chrono::milliseconds(200)));
  std::vector<std::string> arrData;
  arrData.push_back("A");
  arrData.push_back("B");
  arrData.push_back("C");
  traceEvent.addMeta(TraceFieldType::Protocol, arrData);

  std::ostringstream out;
  out << "TraceEvent(";
  out << "type='TotalRequest', ";
  out << "id='" << traceEvent.getID() << "', ";
  out << "parentID='1', ";
  out << "start='100', ";
  out << "end='200', ";
  out << "metaData='{protocol: [\"A\",\"B\",\"C\"], }')";

  ASSERT_EQ(out.str(), traceEvent.toString());
}
