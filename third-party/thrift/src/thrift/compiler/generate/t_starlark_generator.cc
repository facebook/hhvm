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

#include <utility>

#include <thrift/compiler/generate/mstch_objects.h>
#include <thrift/compiler/generate/t_mstch_generator.h>

namespace apache::thrift::compiler {
namespace {

bool is_supported_type(const t_type& type);

bool is_supported_primitive(const t_type& type) {
  const t_primitive_type* primitive = type.try_as<t_primitive_type>();
  return primitive != nullptr &&
      primitive->primitive_type() != t_primitive_type::type::t_void &&
      primitive->primitive_type() != t_primitive_type::type::t_binary;
}

bool is_supported_collection(const t_type& type) {
  if (type.is<t_enum>()) {
    return true;
  }
  if (const t_list* list = type.try_as<t_list>()) {
    return is_supported_type(*list->elem_type());
  }
  if (const t_map* map = type.try_as<t_map>()) {
    return is_supported_type(*map->key_type()) &&
        is_supported_type(*map->val_type());
  }
  return false;
}

bool is_supported_type(const t_type& type) {
  const t_type& true_type = *type.get_true_type();
  return is_supported_primitive(true_type) ||
      is_supported_collection(true_type);
}

class t_starlark_generator : public t_mstch_generator {
 public:
  using t_mstch_generator::t_mstch_generator;

  std::string template_prefix() const override { return "starlark"; }

  void generate_program() override;

 private:
  prototype<t_type>::ptr make_prototype_for_type(
      const prototype_database& proto) const override {
    auto base = t_whisker_generator::make_prototype_for_type(proto);
    auto def = whisker::dsl::prototype_builder<h_type>::extends(base);

    def.property("starlark_supported?", &is_supported_type);

    return std::move(def).make();
  }
};

void t_starlark_generator::generate_program() {
  out_dir_base_ = "gen-star";

  const auto* program = get_program();
  auto mstch_program = mstch_context_.program_factory->make_mstch_object(
      program, mstch_context_);

  render_to_file(
      std::move(mstch_program), "definitions.star", program->name() + ".star");
}

THRIFT_REGISTER_GENERATOR(starlark, "Starlark", "Starlark generator");

} // namespace
} // namespace apache::thrift::compiler
