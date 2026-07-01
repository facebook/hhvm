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

#include <memory>
#include <stdexcept>
#include <string_view>

#include <thrift/lib/cpp2/dynamic/ServiceCatalog.h>
#include <thrift/lib/cpp2/dynamic/ServiceDescriptor.h>
#include <thrift/lib/cpp2/schema/SyntaxGraph.h>

#include <gtest/gtest.h>
#include <thrift/lib/cpp2/dynamic/test/gen-cpp2/ServiceDescriptorTestService.h>

namespace apache::thrift::dynamic {
namespace {

std::shared_ptr<const syntax_graph::SyntaxGraph> buildGraph() {
  using Service =
      facebook::thrift::service_descriptor_test::ServiceDescriptorTestService;
  auto handler = std::make_shared<apache::thrift::ServiceHandler<Service>>();
  auto schema = handler->getServiceSchema();
  if (!schema.has_value()) {
    throw std::runtime_error("service has no embedded schema");
  }
  return std::make_shared<syntax_graph::SyntaxGraph>(
      syntax_graph::SyntaxGraph::fromSchema(
          apache::thrift::type::Schema(schema->schema)));
}

constexpr std::string_view kServiceName = "ServiceDescriptorTestService";

TEST(ServiceCatalogTest, IndexesServiceByUri) {
  auto graph = buildGraph();
  const ServiceCatalog& catalog = graph->asServiceCatalog();

  std::string_view foundUri;
  for (std::string_view uri : catalog.serviceUris()) {
    const ServiceDescriptor* svc = catalog.getService(uri);
    ASSERT_NE(svc, nullptr); // every enumerated URI must resolve
    if (svc->serviceName() == kServiceName) {
      foundUri = uri;
    }
  }

  ASSERT_FALSE(foundUri.empty());
  EXPECT_EQ(catalog.getServiceOrThrow(foundUri).serviceName(), kServiceName);
}

TEST(ServiceCatalogTest, LookupIsByUriNotName) {
  auto graph = buildGraph();
  const ServiceCatalog& catalog = graph->asServiceCatalog();
  // The unqualified service name is not a key; only the URI is.
  EXPECT_EQ(catalog.getService(kServiceName), nullptr);
}

TEST(ServiceCatalogTest, MissingUri) {
  auto graph = buildGraph();
  const ServiceCatalog& catalog = graph->asServiceCatalog();
  EXPECT_EQ(catalog.getService("meta.com/does/not/Exist"), nullptr);
  EXPECT_THROW(
      catalog.getServiceOrThrow("meta.com/does/not/Exist"), std::out_of_range);
}

} // namespace
} // namespace apache::thrift::dynamic
