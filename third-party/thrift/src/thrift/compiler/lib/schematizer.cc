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

#include <thrift/compiler/lib/schematizer.h>

namespace apache {
namespace thrift {
namespace compiler {
std::unique_ptr<t_const_value> schematizer::gen_schema(
    const t_structured& node) {
  auto schema = std::make_unique<t_const_value>();
  schema->set_map();

  // Definition
  auto definition = std::make_unique<t_const_value>();
  definition->set_map();
  definition->add_map(
      std::make_unique<t_const_value>("name"),
      std::make_unique<t_const_value>(node.name()));
  definition->add_map(
      std::make_unique<t_const_value>("uri"),
      std::make_unique<t_const_value>(node.uri()));
  // TODO: annotations
  schema->add_map(
      std::make_unique<t_const_value>("definition"), std::move(definition));

  // Fields
  auto fields = std::make_unique<t_const_value>();
  fields->set_list();

  // TODO(iahs): fill in fields

  schema->add_map(std::make_unique<t_const_value>("fields"), std::move(fields));

  return schema;
}
} // namespace compiler
} // namespace thrift
} // namespace apache
