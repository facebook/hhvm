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

namespace apache {
namespace thrift {
namespace compiler {
namespace {

class t_starlark_generator : public t_mstch_generator {
 public:
  using t_mstch_generator::t_mstch_generator;

  std::string template_prefix() const override { return "starlark"; }

  void generate_program() override;

 private:
  void set_mstch_factories();
};

class mstch_starlark_type : public mstch_type {
 public:
  mstch_starlark_type(
      const t_type* t, mstch_context& ctx, mstch_element_position pos)
      : mstch_type(t, ctx, pos) {
    register_methods(
        this,
        {
            {"type:starlark_supported?",
             &mstch_starlark_type::is_starlark_supported},
        });
  }

  mstch::node is_starlark_supported() { return is_supported_type(type_); }

 private:
  bool is_supported_type(const t_type* type) {
    return is_primitive(type->get_true_type()) ||
        is_supported_collection(type->get_true_type());
  }

  bool is_primitive(const t_type* type) {
    return type->is_byte() || type->is_i16() || type->is_i32() ||
        type->is_i64() || type->is_string() || type->is_bool() ||
        type->is_float() || type->is_double();
  }

  bool is_supported_collection(const t_type* type) {
    if (type->is_enum()) {
      return true;
    }
    if (type->is_list()) {
      return is_supported_type(
          dynamic_cast<const t_list*>(type)->get_elem_type());
    }
    if (type->is_map()) {
      const t_map* map = dynamic_cast<const t_map*>(type);
      return is_supported_type(map->get_key_type()) &&
          is_supported_type(map->get_val_type());
    }
    return false;
  }
};

void t_starlark_generator::generate_program() {
  out_dir_base_ = "gen-star";

  set_mstch_factories();
  const auto* program = get_program();
  auto mstch_program = mstch_context_.program_factory->make_mstch_object(
      program, mstch_context_);

  render_to_file(
      std::move(mstch_program), "definitions.star", program->name() + ".star");
}

void t_starlark_generator::set_mstch_factories() {
  mstch_context_.add<mstch_starlark_type>();
}

THRIFT_REGISTER_GENERATOR(starlark, "Starlark", "Starlark generator");

} // namespace
} // namespace compiler
} // namespace thrift
} // namespace apache
