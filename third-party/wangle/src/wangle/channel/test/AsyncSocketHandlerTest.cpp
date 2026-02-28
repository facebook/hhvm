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

#include <folly/portability/GTest.h>
#include <wangle/channel/AsyncSocketHandler.h>
#include <wangle/channel/Pipeline.h>
#include <wangle/channel/test/MockHandler.h>
#include <wangle/channel/test/MockPipeline.h>

using namespace folly;
using namespace testing;
using namespace wangle;

TEST(AsyncSocketHandlerTest, WriteErrOnShutdown) {
  InSequence dummy;

  EventBase evb;
  auto socket = AsyncSocket::newSocket(&evb);
  StrictMock<MockPipelineManager> manager;
  auto pipeline = DefaultPipeline::create();
  pipeline->setPipelineManager(&manager);
  pipeline->addBack(AsyncSocketHandler(std::move(socket))).finalize();

  // close() the pipeline multiple times.
  // deletePipeline should only be called once.
  EXPECT_CALL(manager, deletePipeline(_)).Times(1);
  pipeline->close();
  pipeline->close();
}

TEST(AsyncSocketHandlerTest, TransportActiveInactive) {
  InSequence dummy;

  EventBase evb;
  auto socket = AsyncSocket::newSocket(&evb);
  auto handler = std::make_shared<StrictMock<MockBytesToBytesHandler>>();
  auto pipeline = DefaultPipeline::create();
  pipeline->addBack(AsyncSocketHandler(std::move(socket)));
  pipeline->addBack(handler);
  pipeline->finalize();

  EXPECT_CALL(*handler, transportActive(_)).Times(1);
  pipeline->transportActive();
  EXPECT_CALL(*handler, transportInactive(_)).Times(1);
  pipeline->transportInactive();
  EXPECT_CALL(*handler, transportActive(_)).Times(1);
  pipeline->transportActive();
  // Transport is currently active. Calling pipeline->close()
  // should result in transportInactive being fired.
  EXPECT_CALL(*handler, mockClose(_))
      .WillOnce(Return(handler->defaultFuture()));
  EXPECT_CALL(*handler, transportInactive(_)).Times(1);
  pipeline->close();

  socket = AsyncSocket::newSocket(&evb);
  handler = std::make_shared<StrictMock<MockBytesToBytesHandler>>();
  pipeline = DefaultPipeline::create();
  pipeline->addBack(AsyncSocketHandler(std::move(socket)));
  pipeline->addBack(handler);
  pipeline->finalize();

  EXPECT_CALL(*handler, transportActive(_)).Times(1);
  pipeline->transportActive();
  EXPECT_CALL(*handler, transportInactive(_)).Times(1);
  pipeline->transportInactive();
  // Transport is currently inactive. Calling pipeline->close()
  // should not result in transportInactive being fired.
  EXPECT_CALL(*handler, mockClose(_))
      .WillOnce(Return(handler->defaultFuture()));
  EXPECT_CALL(*handler, transportInactive(_)).Times(0);
  pipeline->close();
}
