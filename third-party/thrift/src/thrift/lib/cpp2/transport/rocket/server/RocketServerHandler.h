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

#pragma once
#include <cstdint>

namespace apache::thrift {

class Cpp2ConnContext;

namespace rocket {

class RocketSinkClientCallback;
class RocketStreamClientCallback;
class RequestChannelFrame;
class RequestFnfFrame;
class RequestResponseFrame;
class RequestStreamFrame;
class SetupFrame;

class IRocketServerConnection;
class RocketServerConnection;
class RocketServerFrameContext;
class RocketBiDiClientCallback;

struct ChannelRequestCallbackFactory;

class RocketServerHandler {
 public:
  virtual ~RocketServerHandler() = default;

  virtual void handleSetupFrame(SetupFrame&&, IRocketServerConnection&) {}

  virtual void handleRequestResponseFrame(
      RequestResponseFrame&& frame, RocketServerFrameContext&& context) = 0;

  virtual void handleRequestFnfFrame(
      RequestFnfFrame&& frame, RocketServerFrameContext&& context) = 0;

  virtual void handleRequestStreamFrame(
      RequestStreamFrame&&,
      RocketServerFrameContext&& context,
      RocketStreamClientCallback*) = 0;

  virtual void handleRequestChannelFrame(
      RequestChannelFrame&&,
      RocketServerFrameContext&& context,
      ChannelRequestCallbackFactory) = 0;

  virtual void requestComplete() {}

  virtual void terminateInteraction(int64_t /*id*/) {}

  virtual void connectionClosing() = 0;

  virtual Cpp2ConnContext* getCpp2ConnContext() { return nullptr; }

  virtual void onBeforeHandleFrame() {}

  virtual int32_t getVersion() const { return 0; }
};

} // namespace rocket
} // namespace apache::thrift
