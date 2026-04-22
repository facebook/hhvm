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

#include <thrift/lib/cpp2/dynamic/DynamicServiceSchema.h>
#include <thrift/lib/cpp2/schema/SyntaxGraph.h>

namespace apache::thrift::dynamic {

/**
 * A DynamicServiceSchema backed by a SyntaxGraph.
 *
 * String_views and TypeRefs reference the SyntaxGraph's internal storage,
 * which is kept alive by shared ownership.
 */
class SyntaxGraphServiceSchema final : public DynamicServiceSchema {
 public:
  /**
   * Construct from a ServiceNode within a SyntaxGraph.
   * Includes inherited functions from base services.
   */
  SyntaxGraphServiceSchema(
      std::shared_ptr<const syntax_graph::SyntaxGraph> syntaxGraph,
      const syntax_graph::ServiceNode& service);

  std::string_view serviceName() const override;
  folly::span<const Function> functions() const override;
  std::shared_ptr<const type_system::TypeSystem> getTypeSystem() const override;

 private:
  std::shared_ptr<const syntax_graph::SyntaxGraph> syntaxGraph_;
  std::string serviceName_;
  std::vector<Function> functions_;
};

} // namespace apache::thrift::dynamic
