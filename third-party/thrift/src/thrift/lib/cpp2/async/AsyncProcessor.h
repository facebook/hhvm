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

#include <exception>
#include <mutex>
#include <string>
#include <string_view>
#include <variant>

#include <folly/ExceptionWrapper.h>
#include <folly/Portability.h>
#include <folly/String.h>
#include <folly/Synchronized.h>
#include <folly/Unit.h>
#include <folly/concurrency/memory/PrimaryPtr.h>
#include <folly/container/F14Map.h>
#include <folly/futures/Future.h>
#include <folly/io/async/EventBase.h>
#include <folly/lang/Badge.h>

#include <thrift/lib/cpp/TApplicationException.h>
#include <thrift/lib/cpp/TProcessor.h>
#include <thrift/lib/cpp/Thrift.h>
#include <thrift/lib/cpp/concurrency/Thread.h>
#include <thrift/lib/cpp/concurrency/ThreadManager.h>
#include <thrift/lib/cpp/protocol/TProtocolTypes.h>
#include <thrift/lib/cpp/transport/THeader.h>
#include <thrift/lib/cpp2/SerializationSwitch.h>
#include <thrift/lib/cpp2/Thrift.h>
#include <thrift/lib/cpp2/TrustedServerException.h>
#include <thrift/lib/cpp2/async/AsyncProcessorFactory.h>
#include <thrift/lib/cpp2/async/Interaction.h>
#include <thrift/lib/cpp2/async/ReplyInfo.h>
#include <thrift/lib/cpp2/async/ResponseChannel.h>
#include <thrift/lib/cpp2/async/RpcTypes.h>
#include <thrift/lib/cpp2/async/ServerRequestData.h>
#include <thrift/lib/cpp2/async/ServerStream.h>
#include <thrift/lib/cpp2/async/Sink.h>
#include <thrift/lib/cpp2/async/processor/AsyncProcessor.h>
#include <thrift/lib/cpp2/async/processor/AsyncProcessorFunc.h>
#include <thrift/lib/cpp2/async/processor/EventTask.h>
#include <thrift/lib/cpp2/async/processor/GeneratedAsyncProcessorBase.h>
#include <thrift/lib/cpp2/async/processor/HandlerCallback.h>
#include <thrift/lib/cpp2/async/processor/HandlerCallbackBase.h>
#include <thrift/lib/cpp2/async/processor/HandlerCallbackOneWay.h>
#include <thrift/lib/cpp2/async/processor/RequestParams.h>
#include <thrift/lib/cpp2/async/processor/RequestTask.h>
#include <thrift/lib/cpp2/async/processor/ServerInterface.h>
#include <thrift/lib/cpp2/async/processor/ServerRequest.h>
#include <thrift/lib/cpp2/async/processor/ServerRequestHelper.h>
#include <thrift/lib/cpp2/async/processor/ServerRequestTask.h>
#include <thrift/lib/cpp2/async/processor/ServiceHandlerBase.h>
#include <thrift/lib/cpp2/protocol/Protocol.h>
#include <thrift/lib/cpp2/schema/SchemaV1.h>
#include <thrift/lib/cpp2/server/ConcurrencyControllerInterface.h>
#include <thrift/lib/cpp2/server/Cpp2ConnContext.h>
#include <thrift/lib/cpp2/server/IOWorkerContext.h>
#include <thrift/lib/cpp2/server/RequestPileInterface.h>
#include <thrift/lib/cpp2/server/ResourcePoolHandle.h>
#include <thrift/lib/cpp2/util/Checksum.h>
#include <thrift/lib/cpp2/util/IntrusiveSharedPtr.h>
#include <thrift/lib/cpp2/util/TypeErasedValue.h>
#include <thrift/lib/thrift/gen-cpp2/RpcMetadata_types.h>
#include <thrift/lib/thrift/gen-cpp2/metadata_types.h>
