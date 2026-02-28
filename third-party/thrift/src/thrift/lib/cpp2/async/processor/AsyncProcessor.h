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

#include <folly/io/async/EventBase.h>

#include <thrift/lib/cpp/TProcessor.h>
#include <thrift/lib/cpp2/async/AsyncProcessorFactory.h>
#include <thrift/lib/cpp2/async/ResponseChannel.h>
#include <thrift/lib/cpp2/server/Cpp2ConnContext.h>
#include <thrift/lib/cpp2/server/ServerConfigs.h>
#include <thrift/lib/thrift/gen-cpp2/metadata_types.h>

namespace apache::thrift {

/**
 * A class that is created once per-connection and handles incoming requests.
 * This is the hand-off point from Thrift's IO threads to user code - the
 * functions here are called on the IO thread.
 *
 * While this is a customization point, its API is not stable. Most services use
 * GeneratedAsyncProcessorBase, which handles scheduling of methods on to the
 * ThreadManager (tm) or executing inline (eb).
 */
class AsyncProcessor : public TProcessorBase {
 protected:
  using TProcessorBase::TProcessorBase;

 public:
  ~AsyncProcessor() override = default;

  // Return the name of the service provided by this AsyncProcessor
  virtual std::string_view getServiceName();

  using MethodMetadata = AsyncProcessorFactory::MethodMetadata;
  using WildcardMethodMetadata = AsyncProcessorFactory::WildcardMethodMetadata;

  /**
   * Processes one incoming request / method.
   *
   * @param methodMetadata See AsyncProcessorFactory::createMethodMetadata()
   */
  virtual void processSerializedCompressedRequestWithMetadata(
      ResponseChannelRequest::UniquePtr req,
      SerializedCompressedRequest&& serializedRequest,
      const MethodMetadata& methodMetadata,
      protocol::PROTOCOL_TYPES prot_type,
      Cpp2RequestContext* context,
      folly::EventBase* eb,
      concurrency::ThreadManager* tm) = 0;

  /**
   * Reflects on the current service's methods, associated structs etc. at
   * runtime. This is useful to, for example, a tool that can send requests to a
   * service without knowing its schemata ahead of time.
   *
   * This is analogous to GraphQL introspection
   * (https://graphql.org/learn/introspection/) and can be used to build a tool
   * like GraphiQL.
   */
  virtual void getServiceMetadata(metadata::ThriftServiceMetadataResponse&) {}

  virtual void terminateInteraction(
      int64_t id, Cpp2ConnContext& conn, folly::EventBase&) noexcept;
  virtual void destroyAllInteractions(
      Cpp2ConnContext& conn, folly::EventBase&) noexcept;

  virtual void processInteraction(ServerRequest&&) = 0;

  // This is the main interface we are migrating to. Eventually it should
  // replace all the processSerialized... methods.
  //
  // An AsyncProcessor implementation should call the handler for this request
  // on the thread it this method is called on. It does not need to call any
  // hooks for modules before executing the request.
  virtual void executeRequest(
      ServerRequest&& request,
      const AsyncProcessorFactory::MethodMetadata& methodMetadata);

  /**
   * TProcessorEventHandler instances can come from 3 places:
   *   1. ServerModules added to ThriftServer
   *   2. Globally-registered via
   * TProcessorBase::addProcessorEventHandler_deprecated
   *   3. AsyncProcessor::addEventHandler calls (typically in services that
   *      override getProcessor in their handler)
   *
   * This function combines all of these sources together such that
   * getEventHandlers() returns the united set, as if by calling
   * addEventHandler.
   *
   * This function is thread-safe. However, all calls MUST pass in the same
   * server object. This is reasonable because an AsyncProcessor cannot be used
   * across ThriftServer instances.
   */
  void coalesceWithServerScopedLegacyEventHandlers(
      const apache::thrift::server::ServerConfigs& server);
};

} // namespace apache::thrift
