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
#include <utility>

#include <boost/optional.hpp>
#include <fmt/core.h>

#include <thrift/compiler/ast/t_base_type.h>
#include <thrift/compiler/ast/t_container.h>
#include <thrift/compiler/ast/t_field.h>
#include <thrift/compiler/ast/t_named.h>
#include <thrift/compiler/ast/t_program.h>
#include <thrift/compiler/ast/t_sink.h>
#include <thrift/compiler/ast/t_stream.h>
#include <thrift/compiler/ast/t_type.h>
#include <thrift/compiler/gen/cpp/detail/gen.h>
#include <thrift/compiler/gen/cpp/namespace_resolver.h>
#include <thrift/compiler/gen/cpp/reference_type.h>
#include <thrift/compiler/lib/uri.h>

template <>
struct std::hash<std::pair<
    const apache::thrift::compiler::t_field*,
    apache::thrift::compiler::gen::cpp::reference_type>> {
  std::size_t operator()(const std::pair<
                         const apache::thrift::compiler::t_field*,
                         apache::thrift::compiler::gen::cpp::reference_type>& p)
      const noexcept {
    uintptr_t val = reinterpret_cast<uintptr_t>(p.first);
    uintptr_t ref = static_cast<uintptr_t>(p.second);
    // Adjacent t_field are far enough apart that adding ref type won't
    // introduce collisions.
    assert(sizeof(apache::thrift::compiler::t_field) > ref);
    return std::hash<uintptr_t>()(val + ref);
  }
};

namespace apache {
namespace thrift {
namespace compiler {
namespace gen {
namespace cpp {

// A class that resolves C++ type names from Thrift types and caches the
// results.
class type_resolver {
 public:
  // Returns C++ type name for the given Thrift type.
  const std::string& get_native_type(const t_type& node) {
    return detail::get_or_gen(
        type_cache_, &node, [&]() { return gen_type(node); });
  }
  const std::string& get_native_type(const t_typedef& node);
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

  // Returns C++ type tag of given thrift type
  const std::string& get_type_tag(const t_type& node) {
    return detail::get_or_gen(
        type_tag_cache_, &node, [&] { return gen_type_tag(node); });
  }

  // Returns C++ type tag of given thrift type
  const std::string& get_type_tag(const t_field& node) {
    return detail::get_or_gen(
        field_type_tag_cache_, &node, [&] { return gen_type_tag(node); });
  }

  std::string get_namespaced_name(
      const t_program& program, const t_named& node) {
    return namespaces_.get_namespaced_name(program, node);
  }

  const std::string& get_underlying_namespaced_name(const t_type& node);

  const std::string& get_underlying_name(const t_type& node);

  const std::string* get_extra_namespace(const t_type& node);

  // Checks whether a t_type could resolve to a scalar.
  //
  // A type could resolve to a scalar if t_type is
  // a scalar, or has an 'cpp.type' and 'cpp.adapter' annotation.
  // Note, the 'cpp.template' annotation only applies to container
  // types, so it could never resolve to a scalar.
  // In C++, scalars sometimes require a special treatment. For
  // example, they are not initialized unless the default
  // constructor is explicitly invoked. So if we declare
  // two variables, like:
  //   int i, j{};
  // At this point, i could be any value, as it has not been
  // explicitly initalized, while j is guaranteed to be 0, the
  // intrinsic default for an int. On the other hand, if i
  // had not been a scalar, it would have been initalized
  // implicitly.
  static bool can_resolve_to_scalar(const t_type& node);

  // Returns the c++ type that should be used to store the field's value.
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
    return node.find_annotation_or_null({"cpp.type", "cpp2.type"});
  }
  static const std::string* find_first_adapter(const t_type& node);
  static const std::string* find_first_adapter(const t_field& node);
  static const std::string* find_template(const t_type& node) {
    return node.find_annotation_or_null({"cpp.template", "cpp2.template"});
  }

  static bool is_directly_adapted(const t_type& node) {
    return find_nontransitive_adapter(node);
  }

 private:
  using type_resolve_fn =
      const std::string& (type_resolver::*)(const t_type& node);

  namespace_resolver namespaces_;
  std::unordered_map<const t_type*, std::string> type_cache_;
  std::unordered_map<const t_sink*, std::string> sink_cache_;
  std::unordered_map<const t_const*, std::string> const_cache_;
  std::unordered_map<const t_field*, std::string> field_type_cache_;
  std::unordered_map<const t_type*, std::string> standard_type_cache_;
  std::unordered_map<const t_type*, std::string> underlying_type_cache_;
  std::unordered_map<const t_type*, std::string>
      underlying_namespaced_name_cache_;
  std::unordered_map<std::pair<const t_field*, reference_type>, std::string>
      storage_type_cache_;
  std::unordered_map<const t_type*, std::string> type_tag_cache_;
  std::unordered_map<const t_field*, std::string> field_type_tag_cache_;
  std::unordered_map<const t_field*, std::string> field_reference_type_cache_;

  static const std::string* get_string_from_annotation_or_null(
      const t_named& node, const char* uri, const char* key);

  static const std::string& default_type(t_base_type::type btype);
  static const std::string& default_template(t_container::type ctype);

  const std::string& get_namespace(const t_program& program);

  // Generatating functions.
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
  std::string gen_storage_type(
      const std::string& native_type, reference_type& ref_type, const t_field&);
  std::string gen_container_type(
      const t_container& node, type_resolve_fn resolve_fn);
  static std::string gen_adapted_type(
      const std::string* adapter, const std::string& standard_type);
  static std::string gen_adapted_type(
      const std::string* adapter,
      int16_t field_id,
      const std::string& standard_type,
      const t_structured& parent);

  std::string gen_thrift_type_tag(const t_type&);
  std::string gen_type_tag(const t_type&);
  std::string gen_type_tag(const t_field&);
  std::string gen_reference_type(const t_field& node);

  const std::string& resolve(type_resolve_fn resolve_fn, const t_type& node) {
    return (this->*resolve_fn)(node);
  }
};

} // namespace cpp
} // namespace gen
} // namespace compiler
} // namespace thrift
} // namespace apache
