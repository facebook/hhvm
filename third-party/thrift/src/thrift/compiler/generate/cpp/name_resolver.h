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

#include <stdint.h>
#include <initializer_list>
#include <string>
#include <unordered_map>

#include <thrift/compiler/ast/t_container.h>
#include <thrift/compiler/ast/t_field.h>
#include <thrift/compiler/ast/t_primitive_type.h>
#include <thrift/compiler/ast/t_type.h>
#include <thrift/compiler/ast/uri.h>
#include <thrift/compiler/generate/cpp/reference_type.h>

namespace apache::thrift::compiler {
namespace detail {
template <typename C, typename K = typename C::key_type, typename G>
auto& get_or_gen(C& cache, const K& key, const G& gen_func) {
  auto itr = cache.find(key);
  if (itr == cache.end()) {
    itr = cache.emplace(key, gen_func()).first;
  }
  return itr->second;
}

std::string gen_template_type(
    std::string template_name, std::initializer_list<std::string> args);
} // namespace detail

class t_function;
class t_named;
class t_program;
class t_sink;
class t_stream;
class t_structured;
class t_typedef;

using cpp_reference_type = gen::cpp::reference_type;

// A class that resolves C++ names from Thrift entities and caches the results.
class cpp_name_resolver {
 public:
  // Returns C++ type name for the given Thrift type.
  const std::string& get_native_type(const t_type& node) {
    return detail::get_or_gen(
        type_cache_, &node, [&]() { return gen_type(node); });
  }
  // Returns C++ type name for the given Thrift field.
  const std::string& get_native_type(
      const t_field& field, const t_structured& parent);

  const std::string& get_native_type(const t_const& cnst);

  // Generates the C++ return type name for a function. It takes into account
  // not just the Thrift function return type but also sink and stream if any.
  const std::string& get_return_type(const t_function& fun);

  const std::string& get_underlying_type_name(const t_type& node);
  const std::string& get_underlying_type_name(const t_typedef& node);

  // Returns the C++ type that the runtime knows how to handle.
  const std::string& get_standard_type(const t_type& node) {
    return detail::get_or_gen(
        standard_type_cache_, &node, [&]() { return gen_standard_type(node); });
  }
  const std::string& get_standard_type(const t_field& node) {
    return detail::get_or_gen(field_standard_type_cache_, &node, [&]() {
      return gen_standard_type(node);
    });
  }

  // Returns C++ type tag of given thrift type
  const std::string& get_type_tag(const t_type& node) {
    return detail::get_or_gen(
        type_tag_cache_, &node, [&] { return gen_type_tag(node); });
  }

  // Returns C++ type tag of given thrift type
  const std::string& get_type_tag(
      const t_field& node, const t_structured& parent) {
    return detail::get_or_gen(field_type_tag_cache_, &node, [&] {
      return gen_type_tag(node, parent);
    });
  }

  // Returns C++ namespace for the given program.
  const std::string& get_namespace(const t_program& node) {
    return detail::get_or_gen(
        namespace_cache_, &node, [&]() { return gen_namespace(node); });
  }

  const std::string& get_namespaced_name(
      const t_program& program, const t_named& node) {
    return detail::get_or_gen(name_cache_, &node, [&]() {
      return gen_namespaced_name(program, node);
    });
  }
  const std::string& get_namespaced_name(
      const t_program* program, const t_named& node) {
    return program == nullptr ? get_cpp_name(node)
                              : get_namespaced_name(*program, node);
  }
  const std::string& get_namespaced_name(const t_type& node) {
    return get_namespaced_name(node.program(), node);
  }

  static const std::string& get_cpp_name(const t_named& node) {
    if (const auto* cpp_name =
            node.find_structured_annotation_or_null(kCppNameUri)) {
      return cpp_name->get_value_from_structured_annotation("value")
          .get_string();
    }
    return node.get_unstructured_annotation("cpp.name", &node.name());
  }

  static std::string gen_namespace(const t_program& progam);
  static std::string gen_unprefixed_namespace(const t_program& progam);
  static std::vector<std::string> gen_namespace_components(
      const t_program& program);

  const std::string& get_underlying_namespaced_name(const t_type& node);

  static const std::string& get_underlying_name(const t_type& node);

  const std::string* get_extra_namespace(const t_type& node);

  // Checks whether a t_type could resolve to a scalar.
  //
  // A type could resolve to a scalar if t_type is
  // a scalar, or has an 'cpp.type' and 'cpp.adapter' annotation.
  // Note, the 'cpp.template' annotation only applies to container
  // types, so it could never resolve to a scalar.
  // In C++, scalars sometimes require a special treatment. For
  // example, they are not initialized by default.
  static bool can_resolve_to_scalar(const t_type& node);

