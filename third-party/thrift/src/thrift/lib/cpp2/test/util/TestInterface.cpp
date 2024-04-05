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

#include <thrift/lib/cpp2/test/util/TestInterface.h>

#include <fmt/core.h>
#include <folly/io/Cursor.h>

const std::string kEchoSuffix(45, 'c');

void TestInterface::sendResponse(std::string& _return, int64_t size) {
  if (size >= 0) {
    usleep(size);
  }

  _return = fmt::format("test{0}", size);
}

void TestInterface::noResponse(int64_t size) {
  usleep(size);
}

void TestInterface::voidResponse() {}

void TestInterface::echoRequest(
    std::string& _return, std::unique_ptr<std::string> req) {
  _return = *req + kEchoSuffix;
}

void TestInterface::async_tm_serializationTest(
    std::unique_ptr<StringCob> callback, bool) {
  std::unique_ptr<std::string> sp(new std::string("hello world"));
  callback->result(std::move(sp));
}

void TestInterface::async_eb_eventBaseAsync(
    std::unique_ptr<StringCob> callback) {
  std::unique_ptr<std::string> hello(new std::string("hello world"));
  callback->result(std::move(hello));
}

void TestInterface::async_tm_notCalledBack(
    std::unique_ptr<apache::thrift::HandlerCallback<void>>) {}

void TestInterface::echoIOBuf(
    std::unique_ptr<folly::IOBuf>& ret, std::unique_ptr<folly::IOBuf> buf) {
  ret = std::move(buf);
  folly::io::Appender cursor(ret.get(), kEchoSuffix.size());
  cursor.push(folly::StringPiece(kEchoSuffix.data(), kEchoSuffix.size()));
}

int32_t TestInterface::processHeader() {
  // Handler method can touch header at any time. Use this to test race
  // condition on per-request THeader.
  auto header = getConnectionContext()->getHeader();
  for (int i = 0; i < 1000000; i++) {
    header->setHeader("foo", "bar");
    header->clearHeaders();
  }
  return 1;
}

void TestInterface::throwsHandlerException() {
  throw std::runtime_error("exception");
}
