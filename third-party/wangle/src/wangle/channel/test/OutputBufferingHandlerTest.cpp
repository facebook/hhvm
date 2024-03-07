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

#include <folly/io/async/AsyncSocket.h>
#include <folly/portability/GMock.h>
#include <folly/portability/GTest.h>
#include <wangle/channel/OutputBufferingHandler.h>
#include <wangle/channel/StaticPipeline.h>
#include <wangle/channel/test/MockHandler.h>

using namespace folly;
using namespace wangle;
using namespace testing;

using MockBytesHandler =
    StrictMock<MockHandlerAdapter<IOBufQueue&, std::unique_ptr<IOBuf>>>;

MATCHER_P(IOBufContains, str, "") {
  return arg->moveToFbString() == str;
}

TEST(OutputBufferingHandlerTest, Basic) {
  MockBytesHandler mockHandler;
  EXPECT_CALL(mockHandler, attachPipeline(_));
  auto pipeline = StaticPipeline<
      IOBufQueue&,
      std::unique_ptr<IOBuf>,
      MockBytesHandler,
      OutputBufferingHandler>::create(&mockHandler, OutputBufferingHandler());

  EventBase eb;
  auto socket = AsyncSocket::newSocket(&eb);
  pipeline->setTransport(std::move(socket));

  // Buffering should prevent writes until the EB loops, and the writes should
  // be batched into one write call.
  auto f1 = pipeline->write(IOBuf::copyBuffer("hello"));
  auto f2 = pipeline->write(IOBuf::copyBuffer("world"));
  EXPECT_FALSE(f1.isReady());
  EXPECT_FALSE(f2.isReady());
  EXPECT_CALL(mockHandler, write_(_, IOBufContains("helloworld")));
  eb.loopOnce();
  EXPECT_TRUE(f1.isReady());
  EXPECT_TRUE(f2.isReady());
  EXPECT_CALL(mockHandler, detachPipeline(_));

  // Make sure the SharedPromise resets correctly
  auto f = pipeline->write(IOBuf::copyBuffer("foo"));
  EXPECT_FALSE(f.isReady());
  EXPECT_CALL(mockHandler, write_(_, IOBufContains("foo")));
  eb.loopOnce();
  EXPECT_TRUE(f.isReady());
  pipeline.reset();
}
