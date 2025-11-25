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
#include <thrift/compiler/generate/t_whisker_generator.h>

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

class t_starlark_generator : public t_whisker_generator {
 public:
  using t_whisker_generator::t_whisker_generator;

  std::string template_prefix() const override { return "starlark"; }

  void generate_program() override;

 private:
  whisker::map::raw globals(whisker::prototype_database& proto) const override {
    whisker::map::raw globals = t_whisker_generator::globals(proto);
    // By default, Whisker considers f64s to be unprintable in strict mode, as
    // floats can have non-deterministic string results (e.g. fmt vs
    // std::ostream). For this reason, each generator should explicitly define
    // how its floats should be rendered.
    globals["float_to_string"] = whisker::dsl::make_function(
        "float_to_string",
        [](whisker::dsl::function::context ctx) -> whisker::object {
          ctx.declare_named_arguments({});
          ctx.declare_arity(1);
          return whisker::make::string(
              fmt::format("{}", ctx.argument<whisker::f64>(0)));
        });
    return globals;
  }

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

  render_to_file(
      /*output_file=*/get_program()->name() + ".star",
      /*template_file=*/"definitions.star",
      // `root_program` is a Whisker generator global
      /*context=*/whisker::make::map());
}

THRIFT_REGISTER_GENERATOR(starlark, "Starlark", "Starlark generator");

} // namespace
} // namespace apache::thrift::compiler
