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

#include <thrift/compiler/generate/t_mstch_generator.h>

#include <memory>
#include <optional>
#include <sstream>
#include <stdexcept>
#include <string>
#include <unordered_map>

#include <fmt/format.h>

#include <thrift/compiler/whisker/ast.h>
#include <thrift/compiler/whisker/mstch_compat.h>
#include <thrift/compiler/whisker/parser.h>
#include <thrift/compiler/whisker/source_location.h>
#include <thrift/compiler/whisker/standard_library.h>

using namespace std;

namespace fs = std::filesystem;

namespace apache::thrift::compiler {

namespace {} // namespace

mstch::map t_mstch_generator::dump(const t_program& program) {
  mstch::map result{
      {"name", program.name()},
      {"path", program.path()},
      {"includePrefix", program.include_prefix()},
      {"structs", dump_elems(program.structured_definitions())},
      {"enums", dump_elems(program.enums())},
      {"services", dump_elems(program.services())},
      {"typedefs", dump_elems(program.typedefs())},
      {"constants", dump_elems(program.consts())},
  };

  mstch::map extension = extend_program(program);
  extension.insert(result.begin(), result.end());
  return prepend_prefix("program", std::move(extension));
}

mstch::map t_mstch_generator::dump(const t_structured& strct, bool shallow) {
  mstch::map result{
      {"name", strct.name()},
      {"fields?", strct.has_fields()},
      {"fields",
       shallow ? static_cast<mstch::node>(false) : dump_elems(strct.fields())},
      {"exception?", strct.is<t_exception>()},
      {"union?", strct.is<t_union>()},
      {"plain?", !strct.is<t_exception>() && !strct.is<t_union>()},
      {"annotations", dump_elems(strct.unstructured_annotations())},
  };

  mstch::map extension = extend_struct(strct);
  extension.insert(result.begin(), result.end());
  return prepend_prefix("struct", std::move(extension));
}

mstch::map t_mstch_generator::dump(const t_field& field, int32_t index) {
  auto req = field.get_req();
  mstch::map result{
      {"name", field.name()},
      {"key", std::to_string(field.get_key())},
      {"type", dump(*field.get_type())},
      {"index", std::to_string(index)},
      {"index_plus_one", std::to_string(index + 1)},
      {"required?", req == t_field::e_req::required},
      {"optional?", req == t_field::e_req::optional},
      {"opt_in_req_out?", req == t_field::e_req::opt_in_req_out},
      {"terse?", req == t_field::e_req::terse},
      {"annotations", dump_elems(field.unstructured_annotations())},
  };

  if (field.get_value() != nullptr) {
    result.emplace("value", dump(*field.get_value()));
  }

  mstch::map extension = extend_field(field);
  extension.insert(result.begin(), result.end());
  return prepend_prefix("field", std::move(extension));
}

mstch::map t_mstch_generator::dump(const t_type& orig_type) {
  const t_type& type =
      should_resolve_typedefs() ? *orig_type.get_true_type() : orig_type;

  mstch::map result{
      {"name", type.name()},
      {"annotations", dump_elems(type.unstructured_annotations())},

      {"void?", type.is_void()},
      {"string?", type.is_string()},
      {"binary?", type.is_binary()},
      {"bool?", type.is_bool()},
      {"byte?", type.is_byte()},
      {"i16?", type.is_i16()},
      {"i32?", type.is_i32()},
      {"i64?", type.is_i64()},
      {"double?", type.is_double()},
      {"float?", type.is_float()},
      {"floating_point?", type.is_floating_point()},
      {"struct?", type.is<t_struct>()},
      {"union?", type.is<t_union>()},
      {"enum?", type.is<t_enum>()},
      {"base?", type.is<t_primitive_type>()},
      {"container?", type.is<t_container>()},
      {"list?", type.is<t_list>()},
      {"set?", type.is<t_set>()},
      {"map?", type.is<t_map>()},
      {"typedef?", type.is<t_typedef>()},
  };

  if (type.is<t_struct>()) {
    // Shallow dump the struct
    result.emplace("struct", dump(dynamic_cast<const t_struct&>(type), true));
  } else if (type.is<t_union>()) {
    // Shallow dump the union
    result.emplace("union", dump(dynamic_cast<const t_union&>(type), true));
  } else if (type.is<t_enum>()) {
    result.emplace("enum", dump(dynamic_cast<const t_enum&>(type)));
  } else if (type.is<t_list>()) {
    result.emplace(
        "list_elem_type",
        dump(*dynamic_cast<const t_list&>(type).get_elem_type()));
  } else if (type.is<t_set>()) {
    result.emplace(
        "set_elem_type",
        dump(*dynamic_cast<const t_set&>(type).get_elem_type()));
  } else if (type.is<t_map>()) {
    result.emplace(
        "key_type", dump(*dynamic_cast<const t_map&>(type).get_key_type()));
    result.emplace(
        "value_type", dump(*dynamic_cast<const t_map&>(type).get_val_type()));
  } else if (type.is<t_typedef>()) {
    result.emplace(
        "typedef_type", dump(*dynamic_cast<const t_typedef&>(type).get_type()));
  }

  mstch::map extension = extend_type(type);
  extension.insert(result.begin(), result.end());
  return prepend_prefix("type", std::move(extension));
}

mstch::map t_mstch_generator::dump(const t_enum& enm) {
  mstch::map result{
      {"name", enm.name()},
      {"values", dump_elems(enm.get_enum_values())},
      {"annotations", dump_elems(enm.unstructured_annotations())},
  };

  mstch::map extension = extend_enum(enm);
  extension.insert(result.begin(), result.end());
  return prepend_prefix("enum", std::move(extension));
}

mstch::map t_mstch_generator::dump(const t_enum_value& val) {
  mstch::map result{
      {"name", val.name()},
      {"value", std::to_string(val.get_value())},
  };

  mstch::map extension = extend_enum_value(val);
  extension.insert(result.begin(), result.end());
  return prepend_prefix("enum_value", std::move(extension));
}

mstch::map t_mstch_generator::dump(const t_service& service) {
  const t_service* extends = service.get_extends();
  mstch::map result{
      {"name", service.name()},
      {"annotations", dump_elems(service.unstructured_annotations())},
      {"functions", dump_elems(service.get_functions())},
      {"functions?", !service.get_functions().empty()},
      {"extends?", extends != nullptr},
      {"extends", extends ? static_cast<mstch::node>(dump(*extends)) : false},
  };

  mstch::map extension = extend_service(service);
  extension.insert(result.begin(), result.end());
  return prepend_prefix("service", std::move(extension));
}

mstch::map t_mstch_generator::dump(const t_function& function) {
  auto exceptions = get_elems(function.exceptions());
  mstch::map result{
      {"name", function.name()},
      {"oneway?", function.qualifier() == t_function_qualifier::oneway},
      {"return_type", dump(*function.return_type())},
      {"exceptions", dump_elems(exceptions)},
      {"exceptions?", !exceptions.empty()},
      {"annotations", dump_elems(function.unstructured_annotations())},
      {"args", dump_elems(function.params().fields())},
  };

  mstch::map extension = extend_function(function);
  extension.insert(result.begin(), result.end());
  return prepend_prefix("function", std::move(extension));
}

mstch::map t_mstch_generator::dump(const t_const& cnst) {
  mstch::map result{
      {"type", dump(*cnst.type())},
      {"name", cnst.name()},
      {"value", dump(*cnst.value())},
  };

  mstch::map extension = extend_const(cnst);
  extension.insert(result.begin(), result.end());
  return prepend_prefix("constant", std::move(extension));
}

mstch::map t_mstch_generator::dump(const t_const_value& value) {
  using cv = t_const_value::t_const_value_kind;
  const cv type = value.kind();
  mstch::map result{
      {"bool?", type == cv::CV_BOOL},
      {"double?", type == cv::CV_DOUBLE},
      {"integer?", type == cv::CV_INTEGER && !value.get_enum_value()},
      {"enum?", type == cv::CV_INTEGER && value.get_enum_value()},
      {"string?", type == cv::CV_STRING},
      {"base?",
       type == cv::CV_BOOL || type == cv::CV_DOUBLE || type == cv::CV_INTEGER ||
           type == cv::CV_STRING},
      {"map?", type == cv::CV_MAP},
      {"list?", type == cv::CV_LIST},
      {"container?", type == cv::CV_MAP || type == cv::CV_LIST},
  };

  switch (type) {
    case cv::CV_DOUBLE:
      result.emplace("value", value.get_double());
      result.emplace("double_value", value.get_double());
      result.emplace("nonzero?", value.get_double() != 0.0);
      break;
    case cv::CV_BOOL:
      result.emplace("value", value.get_bool() ? 1 : 0);
      result.emplace("bool_value", value.get_bool());
      result.emplace("nonzero?", value.get_bool());
      break;
    case cv::CV_INTEGER:
      if (value.get_enum_value()) {
        result.emplace("enum_name", value.get_enum()->name());
        result.emplace("enum_value_name", value.get_enum_value()->name());
      }
      result.emplace("value", std::to_string(value.get_integer()));
      result.emplace("integer_value", std::to_string(value.get_integer()));
      result.emplace("nonzero?", value.get_integer() != 0);
      break;
    case cv::CV_STRING:
      result.emplace("value", value.get_string());
      result.emplace("string_value", value.get_string());
      break;
    case cv::CV_MAP:
      result.emplace("map_elements", dump_elems(value.get_map()));
      break;
    case cv::CV_LIST:
      result.emplace("list_elements", dump_elems(value.get_list()));
      break;
    default:
      std::ostringstream err;
      err << "Unhandled t_const_value_kind " << value.kind();
      throw std::domain_error{err.str()};
  }

  mstch::map extension = extend_const_value(value);
  extension.insert(result.begin(), result.end());
  return prepend_prefix("value", std::move(extension));
}

mstch::map t_mstch_generator::dump(
    const std::map<t_const_value*, t_const_value*>::value_type& pair) {
  mstch::map result{
      {"key", dump(*pair.first)},
      {"value", dump(*pair.second)},
  };
  mstch::map extension = extend_const_value_map_elem(pair);
  extension.insert(result.begin(), result.end());
  return prepend_prefix("element", std::move(extension));
}

mstch::map t_mstch_generator::dump(const annotation& pair) {
  mstch::map result{
      {"key", pair.first},
      {"value", pair.second.value},
  };
  mstch::map extension = extend_annotation(pair);
  extension.insert(result.begin(), result.end());
  return prepend_prefix("annotation", std::move(extension));
}

mstch::map t_mstch_generator::dump(const t_typedef& typdef) {
  mstch::map result{
      {"type", dump(*typdef.get_type())},
      {"name", typdef.name()},
  };

  mstch::map extension = extend_typedef(typdef);
  extension.insert(result.begin(), result.end());
  return prepend_prefix("typedef", std::move(extension));
}

mstch::map t_mstch_generator::dump(const string& value) {
  mstch::map result{{"value", value}};
  return result;
}

mstch::map t_mstch_generator::dump_options() {
  mstch::map result;
  for (auto& elem : compiler_options()) {
    result.emplace(elem.first, elem.second);
  }
  return prepend_prefix("option", std::move(result));
}

// Extenders, by default do no extending

mstch::map t_mstch_generator::extend_program(const t_program&) {
  return {};
}

mstch::map t_mstch_generator::extend_struct(const t_structured&) {
  return {};
}

mstch::map t_mstch_generator::extend_field(const t_field&) {
  return {};
}

mstch::map t_mstch_generator::extend_type(const t_type&) {
  return {};
}

mstch::map t_mstch_generator::extend_enum(const t_enum&) {
  return {};
}

mstch::map t_mstch_generator::extend_enum_value(const t_enum_value&) {
  return {};
}

mstch::map t_mstch_generator::extend_service(const t_service&) {
  return {};
}

mstch::map t_mstch_generator::extend_function(const t_function&) {
  return {};
}

mstch::map t_mstch_generator::extend_typedef(const t_typedef&) {
  return {};
}

mstch::map t_mstch_generator::extend_const(const t_const&) {
  return {};
}

mstch::map t_mstch_generator::extend_const_value(const t_const_value&) {
  return {};
}

mstch::map t_mstch_generator::extend_const_value_map_elem(
    const std::map<t_const_value*, t_const_value*>::value_type&) {
  return {};
}

mstch::map t_mstch_generator::extend_annotation(const annotation&) {
  return {};
}

bool t_mstch_generator::has_option(const std::string& option) const {
  return has_compiler_option(option);
}

std::optional<std::string> t_mstch_generator::get_option(
    const std::string& option) const {
  if (std::optional<std::string_view> found = get_compiler_option(option)) {
    return std::string{*found};
  }
  return std::nullopt;
}

mstch::map t_mstch_generator::prepend_prefix(
    const std::string& prefix, mstch::map map) {
  mstch::map res;
  for (auto& pair : map) {
    res.emplace(prefix + ":" + pair.first, std::move(pair.second));
  }
  return res;
}

whisker::map::raw t_mstch_generator::globals() const {
  auto options = render_options();
  whisker::map::raw result = t_whisker_generator::globals();
  for (const auto& undefined_name : options.allowed_undefined_variables) {
    result.insert({undefined_name, whisker::make::null});
  }
  return result;
}

t_mstch_generator::strictness_options t_mstch_generator::strictness() const {
  strictness_options strict;
  // Our legacy code has a ton of non-boolean conditionals
  strict.boolean_conditional = false;
  // Our legacy code relies on printing null as empty string
  strict.printable_types = false;
  // Undefined variables are covered by globals(...)
  strict.undefined_variables = true;
  return strict;
}

std::string t_mstch_generator::render(
    const std::string& template_name, const mstch::node& context) {
  return render(template_name, whisker::from_mstch(context));
}

void t_mstch_generator::render_to_file(
    const mstch::map& context,
    const std::string& template_name,
    const std::filesystem::path& path) {
  write_to_file(path, render(template_name, context));
}

const std::shared_ptr<mstch_base>& t_mstch_generator::cached_program(
    const t_program* program) {
  const auto& id = program->path();
  auto itr = mstch_context_.program_cache.find(id);
  if (itr == mstch_context_.program_cache.end()) {
    itr = mstch_context_.program_cache
              .emplace(
                  id,
                  mstch_context_.program_factory->make_mstch_object(
                      program, mstch_context_))
              .first;
  }
  return itr->second;
}

} // namespace apache::thrift::compiler
