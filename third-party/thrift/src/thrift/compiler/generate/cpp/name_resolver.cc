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

#include <thrift/compiler/generate/cpp/name_resolver.h>

#include <stdexcept>
#include <typeinfo>

#include <fmt/ranges.h>
#include <thrift/compiler/ast/t_function.h>
#include <thrift/compiler/ast/t_list.h>
#include <thrift/compiler/ast/t_map.h>
#include <thrift/compiler/ast/t_node.h>
#include <thrift/compiler/ast/t_program.h>
#include <thrift/compiler/ast/t_set.h>
#include <thrift/compiler/ast/t_struct.h>
#include <thrift/compiler/ast/t_typedef.h>
#include <thrift/compiler/ast/uri.h>

namespace apache::thrift::compiler {
namespace {

const t_type* find_first_type(const t_type& node) {
  const t_type* result = nullptr;
  t_typedef::find_type_if(&node, [&result](const t_type* type) {
    if (cpp_name_resolver::find_type(*type)) {
      result = type;
      return true;
    }
    return false;
  });
  return result;
}

} // namespace

namespace detail {

std::string gen_template_type(
    std::string template_name, std::initializer_list<std::string> args) {
  template_name += "<";
  auto delim = "";
  for (const auto& arg : args) {
    template_name += delim;
    delim = ", ";
    template_name += arg;
  }
  template_name += ">";
  return template_name;
}

} // namespace detail

const std::string& cpp_name_resolver::get_native_type(
    const t_field& field, const t_structured& parent) {
  const t_type& type = *field.type();

  // Handle @cpp.Adapter on field.
  if (auto* annotation =
          field.find_structured_annotation_or_null(kCppAdapterUri)) {
    if (auto* adapted_type =
            annotation->get_value_from_structured_annotation_or_null(
                "adaptedType")) {
      return adapted_type->get_string();
    }
    const auto& adapter_on_field =
        annotation->get_value_from_structured_annotation("name").get_string();
    return detail::get_or_gen(field_type_cache_, &field, [&]() {
      return gen_field_type(field.id(), type, parent, &adapter_on_field);
    });
  }

  // Handle @cpp.Type on field.
  if (auto* annotation =
          field.find_structured_annotation_or_null(kCppTypeUri)) {
    if (auto name =
            annotation->get_value_from_structured_annotation_or_null("name")) {
      return detail::get_or_gen(
          field_type_cache_, &field, [&]() { return name->get_string(); });
    } else {
      auto& tmplate =
          annotation->get_value_from_structured_annotation("template");
      return detail::get_or_gen(field_type_cache_, &field, [&]() {
        return gen_container_type(
            type.get_true_type()->as<t_container>(),
            &cpp_name_resolver::get_native_type,
            &tmplate.get_string());
      });
    }
  }

  return get_native_type(type);
}

const std::string& cpp_name_resolver::get_native_type(const t_const& cnst) {
  const t_type& type = *cnst.type();

  if (auto* annotation =
          cnst.find_structured_annotation_or_null(kCppAdapterUri)) {
    if (auto* adapted_type =
            annotation->get_value_from_structured_annotation_or_null(
                "adaptedType")) {
      return adapted_type->get_string();
    }
    return detail::get_or_gen(const_cache_, &cnst, [&]() {
      const auto& adapter =
          annotation->get_value_from_structured_annotation("name").get_string();
      return gen_adapted_type(&adapter, gen_type(type));
    });
  }

  return get_native_type(type);
}

const std::string& cpp_name_resolver::get_return_type(const t_function& fun) {
  // Most functions don't have sinks or streams so handle the common case first.
  if (!fun.sink_or_stream()) {
    const t_type& return_type = *fun.return_type();
    return detail::get_or_gen(
        type_cache_, &return_type, [&]() { return gen_type(return_type); });
  }

  type_resolve_fn resolve_fn = &cpp_name_resolver::get_native_type;
  const t_sink* sink = fun.sink();
  const t_stream* stream = fun.stream();
  if (sink != nullptr && stream != nullptr) {
    return detail::get_or_gen(bidi_cache_, sink, [&]() {
      if (!fun.has_void_initial_response()) {
        return detail::gen_template_type(
            "::apache::thrift::ResponseAndStreamTransformation",
            {resolve(resolve_fn, *fun.return_type().get_type()),
             resolve(resolve_fn, *sink->get_elem_type()),
             resolve(resolve_fn, *stream->elem_type().get_type())});
      }
      return detail::gen_template_type(
          "::apache::thrift::StreamTransformation",
          {resolve(resolve_fn, *sink->get_elem_type()),
           resolve(resolve_fn, *stream->elem_type().get_type())});
    });
  }

  if (sink != nullptr) {
    return detail::get_or_gen(sink_cache_, sink, [&]() {
      if (!fun.has_void_initial_response()) {
        return detail::gen_template_type(
            "::apache::thrift::ResponseAndSinkConsumer",
            {resolve(resolve_fn, *fun.return_type().get_type()),
             resolve(resolve_fn, *sink->get_elem_type()),
             resolve(resolve_fn, *sink->get_final_response_type())});
      }
      return detail::gen_template_type(
          "::apache::thrift::SinkConsumer",
          {resolve(resolve_fn, *sink->get_elem_type()),
           resolve(resolve_fn, *sink->get_final_response_type())});
    });
  }

  assert(stream != nullptr);
  return detail::get_or_gen(stream_cache_, stream, [&]() {
    if (!fun.has_void_initial_response()) {
      return detail::gen_template_type(
          "::apache::thrift::ResponseAndServerStream",
          {resolve(resolve_fn, *fun.return_type().get_type()),
           resolve(resolve_fn, *stream->elem_type().get_type())});
    }
    return detail::gen_template_type(
        "::apache::thrift::ServerStream",
        {resolve(resolve_fn, *stream->elem_type().get_type())});
  });
}

const std::string& cpp_name_resolver::get_underlying_type_name(
    const t_type& node) {
  if (auto* annotation = find_nontransitive_adapter(node)) {
    if (auto* adapted_type =
            annotation->get_value_from_structured_annotation_or_null(
                "adaptedType")) {
      return adapted_type->get_string();
    }
    const auto& adapter =
        annotation->get_value_from_structured_annotation("name").get_string();
    return detail::get_or_gen(underlying_type_cache_, &node, [&]() {
      return gen_adapted_type(&adapter, get_underlying_namespaced_name(node));
    });
  }
  return get_native_type(node);
}

const std::string& cpp_name_resolver::get_underlying_type_name(
    const t_typedef& node) {
  // When `t_placeholder_typedef` is used, `t_type_ref::deref` will
  // automatically dereference `t_placeholder_typedef` as well. Since
  // unstructured annotations are stored in `t_placeholder_typedef`, we can't
  // use `t_type_ref::deref`.
  const t_type* type = node.type().get_type();
  if (type == nullptr) {
    throw std::runtime_error("t_type_ref has no type.");
  }

  if (auto* annotation = find_nontransitive_adapter(node)) {
    if (auto* adapted_type =
            annotation->get_value_from_structured_annotation_or_null(
                "adaptedType")) {
      return adapted_type->get_string();
    }
    const auto& adapter =
        annotation->get_value_from_structured_annotation("name").get_string();
    return detail::get_or_gen(underlying_type_cache_, &node, [&]() {
      return gen_adapted_type(&adapter, get_native_type(*type));
    });
  }
  return get_native_type(*type);
}

const std::string& cpp_name_resolver::get_underlying_namespaced_name(
    const t_type& node) {
  return detail::get_or_gen(underlying_namespaced_name_cache_, &node, [&] {
    if (auto program = node.program()) {
      auto extra = get_extra_namespace(node);
      return fmt::format(
          "{}::{}{}",
          get_namespace(*program),
          extra ? *extra + "::" : "",
          get_underlying_name(node));
    }
    return gen_standard_type(
        node, &cpp_name_resolver::get_underlying_namespaced_name);
  });
}

const std::string& cpp_name_resolver::get_underlying_name(const t_type& node) {
  if (const t_const* annotation = find_nontransitive_adapter(node)) {
    if (const t_const_value* value =
            annotation->get_value_from_structured_annotation_or_null(
                "underlyingName")) {
      return value->get_string();
    }
  }
  return cpp_name_resolver::get_cpp_name(node);
}

const std::string* cpp_name_resolver::get_extra_namespace(const t_type& node) {
  if (const t_const* annotation = find_nontransitive_adapter(node)) {
    const auto* ns = annotation->get_value_from_structured_annotation_or_null(
        "extraNamespace");
    if (ns != nullptr && !ns->get_string().empty()) {
      return &ns->get_string();
    }
    // If there is also no underlying name, use "detail".
    const auto* name = annotation->get_value_from_structured_annotation_or_null(
        "underlyingName");
    if (name == nullptr || name->get_string().empty()) {
      static const std::string detail = "detail";
      return &detail;
    }
  }
  return nullptr;
}

const std::string& cpp_name_resolver::get_storage_type(
    const t_field& field, const t_structured& parent) {
  auto ref_type = gen::cpp::find_ref_type(field);
  const std::string& native_type = get_native_type(field, parent);
  if (ref_type == cpp_reference_type::none) {
    // The storage type is just the type name.
    return native_type;
  }
  return detail::get_or_gen(storage_type_cache_, &field, [&]() {
    return gen_storage_type(native_type, ref_type);
  });
}

const std::string& cpp_name_resolver::get_reference_type(const t_field& node) {
  return detail::get_or_gen(field_reference_type_cache_, &node, [&]() {
    return gen_reference_type(node);
  });
}

const t_const* cpp_name_resolver::find_nontransitive_adapter(
    const t_type& node) {
  if (!is_transitive_annotation(node)) {
    return node.find_structured_annotation_or_null(kCppAdapterUri);
  }
  return nullptr;
}

const std::string* cpp_name_resolver::find_first_adapter(const t_type& node) {
  if (auto annotation = t_typedef::get_first_structured_annotation_or_null(
          &node, kCppAdapterUri)) {
    return &annotation->get_value_from_structured_annotation("name")
                .get_string();
  }
  return nullptr;
}

const std::string* cpp_name_resolver::find_first_adapter(const t_field& field) {
  if (const std::string* adapter = find_structured_adapter_annotation(field)) {
    return adapter;
  }
  if (const std::string* adapter = find_first_adapter(*field.type())) {
    return adapter;
  }
  return nullptr;
}

std::vector<std::string> cpp_name_resolver::gen_namespace_components(
    const t_program& program) {
  t_program::namespace_config conf;
  conf.no_top_level_domain = true;
  auto components = program.gen_namespace_or_default("cpp2", conf);
  if (components.empty()) {
    components = program.gen_namespace_or_default("cpp", conf);
    components.push_back("cpp2");
  }
  return components;
}

std::string cpp_name_resolver::gen_namespace(const t_program& program) {
  return "::" + gen_unprefixed_namespace(program);
}

std::string cpp_name_resolver::gen_unprefixed_namespace(
    const t_program& program) {
  const auto components = gen_namespace_components(program);
  return fmt::format("{}", fmt::join(components, "::"));
}

bool cpp_name_resolver::can_resolve_to_scalar(const t_type& node) {
  return is_scalar(*node.get_true_type()) || find_first_adapter(node) ||
      find_first_type(node);
}

const std::string& cpp_name_resolver::default_template(
    t_container::type ctype) {
  switch (ctype) {
    case t_container::type::t_list: {
      static const auto& value = *new std::string("::std::vector");
      return value;
    }
    case t_container::type::t_set: {
      static const auto& value = *new std::string("::std::set");
      return value;
    }
    case t_container::type::t_map: {
      static const auto& value = *new std::string("::std::map");
      return value;
    }
  }
  throw std::runtime_error(
      "unknown container type: " + std::to_string(static_cast<int>(ctype)));
}

const std::string& cpp_name_resolver::default_type(
    t_primitive_type::type btype) {
  switch (btype) {
    case t_primitive_type::type::t_void: {
      static const auto& value = *new std::string("void");
      return value;
    }
    case t_primitive_type::type::t_bool: {
      static const auto& value = *new std::string("bool");
      return value;
    }
    case t_primitive_type::type::t_byte: {
      static const auto& value = *new std::string("::std::int8_t");
      return value;
    }
    case t_primitive_type::type::t_i16: {
      static const auto& value = *new std::string("::std::int16_t");
      return value;
    }
    case t_primitive_type::type::t_i32: {
      static const auto& value = *new std::string("::std::int32_t");
      return value;
    }
    case t_primitive_type::type::t_i64: {
      static const auto& value = *new std::string("::std::int64_t");
      return value;
    }
    case t_primitive_type::type::t_float: {
      static const auto& value = *new std::string("float");
      return value;
    }
    case t_primitive_type::type::t_double: {
      static const auto& value = *new std::string("double");
      return value;
    }
    case t_primitive_type::type::t_string:
    case t_primitive_type::type::t_binary: {
      static const auto& value = *new std::string("::std::string");
      return value;
    }
  }
  throw std::runtime_error(
      "unknown base type: " + std::to_string(static_cast<int>(btype)));
}

std::string cpp_name_resolver::gen_type(const t_type& node) {
  if (const auto* type = find_type(node)) {
    // Use the override.
    return *type;
  }
  // Use the unmodified name.
  return gen_standard_type(node, &cpp_name_resolver::get_native_type);
}

std::string cpp_name_resolver::gen_standard_type(const t_type& node) {
  if (const auto* type = find_type(node)) {
    // Return the override.
    return *type;
  }

  if (const auto* ttypedef = node.try_as<t_typedef>()) {
    // Traverse the typedef.
    // TODO(afuller): Always traverse the adapter. There are some cpp.type and
    // cpp.template annotations that rely on the namespacing of the typedef to
    // avoid namespacing issues with the annotation itself. To avoid breaking
    // these cases we are only traversing the typedef when the presences of an
    // adapter requires we do so. However, we should update all annotations to
    // using fully qualified names, then always traverse here.
    if (find_first_adapter(node) != nullptr) {
      return get_standard_type(*ttypedef->get_type());
    }
  }

  if (is_directly_adapted(node)) {
    return get_underlying_namespaced_name(node);
  }

  return gen_standard_type(node, &cpp_name_resolver::get_standard_type);
}

std::string cpp_name_resolver::gen_standard_type(const t_field& node) {
  const t_type& type = *node.type();
  if (auto* annotation = node.find_structured_annotation_or_null(kCppTypeUri)) {
    if (auto name =
            annotation->get_value_from_structured_annotation_or_null("name")) {
      return detail::get_or_gen(field_standard_type_cache_, &node, [&]() {
        return name->get_string();
      });
    } else {
      auto& tmplate =
          annotation->get_value_from_structured_annotation("template");
      return detail::get_or_gen(field_standard_type_cache_, &node, [&]() {
        return gen_container_type(
            type.get_true_type()->as<t_container>(),
            &cpp_name_resolver::get_native_type,
            &tmplate.get_string());
      });
    }
  }
  return gen_standard_type(type);
}

std::string cpp_name_resolver::gen_storage_type(
    const std::string& native_type, cpp_reference_type ref_type) {
  switch (ref_type) {
    case cpp_reference_type::unique:
      return detail::gen_template_type("::std::unique_ptr", {native_type});
    case cpp_reference_type::shared_mutable:
      return detail::gen_template_type("::std::shared_ptr", {native_type});
    case cpp_reference_type::shared_const:
      return detail::gen_template_type(
          "::std::shared_ptr", {"const " + native_type});
    case cpp_reference_type::boxed:
      return detail::gen_template_type(
          "::apache::thrift::detail::boxed_value_ptr", {native_type});
    case cpp_reference_type::boxed_intern:
      return detail::gen_template_type(
          "::apache::thrift::detail::boxed_value", {native_type});
    default:
      throw std::runtime_error("unknown cpp ref_type");
  }
}

std::string cpp_name_resolver::gen_standard_type(
    const t_type& node, type_resolve_fn resolve_fn) {
  // Base types have fixed type mappings.
  if (const auto* tbase_type = node.try_as<t_primitive_type>()) {
    return default_type(tbase_type->primitive_type());
  }

  // Containers have fixed template mappings.
  if (const auto* tcontainer = node.try_as<t_container>()) {
    return gen_container_type(*tcontainer, resolve_fn);
  }

  // If there's a template annotation on a typedef we have to pass it to the
  // underlying container type.
  if (const auto* templte = find_template(node)) {
    if (const auto* tcontainer = node.get_true_type()->try_as<t_container>()) {
      return gen_container_type(*tcontainer, resolve_fn, templte);
    }
  }

  // For everything else, just use namespaced name.
  return get_namespaced_name(node);
}

std::string cpp_name_resolver::gen_container_type(
    const t_container& node,
    type_resolve_fn resolve_fn,
    const std::string* val) {
  val = val ? val : find_template(node);
  const auto& template_name =
      val ? *val : default_template(node.container_type());

  if (const t_list* list = node.try_as<t_list>()) {
    return detail::gen_template_type(
        template_name, {resolve(resolve_fn, *list->elem_type())});
  } else if (const t_set* set = node.try_as<t_set>()) {
    return detail::gen_template_type(
        template_name, {resolve(resolve_fn, *set->elem_type())});
  } else if (const t_map* tmap = node.try_as<t_map>()) {
    return detail::gen_template_type(
        template_name,
        {resolve(resolve_fn, *tmap->get_key_type()),
         resolve(resolve_fn, *tmap->get_val_type())});
  }
  throw std::runtime_error(
      fmt::format("unknown container type: {}", typeid(node).name()));
}

std::string cpp_name_resolver::gen_adapted_type(
    const std::string* adapter, const std::string& standard_type) {
  return adapter == nullptr ? standard_type
                            : detail::gen_template_type(
                                  "::apache::thrift::adapt_detail::adapted_t",
                                  {*adapter, standard_type});
}
std::string cpp_name_resolver::gen_adapted_type(
    const std::string* adapter,
    int16_t field_id,
    const std::string& standard_type,
    const t_structured& parent) {
  return adapter == nullptr
      ? standard_type
      : detail::gen_template_type(
            "::apache::thrift::adapt_detail::adapted_field_t",
            {
                *adapter,
                std::to_string(field_id),
                standard_type,
                get_underlying_name(parent),
            });
}

std::string cpp_name_resolver::gen_type_tag(
    const t_type& type, bool ignore_cpp_type) {
  std::string tag = type.is<t_typedef>()
      ? gen_type_tag(*type.as<t_typedef>().get_type())
      : gen_thrift_type_tag(type);

  if (!ignore_cpp_type &&
      (cpp_name_resolver::find_type(type) ||
       cpp_name_resolver::find_template(type))) {
    return fmt::format(
        "::apache::thrift::type::cpp_type<{}, {}>", get_native_type(type), tag);
  }

  if (const auto* adapter =
          type.find_structured_annotation_or_null(kCppAdapterUri)) {
    return fmt::format(
        "::apache::thrift::type::adapted<{}, {}>",
        adapter->get_value_from_structured_annotation("name").get_string(),
        tag);
  }

  return tag;
}

std::string cpp_name_resolver::gen_type_tag(
    const t_field& field, const t_structured& parent) {
  std::string type_tag;

  const std::string* adapter = find_structured_adapter_annotation(field);

  // TODO(dokwon): Remove allowing both @cpp.Type and @cpp.Adapter on a field
  // once @scope.Transitive bug is fixed.
  if (field.find_structured_annotation_or_null(kCppTypeUri)) {
    type_tag = fmt::format(
        "::apache::thrift::type::cpp_type<{}, {}>",
        adapter ? get_native_type(*field.type())
                : get_native_type(field, parent),
        gen_type_tag(*field.type(), true));
  } else {
    type_tag = gen_type_tag(*field.type());
  }
  if (adapter) {
    return fmt::format(
        "::apache::thrift::type::adapted<{}, {}>", *adapter, type_tag);
  }
  return type_tag;
}

std::string cpp_name_resolver::gen_thrift_type_tag(
    const t_type& original_type) {
  static const std::string ns = "::apache::thrift::type::";
  const auto& type = *original_type.get_true_type();
  if (type.is_void()) {
    return ns + "void_t";
  } else if (type.is_bool()) {
    return ns + "bool_t";
  } else if (type.is_byte()) {
    return ns + "byte_t";
  } else if (type.is_i16()) {
    return ns + "i16_t";
  } else if (type.is_i32()) {
    return ns + "i32_t";
  } else if (type.is_i64()) {
    return ns + "i64_t";
  } else if (type.is_float()) {
    return ns + "float_t";
  } else if (type.is_double()) {
    return ns + "double_t";
  } else if (type.is<t_enum>()) {
    return ns + "enum_t<" + get_standard_type(type) + ">";
  } else if (type.is_string()) {
    return ns + "string_t";
  } else if (type.is_binary()) {
    return ns + "binary_t";
  } else if (const t_list* list = type.try_as<t_list>()) {
    auto& elem = *list->elem_type();
    auto elem_tag = gen_type_tag(elem);
    return ns + "list<" + elem_tag + ">";
  } else if (const t_set* set = type.try_as<t_set>()) {
    auto& elem = *set->elem_type();
    auto elem_tag = gen_type_tag(elem);
    return ns + "set<" + elem_tag + ">";
  } else if (const t_map* map = type.try_as<t_map>()) {
    auto& key = *map->get_key_type();
    auto& val = *map->get_val_type();
    auto key_tag = gen_type_tag(key);
    auto val_tag = gen_type_tag(val);
    return ns + "map<" + key_tag + ", " + val_tag + ">";
  } else if (type.is<t_union>()) {
    return ns + "union_t<" + get_standard_type(type) + ">";
  } else if (type.is<t_struct>()) {
    return ns + "struct_t<" + get_standard_type(type) + ">";
  } else if (type.is<t_exception>()) {
    return ns + "exception_t<" + get_standard_type(type) + ">";
  } else {
    throw std::runtime_error("unknown type for: " + type.get_full_name());
  }
}

std::string cpp_name_resolver::gen_reference_type(const t_field& node) {
  const std::string ns = "::apache::thrift::";

  if (gen::cpp::find_ref_type(node) == cpp_reference_type::boxed) {
    switch (node.get_req()) {
      case t_field::e_req::optional:
        return ns + "optional_boxed_field_ref";
      case t_field::e_req::opt_in_req_out:
      case t_field::e_req::terse:
      case t_field::e_req::required:
      default:
        throw std::runtime_error("unsupported boxed field");
    }
  }

  if (gen::cpp::find_ref_type(node) == cpp_reference_type::boxed_intern) {
    switch (node.get_req()) {
      case t_field::e_req::opt_in_req_out:
        return ns + "intern_boxed_field_ref";
      case t_field::e_req::terse:
        return ns + "terse_intern_boxed_field_ref";
      case t_field::e_req::required:
      case t_field::e_req::optional:
      default:
        throw std::runtime_error("unsupported intern boxed field");
    }
  }

  switch (node.get_req()) {
    case t_field::e_req::required:
      return ns + "required_field_ref";
    case t_field::e_req::optional:
      return ns + "optional_field_ref";
    case t_field::e_req::opt_in_req_out:
      return ns + "field_ref";
    case t_field::e_req::terse:
      return ns + "terse_field_ref";
    default:
      throw std::runtime_error("unknown qualifier");
  }
}

const std::string* cpp_name_resolver::get_string_from_annotation_or_null(
    const t_named& node, const char* uri, const char* key) {
  if (const t_const* annotation =
          node.find_structured_annotation_or_null(uri)) {
    return &annotation->get_value_from_structured_annotation(key).get_string();
  }
  return nullptr;
}

} // namespace apache::thrift::compiler
