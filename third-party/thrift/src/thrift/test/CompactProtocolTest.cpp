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

#include <folly/portability/GTest.h>

// #include <thrift/lib/cpp/transport/TVirtualTransport.h>
#include <thrift/lib/cpp/protocol/TCompactProtocol.h>
#include <thrift/lib/cpp/transport/TBufferTransports.h>

using apache::thrift::protocol::TCompactProtocolT;
using apache::thrift::protocol::TMessageType;
using apache::thrift::transport::TFramedTransport;
using apache::thrift::transport::TMemoryBuffer;

namespace apache {
namespace thrift {

class TCompactProtocolTest : public testing::Test {};

void testTMessageWriteAndRead(
    const std::string& name, TMessageType msgType, int32_t seqId) {
  TMemoryBuffer buffer;
  TCompactProtocolT protocol(&buffer);
  protocol.writeMessageBegin(name, msgType, seqId);

  std::string readName;
  TMessageType readMsgType;
  int32_t readSeqId = 0;
  protocol.readMessageBegin(readName, readMsgType, readSeqId);

  EXPECT_EQ(readName, name);
  EXPECT_EQ(readMsgType, msgType);
  EXPECT_EQ(readSeqId, seqId);
}

TEST_F(TCompactProtocolTest, test_readMessageBegin) {
  testTMessageWriteAndRead("methodName", TMessageType::T_CALL, 1);
  testTMessageWriteAndRead("", TMessageType::T_CALL, 1);

  testTMessageWriteAndRead("methodName", TMessageType::T_CALL, 0);
  testTMessageWriteAndRead("methodName", TMessageType::T_CALL, -1);
  testTMessageWriteAndRead(
      "methodName", TMessageType::T_CALL, std::numeric_limits<int32_t>::max());
  testTMessageWriteAndRead(
      "methodName", TMessageType::T_CALL, std::numeric_limits<int32_t>::min());

  testTMessageWriteAndRead("methodName", TMessageType::T_CALL, 1);
  testTMessageWriteAndRead("methodName", TMessageType::T_REPLY, 1);
  testTMessageWriteAndRead("methodName", TMessageType::T_EXCEPTION, 1);
  testTMessageWriteAndRead("methodName", TMessageType::T_ONEWAY, 1);
}

} // namespace thrift
} // namespace apache
