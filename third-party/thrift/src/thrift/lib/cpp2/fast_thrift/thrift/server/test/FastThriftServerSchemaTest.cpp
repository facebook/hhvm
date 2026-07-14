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

/**
 * Verifies that a @cpp.FastServer service with schema bundling
 * (with_schema=True) surfaces its runtime service schema through the generated
 * FastServiceHandler<S> — the data path SAP's authorization interceptor reads
 * at start-serving. Uses CompositeE2EPrimaryService, which is fully fast
 * (@cpp.FastServer + @cpp.FastClient) and therefore emits neither Client<S>
 * nor ServiceHandler<S>; this is exactly the shape that must still resolve the
 * ServiceNode.
 */

#include <gtest/gtest.h>

#include <thrift/lib/cpp2/fast_thrift/thrift/test/if/gen-cpp2/CompositeE2EPrimaryService.h>
#include <thrift/lib/cpp2/runtime/SchemaRegistry.h>
#include <thrift/lib/cpp2/schema/SyntaxGraph.h>

namespace apache::thrift::fast_thrift::thrift::test::composite_e2e {

using ::apache::thrift::FastServiceHandler;
using ::apache::thrift::SchemaRegistry;
using ::apache::thrift::syntax_graph::ServiceNode;

TEST(FastThriftServerSchemaTest, ExposesServiceSchemaNodes) {
  FastServiceHandler<CompositeE2EPrimaryService> handler;

  auto nodes = handler.getServiceSchemaNodes();
  ASSERT_EQ(nodes.size(), 1);

  // Independent oracle: the global registry resolves the service tag to a
  // definition named after the IDL service. The handler must return that same
  // ServiceNode.
  const auto& definition =
      SchemaRegistry::get().getDefinitionNode<CompositeE2EPrimaryService>();
  EXPECT_EQ(definition.name(), "CompositeE2EPrimaryService");
  EXPECT_EQ(&*nodes[0], &definition.as<ServiceNode>());
}

TEST(FastThriftServerSchemaTest, ExposesServiceSchema) {
  FastServiceHandler<CompositeE2EPrimaryService> handler;

  auto schema = handler.getServiceSchema();
  ASSERT_TRUE(schema.has_value());
  EXPECT_FALSE(schema->definitions.empty());
}

} // namespace apache::thrift::fast_thrift::thrift::test::composite_e2e
