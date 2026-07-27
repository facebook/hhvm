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

const ServiceDescriptor& findTestService(const ServiceCatalog& catalog) {
  if (const ServiceDescriptor* service =
          catalog.getServiceByName(kServiceName)) {
    return *service;
  }
  throw std::runtime_error("test service not found");
}

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
  ASSERT_NE(catalog.getServiceByName(kServiceName), nullptr);
  EXPECT_EQ(
      catalog.getServiceByName(kServiceName)->serviceName(), kServiceName);
}

TEST(ServiceCatalogTest, MissingUri) {
  auto graph = buildGraph();
  const ServiceCatalog& catalog = graph->asServiceCatalog();
  EXPECT_EQ(catalog.getService("meta.com/does/not/Exist"), nullptr);
  EXPECT_THROW(
      catalog.getServiceOrThrow("meta.com/does/not/Exist"), std::out_of_range);
}

TEST(ServiceCatalogTest, MissingName) {
  auto graph = buildGraph();
  const ServiceCatalog& catalog = graph->asServiceCatalog();
  EXPECT_EQ(catalog.getServiceByName("NotAService"), nullptr);
}

TEST(ServiceCatalogTest, FunctionExposesRpcEnvelopeShapes) {
  auto graph = buildGraph();
  const ServiceCatalog& catalog = graph->asServiceCatalog();
  const auto& add = findTestService(catalog).getFunctionByName("add");

  const auto request = add.requestEnvelope();
  ASSERT_EQ(request.fields.size(), 2);
  const auto& a = request.fields[0];
  EXPECT_EQ(a.identity().name(), "a");
  EXPECT_EQ(a.identity().id(), FieldId{1});
  EXPECT_TRUE(a.type().isI32());
  EXPECT_EQ(a.presence(), type_system::PresenceQualifier::UNQUALIFIED);
  const auto& b = request.fields[1];
  EXPECT_EQ(b.identity().name(), "b");
  EXPECT_EQ(b.identity().id(), FieldId{2});
  EXPECT_TRUE(b.type().isI32());
  EXPECT_EQ(b.presence(), type_system::PresenceQualifier::UNQUALIFIED);

  const auto response = add.responseEnvelope();
  ASSERT_EQ(response.fields.size(), 1);
  const auto& success = response.fields[0];
  EXPECT_EQ(success.identity().name(), "success");
  EXPECT_EQ(success.identity().id(), FieldId{0});
  EXPECT_TRUE(success.type().isI32());
  EXPECT_EQ(success.presence(), type_system::PresenceQualifier::OPTIONAL_);
}

TEST(ServiceCatalogTest, VoidFunctionExposesEmptyResponseEnvelope) {
  auto graph = buildGraph();
  const ServiceCatalog& catalog = graph->asServiceCatalog();
  const auto& ping = findTestService(catalog).getFunctionByName("ping");

  EXPECT_TRUE(ping.requestEnvelope().fields.empty());
  EXPECT_TRUE(ping.responseEnvelope().fields.empty());
}

} // namespace
} // namespace apache::thrift::dynamic
