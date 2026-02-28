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

#include <thrift/lib/cpp2/async/tests/util/BiDiTestUtil.h>

namespace apache::thrift::detail::test {

std::unique_ptr<folly::IOBuf> makePayload(const std::string& s) {
  folly::IOBufQueue q;
  CompactProtocolWriter writer;
  writer.setOutput(&q);
  writer.writeStructBegin("");
  writer.writeFieldBegin("field", TType::T_STRING, 0);
  writer.writeString(s);
  writer.writeFieldEnd();
  writer.writeFieldStop();
  writer.writeStructEnd();
  return q.move();
}

std::unique_ptr<folly::IOBuf> makeRequest(const std::string& method) {
  folly::IOBufQueue q;
  CompactProtocolWriter writer;
  writer.setOutput(&q);
  writer.writeMessageBegin(method, MessageType::T_CALL, 0);
  writer.writeStructBegin("");
  writer.writeFieldStop();
  writer.writeStructEnd();
  writer.writeMessageEnd();
  return q.move();
}

std::unique_ptr<folly::IOBuf> makeResponse(const std::string& s) {
  folly::IOBufQueue q;
  CompactProtocolWriter writer;
  writer.setOutput(&q);
  writer.writeMessageBegin("test", MessageType::T_REPLY, 0);
  writer.writeStructBegin("");
  writer.writeFieldBegin("field", TType::T_STRING, 0);
  writer.writeString(s);
  writer.writeFieldEnd();
  writer.writeFieldStop();
  writer.writeStructEnd();
  writer.writeMessageEnd();
  return q.move();
}

StreamPayload makeStreamPayload() {
  return StreamPayload(makePayload("stream-payload"), {});
}

StreamPayload makeSinkPayload() {
  return StreamPayload(makePayload("sink-payload"), {});
}

} // namespace apache::thrift::detail::test