  // Returns the C++ type that should be used to store the field's value.
  //
  // This differs from the type name, when a 'cpp.ref' or 'cpp.ref_type'
  // annotation is applied to the field.
  const std::string& get_storage_type(
      const t_field& field, const t_structured& parent);

  // Returns the C++ reference type of the field.
  const std::string& get_reference_type(const t_field& node);

  static const std::string* find_field_interceptor(const t_field& node) {
    return get_string_from_annotation_or_null(
        node, kCppFieldInterceptorUri, "name");
  }
  static const std::string* find_structured_adapter_annotation(
      const t_named& node) {
    return get_string_from_annotation_or_null(node, kCppAdapterUri, "name");
  }
  static const t_const* find_nontransitive_adapter(const t_type& node);

  static const std::string* find_type(const t_type& node) {
    return node.find_unstructured_annotation_or_null({"cpp.type", "cpp2.type"});
  }
  static const std::string* find_first_adapter(const t_type& node);
  static const std::string* find_first_adapter(const t_field& node);
  static const std::string* find_template(const t_type& node) {
    return node.find_unstructured_annotation_or_null(
        {"cpp.template", "cpp2.template"});
  }

  static bool is_directly_adapted(const t_type& node) {
    return find_nontransitive_adapter(node);
  }

 private:
  using type_resolve_fn =
      const std::string& (cpp_name_resolver::*)(const t_type& node);

  std::unordered_map<const t_program*, std::string> namespace_cache_;
  std::unordered_map<const t_named*, std::string> name_cache_;
  std::unordered_map<const t_type*, std::string> type_cache_;
  std::unordered_map<const t_sink*, std::string> sink_cache_;
  std::unordered_map<const t_stream*, std::string> stream_cache_;
  std::unordered_map<const t_const*, std::string> const_cache_;
  std::unordered_map<const t_field*, std::string> field_type_cache_;
  std::unordered_map<const t_type*, std::string> standard_type_cache_;
  std::unordered_map<const t_field*, std::string> field_standard_type_cache_;
  std::unordered_map<const t_type*, std::string> underlying_type_cache_;
  std::unordered_map<const t_type*, std::string>
      underlying_namespaced_name_cache_;
  std::unordered_map<const t_field*, std::string> storage_type_cache_;
  std::unordered_map<const t_type*, std::string> type_tag_cache_;
  std::unordered_map<const t_field*, std::string> field_type_tag_cache_;
  std::unordered_map<const t_field*, std::string> field_reference_type_cache_;
  // Sinks are not reused, so are sufficient for use as keys
  std::unordered_map<const t_sink*, std::string> bidi_cache_;

  std::string gen_namespaced_name(
      const t_program& program, const t_named& node) {
    return get_namespace(program) + "::" + get_cpp_name(node);
  }
  static const std::string* get_string_from_annotation_or_null(
      const t_named& node, const char* uri, const char* key);

  static const std::string& default_type(t_primitive_type::type btype);
  static const std::string& default_template(const t_container& ctype);

  // Generating functions.
  std::string gen_type(const t_type& node);
  std::string gen_field_type(
      int16_t field_id,
      const t_type& type,
      const t_structured& parent,
      const std::string* adapter) {
    return gen_adapted_type(adapter, field_id, gen_type(type), parent);
  }
  std::string gen_standard_type(const t_type& node);
  std::string gen_standard_type(const t_type& node, type_resolve_fn resolve_fn);
  std::string gen_standard_type(const t_field& node);
  std::string gen_storage_type(
      const std::string& native_type, cpp_reference_type ref_type);
  std::string gen_container_type(
      const t_container& node,
      type_resolve_fn resolve_fn,
      const std::string* templte = nullptr);
  static std::string gen_adapted_type(
      const std::string* adapter, const std::string& standard_type);
  static std::string gen_adapted_type(
      const std::string* adapter,
      int16_t field_id,
      const std::string& standard_type,
      const t_structured& parent);

  std::string gen_thrift_type_tag(const t_type&);
  // TODO(dokwon): Remove ignored_cpp_type once cpp.type lowering migration is
  // complete.
  std::string gen_type_tag(const t_type&, bool ignore_cpp_type = false);
  std::string gen_type_tag(const t_field&, const t_structured&);
  std::string gen_reference_type(const t_field& node);

  const std::string& resolve(type_resolve_fn resolve_fn, const t_type& node) {
    return (this->*resolve_fn)(node);
  }
};

} // namespace apache::thrift::compiler
