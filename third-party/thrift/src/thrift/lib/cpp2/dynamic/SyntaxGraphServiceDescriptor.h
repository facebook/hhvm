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
#include <span>

#include <thrift/lib/cpp2/dynamic/ServiceDescriptor.h>
#include <thrift/lib/cpp2/schema/SyntaxGraph.h>

namespace apache::thrift::dynamic {

class SyntaxGraphServiceDescriptor final : public ServiceDescriptor {
 public:
  SyntaxGraphServiceDescriptor(
      std::shared_ptr<const syntax_graph::SyntaxGraph> syntaxGraph,
      const syntax_graph::ServiceNode& service);

  std::string_view serviceName() const override;
  std::span<const Function> functions() const override;
  std::span<const Interaction> interactions() const override;
  std::span<const DynamicValue> annotations() const override;

 private:
  const type_system::TypeSystem& typeSystem() const override;

  std::shared_ptr<const syntax_graph::SyntaxGraph> syntaxGraph_;
  std::string serviceName_;
  std::vector<Function> functions_;
  std::vector<Interaction> interactions_;
  std::vector<DynamicValue> annotations_;
};

} // namespace apache::thrift::dynamic
