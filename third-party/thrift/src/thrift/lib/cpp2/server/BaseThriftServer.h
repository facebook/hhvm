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

#include <atomic>
#include <chrono>
#include <cstdlib>
#include <map>
#include <memory>
#include <mutex>
#include <stdexcept>
#include <vector>

#include <folly/Memory.h>
#include <folly/Portability.h>
#include <folly/SharedMutex.h>
#include <folly/Synchronized.h>
#include <folly/VirtualExecutor.h>
#include <folly/io/SocketOptionMap.h>
#include <folly/io/async/AsyncTransport.h>
#include <folly/io/async/EventBase.h>
#include <thrift/lib/cpp/concurrency/Thread.h>
#include <thrift/lib/cpp/server/TServerObserver.h>
#include <thrift/lib/cpp/transport/THeader.h>
#include <thrift/lib/cpp2/Flags.h>
#include <thrift/lib/cpp2/Thrift.h>
#include <thrift/lib/cpp2/async/AsyncProcessor.h>
#include <thrift/lib/cpp2/server/ControlServerInterface.h>
#include <thrift/lib/cpp2/server/MonitoringServerInterface.h>
#include <thrift/lib/cpp2/server/SecurityServerInterface.h>
#include <thrift/lib/cpp2/server/ServerAttribute.h>
#include <thrift/lib/cpp2/server/ServerConfigs.h>
#include <thrift/lib/cpp2/server/ServerFlags.h>
#include <thrift/lib/cpp2/server/StatusServerInterface.h>
#include <thrift/lib/cpp2/server/ThriftServerConfig.h>
#include <thrift/lib/cpp2/transport/rocket/framing/parser/AllocatingParserStrategy.h>

namespace wangle {
class ConnectionManager;
}

namespace apache {
namespace thrift {

/**
 *   Base class for thrift servers using cpp2 style generated code.
 */

class BaseThriftServer {
 protected:
  BaseThriftServer() {}
  ~BaseThriftServer() {}

  /*
  The base abstract class has been deprecated.
  Please add all new code to the ThriftServer class.
  */
};
} // namespace thrift
} // namespace apache
