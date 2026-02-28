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

#include <thrift/lib/cpp2/test/util/TestHandler.h>

#include <fmt/core.h>
#include <folly/io/Cursor.h>

const std::string kEchoSuffix(45, 'c');

void TestHandler::sendResponse(std::string& ret, int64_t size) {
  if (size >= 0) {
    usleep(size);
  }

  ret = fmt::format("test{0}", size);
}

void TestHandler::noResponse(int64_t size) {
  usleep(size);
}

void TestHandler::voidResponse() {}

void TestHandler::echoRequest(
    std::string& ret, std::unique_ptr<std::string> req) {
  ret = *req + kEchoSuffix;
}

void TestHandler::async_tm_serializationTest(StringCob::Ptr callback, bool) {
  std::unique_ptr<std::string> sp(new std::string("hello world"));
  callback->result(std::move(sp));
}

void TestHandler::async_eb_eventBaseAsync(StringCob::Ptr callback) {
  std::unique_ptr<std::string> hello(new std::string("hello world"));
  callback->result(std::move(hello));
}

void TestHandler::async_tm_notCalledBack(
    apache::thrift::HandlerCallbackPtr<void>) {}

void TestHandler::echoIOBuf(
    std::unique_ptr<folly::IOBuf>& ret, std::unique_ptr<folly::IOBuf> buf) {
  ret = std::move(buf);
  folly::io::Appender cursor(ret.get(), kEchoSuffix.size());
  cursor.push(folly::StringPiece(kEchoSuffix.data(), kEchoSuffix.size()));
}

int32_t TestHandler::processHeader() {
  // Handler method can touch header at any time. Use this to test race
  // condition on per-request THeader.
  auto header = getConnectionContext()->getHeader();
  for (int i = 0; i < 1000000; i++) {
    header->setHeader("foo", "bar");
    header->clearHeaders();
  }
  return 1;
}

void TestHandler::throwsHandlerException() {
  throw std::runtime_error("exception");
}
