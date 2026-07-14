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

#include <memory>
#include <optional>
#include <vector>

#include <folly/io/async/DelayedDestruction.h>
#include <folly/memory/not_null.h>

#include <thrift/lib/cpp2/fast_thrift/thrift/server/adapter/ThriftServerAppAdapter.h>
#include <thrift/lib/cpp2/schema/SchemaV1.h>
#include <thrift/lib/thrift/gen-cpp2/metadata_types.h>

namespace apache::thrift::syntax_graph {
class ServiceNode;
} // namespace apache::thrift::syntax_graph

namespace apache::thrift::fast_thrift::thrift {

/**
 * Polymorphic base for every generated FastServiceHandler<Service>. Provides
 * a single virtual hook that FastThriftServer calls per accepted connection
 * to obtain a fresh per-connection app adapter. The override knows the
 * concrete <Service>AppAdapter type and constructs it; the server sees only
 * the type-erased base ThriftServerAppAdapter and is responsible for placing
 * it in a pipeline.
 *
 * User code never names the ThriftServerAppAdapterFactory or the adapter —
 * they pass shared_ptr<MyHandler> to FastThriftServer::setInterface and the
 * implicit upcast to ThriftServerAppAdapterFactory happens transparently.
 */
class ThriftServerAppAdapterFactory {
 public:
  virtual ~ThriftServerAppAdapterFactory() = default;

  // Construct a per-connection adapter. The override receives `self` (the
  // server's shared handler ptr) so it can hand a typed shared_ptr to the
  // generated <Service>AppAdapter ctor without enable_shared_from_this.
  virtual ThriftServerAppAdapter::Ptr getAppAdapter(
      std::shared_ptr<ThriftServerAppAdapterFactory> self) = 0;

  // Populate `response` with the static metadata for the service this
  // factory serves. Generated ServiceFastHandler<S> overrides this to call
  // detail::md::ServiceMetadata<S>::gen(response). FastThriftServer invokes
  // it once at start() when enableMetadataService is set; the cached
  // response is then served on every getThriftServiceMetadata() RPC.
  // Default no-op so non-generated factories don't need to participate.
  virtual void getServiceMetadata(
      apache::thrift::metadata::ThriftServiceMetadataResponse& /*response*/) {}

  // Reflect the service's runtime schema. Generated FastServiceHandler<S>
  // overrides these (only when the schema is statically bundled) to return the
  // same data as the legacy ServiceHandler<S>, keyed on the service tag.
  // Consumed at start-serving by schema-driven interceptors (e.g.
  // authorization). Default empty so non-generated factories need not
  // participate.
  virtual std::optional<::apache::thrift::schema::DefinitionsSchema>
  getServiceSchema() {
    return {};
  }

  virtual std::vector<
      folly::not_null<const ::apache::thrift::syntax_graph::ServiceNode*>>
  getServiceSchemaNodes() {
    return {};
  }
};

} // namespace apache::thrift::fast_thrift::thrift
