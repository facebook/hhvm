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

#include <thrift/compiler/generate/common.h>
#include <thrift/compiler/generate/mstch_objects.h>

#include <fmt/core.h>

namespace apache::thrift::compiler {

std::shared_ptr<mstch_base> make_mstch_program_cached(
    const t_program* program, mstch_context& ctx, mstch_element_position pos) {
  const auto& id = program->path();
  auto itr = ctx.program_cache.find(id);
  if (itr == ctx.program_cache.end()) {
    itr = ctx.program_cache.emplace_hint(
        itr, id, ctx.program_factory->make_mstch_object(program, ctx, pos));
  }
  return itr->second;
}

std::shared_ptr<mstch_base> make_mstch_service_cached(
    const t_program* program,
    const t_service* service,
    mstch_context& ctx,
    mstch_element_position pos) {
  std::string service_id = program->path() + service->name();
  auto itr = ctx.service_cache.find(service_id);
  if (itr == ctx.service_cache.end()) {
    itr = ctx.service_cache.emplace_hint(
        itr,
        service_id,
        ctx.service_factory->make_mstch_object(service, ctx, pos));
  }
  return itr->second;
}

bool mstch_base::has_option(const std::string& option) const {
  return context_.options.find(option) != context_.options.end();
}

std::string mstch_base::get_option(const std::string& option) const {
  auto itr = context_.options.find(option);
  if (itr != context_.options.end()) {
    return itr->second;
  }
  return {};
}

mstch_factories::mstch_factories() {
  add<mstch_program>();
  add<mstch_type>();
  add<mstch_typedef>();
  add<mstch_struct>();
  add<mstch_field>();
  add<mstch_enum>();
  add<mstch_enum_value>();
  add<mstch_const>();
  add<mstch_const_value>();
  add<mstch_const_map_element>();
  add<mstch_structured_annotation>();
}

mstch::node mstch_enum::values() {
  return make_mstch_enum_values(enum_->get_enum_values());
}

mstch::node mstch_type::get_structured() {
  if (resolved_type_->is<t_structured>()) {
    std::string id = program_cache_id(
        type_->program(), get_type_namespace(type_->program()));
    return make_mstch_array_cached(
        std::vector<const t_structured*>{
            dynamic_cast<const t_structured*>(resolved_type_)},
        *context_.struct_factory,
        context_.struct_cache,
        id);
  }
  return mstch::node();
}

mstch::node mstch_type::get_enum() {
  if (resolved_type_->is<t_enum>()) {
    std::string id = program_cache_id(
        type_->program(), get_type_namespace(type_->program()));
    return make_mstch_array_cached(
        std::vector<const t_enum*>{dynamic_cast<const t_enum*>(resolved_type_)},
        *context_.enum_factory,
        context_.enum_cache,
        id);
  }
  return mstch::node();
}

mstch::node mstch_type::get_list_type() {
  if (resolved_type_->is<t_list>()) {
    return context_.type_factory->make_mstch_object(
        dynamic_cast<const t_list*>(resolved_type_)->get_elem_type(),
        context_,
        pos_);
  }
  return mstch::node();
}

mstch::node mstch_type::get_set_type() {
  if (resolved_type_->is<t_set>()) {
    return context_.type_factory->make_mstch_object(
        dynamic_cast<const t_set*>(resolved_type_)->get_elem_type(),
        context_,
        pos_);
  }
  return mstch::node();
}

mstch::node mstch_type::get_key_type() {
  if (resolved_type_->is<t_map>()) {
    return context_.type_factory->make_mstch_object(
        &resolved_type_->as<t_map>().key_type().deref(), context_, pos_);
  }
  return mstch::node();
}

mstch::node mstch_type::get_value_type() {
  if (resolved_type_->is<t_map>()) {
    return context_.type_factory->make_mstch_object(
        &resolved_type_->as<t_map>().val_type().deref(), context_, pos_);
  }
  return mstch::node();
}

mstch::node mstch_type::get_typedef_type() {
  if (type_->is<t_typedef>()) {
    return context_.type_factory->make_mstch_object(
        dynamic_cast<const t_typedef*>(type_)->get_type(), context_, pos_);
  }
  return mstch::node();
}

mstch::node mstch_type::get_typedef() {
  if (type_->is<t_typedef>()) {
    return context_.typedef_factory->make_mstch_object(
        dynamic_cast<const t_typedef*>(type_), context_, pos_);
  }
  return mstch::node();
}

mstch::node mstch_field::value() {
  if (!field_->get_value()) {
    return mstch::node();
  }
  return context_.const_value_factory->make_mstch_object(
      field_->get_value(), context_, pos_, nullptr, nullptr);
}

mstch::node mstch_const_map_element::element_key() {
  return context_.const_value_factory->make_mstch_object(
      element_.first, context_, pos_, current_const_, expected_types_.first);
}

mstch::node mstch_const_map_element::element_value() {
  return context_.const_value_factory->make_mstch_object(
      element_.second, context_, pos_, current_const_, expected_types_.second);
}

mstch::node mstch_const_value::enum_value() {
  if (const_value_->get_enum_value() != nullptr) {
    return context_.enum_value_factory->make_mstch_object(
        const_value_->get_enum_value(), context_, pos_);
  }
  return mstch::node();
}

mstch::node mstch_const_value::list_elems() {
  if (type_ == cv::CV_LIST) {
    const t_type* expected_type = nullptr;
    if (expected_type_) {
      if (expected_type_->is<t_list>()) {
        expected_type =
            dynamic_cast<const t_list*>(expected_type_)->get_elem_type();
      } else if (expected_type_->is<t_set>()) {
        expected_type =
            dynamic_cast<const t_set*>(expected_type_)->get_elem_type();
      }
    }
    return make_mstch_consts(
        const_value_->get_list(), current_const_, expected_type);
  }
  return mstch::node();
}

mstch::node mstch_const_value::map_elems() {
  if (type_ != cv::CV_MAP) {
    return mstch::node();
  }
  std::pair<const t_type*, const t_type*> expected_types;
  if (expected_type_ && expected_type_->is<t_map>()) {
    const auto* m = dynamic_cast<const t_map*>(expected_type_);
    expected_types = {&m->key_type().deref(), &m->val_type().deref()};
  }
  return make_mstch_array(
      const_value_->get_map(),
      *context_.const_map_element_factory,
      current_const_,
      expected_types);
}

mstch::node mstch_const_value::const_struct_type() {
  if (!const_value_->ttype()) {
    return {};
  }

  const auto* type = const_value_->ttype()->get_true_type();
  if (type->is<t_structured>()) {
    return context_.type_factory->make_mstch_object(type, context_);
  }

  return {};
}

mstch::node mstch_const_value::const_struct() {
  std::vector<t_const*> constants;
  std::vector<const t_field*> fields;
  mstch::array a;

  const auto* type = const_value_->ttype()->get_true_type();
  if (type->is<t_structured>()) {
    const auto* strct = dynamic_cast<const t_structured*>(type);
    for (auto member : const_value_->get_map()) {
      const auto* field = strct->get_field_by_name(member.first->get_string());
      assert(field != nullptr);
      constants.push_back(new t_const(
          nullptr,
          t_type_ref::from_req_ptr(field->get_type()),
          field->name(),
          member.second->clone()));
      fields.push_back(field);
    }
  }

  for (size_t i = 0, size = constants.size(); i < size; ++i) {
    a.push_back(context_.const_factory->make_mstch_object(
        constants[i],
        context_,
        mstch_element_position(i, size),
        current_const_,
        constants[i]->type(),
        fields[i]));
  }
  return a;
}

mstch::node mstch_const_value::owning_const() {
  return context_.const_factory->make_mstch_object(
      const_value_->get_owner(), context_, pos_, nullptr, nullptr, nullptr);
}

mstch::node mstch_field::type() {
  return context_.type_factory->make_mstch_object(
      field_->get_type(), context_, pos_);
}

mstch::node mstch_struct::fields() {
  return make_mstch_fields(struct_->get_members());
}

mstch::node mstch_function::return_type() {
  const t_type* type = function_->return_type().get_type();
  // Override the return type for compatibility with old codegen.
  if (function_->is_interaction_constructor()) {
    // The old syntax (performs) treats an interaction as a response.
    type = function_->interaction().get_type();
  } else if (function_->sink_or_stream()) {
    type = &t_primitive_type::t_void();
  }
  return context_.type_factory->make_mstch_object(type, context_, pos_);
}

mstch::node mstch_function::arg_list() {
  return make_mstch_fields(function_->params().get_members());
}

mstch::node mstch_function::exceptions() {
  const t_throws* throws = function_->exceptions();
  return throws ? make_mstch_fields(throws->get_members()) : mstch::node();
}

mstch::node mstch_function::sink_first_response_type() {
  const t_sink* sink = function_->sink();
  if (!sink || function_->has_void_initial_response()) {
    return {};
  }
  return context_.type_factory->make_mstch_object(
      function_->return_type().get_type(), context_, pos_);
}

mstch::node mstch_function::sink_elem_type() {
  const t_sink* sink = function_->sink();
  return sink ? context_.type_factory->make_mstch_object(
                    sink->get_elem_type(), context_, pos_)
              : mstch::node();
}

mstch::node mstch_function::sink_exceptions() {
  const t_sink* sink = function_->sink();
  return sink ? make_exceptions(sink->sink_exceptions()) : mstch::node();
}

mstch::node mstch_function::sink_final_reponse_type() {
  const t_sink* sink = function_->sink();
  return sink && sink->get_final_response_type()
      ? context_.type_factory->make_mstch_object(
            sink->get_final_response_type(), context_, pos_)
      : mstch::node();
}

mstch::node mstch_function::sink_final_response_exceptions() {
  const t_sink* sink = function_->sink();
  return sink ? make_exceptions(sink->final_response_exceptions())
              : mstch::node();
}

mstch::node mstch_function::stream_elem_type() {
  const t_stream* stream = function_->stream();
  return stream ? context_.type_factory->make_mstch_object(
                      stream->elem_type().get_type(), context_, pos_)
                : mstch::node();
}

mstch::node mstch_function::stream_first_response_type() {
  const t_stream* stream = function_->stream();
  if (!stream || function_->has_void_initial_response()) {
    return {};
  }
  return context_.type_factory->make_mstch_object(
      function_->return_type().get_type(), context_, pos_);
}

mstch::node mstch_function::stream_exceptions() {
  const t_stream* stream = function_->stream();
  return stream ? make_exceptions(stream->exceptions()) : mstch::node();
}

mstch::node mstch_service::functions() {
  return make_mstch_functions(get_functions());
}

mstch::node mstch_service::extends() {
  const auto* extends = service_->extends();
  if (extends) {
    return make_mstch_extended_service_cached(extends);
  }
  return mstch::node();
}

mstch::node mstch_service::make_mstch_extended_service_cached(
    const t_service* service) {
  std::string id = program_cache_id(
      service->program(), get_service_namespace(service->program()));
  return make_mstch_element_cached(
      service, *context_.service_factory, context_.service_cache, id, 0, 1);
}

mstch::node mstch_typedef::type() {
  return context_.type_factory->make_mstch_object(
      typedef_->get_type(), context_, pos_);
}

mstch::node mstch_const::type() {
  return context_.type_factory->make_mstch_object(
      const_->type(), context_, pos_);
}

mstch::node mstch_const::value() {
  return context_.const_value_factory->make_mstch_object(
      const_->value(), context_, pos_, const_, expected_type_);
}

mstch::node mstch_const::program() {
  return context_.program_factory->make_mstch_object(
      const_->program(), context_, pos_);
}

mstch::node mstch_const::field() {
  return context_.field_factory->make_mstch_object(field_, context_, pos_);
}

mstch::node mstch_program::structs() {
  std::string id = program_cache_id(program_, get_program_namespace(program_));
  return make_mstch_array_cached(
      program_->structured_definitions(),
      *context_.struct_factory,
      context_.struct_cache,
      id);
}

mstch::node mstch_program::enums() {
  std::string id = program_cache_id(program_, get_program_namespace(program_));
  return make_mstch_array_cached(
      program_->enums(), *context_.enum_factory, context_.enum_cache, id);
}

mstch::node mstch_program::services() {
  std::string id = program_cache_id(program_, get_program_namespace(program_));
  return make_mstch_array_cached(
      program_->services(),
      *context_.service_factory,
      context_.service_cache,
      id);
}

mstch::node mstch_program::interactions() {
  return make_mstch_interactions(program_->interactions(), nullptr);
}

mstch::node mstch_program::typedefs() {
  return make_mstch_typedefs(program_->typedefs());
}

mstch::node mstch_program::constants() {
  mstch::array a;
  const auto& container = program_->consts();
  for (size_t i = 0, size = container.size(); i < size; ++i) {
    a.push_back(context_.const_factory->make_mstch_object(
        container[i],
        context_,
        mstch_element_position(i, size),
        container[i],
        container[i]->type(),
        nullptr));
  }
  return a;
}

mstch_context& mstch_context::set_or_erase_option(
    bool condition, const std::string& key, const std::string& value) {
  if (condition) {
    options[key] = value;
  } else {
    options.erase(key);
  }
  return *this;
}

} // namespace apache::thrift::compiler
