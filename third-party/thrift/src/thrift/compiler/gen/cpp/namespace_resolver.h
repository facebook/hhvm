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

#include <string>
#include <unordered_map>
#include <vector>

#include <thrift/compiler/ast/t_program.h>
#include <thrift/compiler/ast/t_type.h>
#include <thrift/compiler/gen/cpp/gen.h>

namespace apache {
namespace thrift {
namespace compiler {
namespace gen {
namespace cpp {

// A class that resolves c++ namespace names for thrift programs and caches the
// results.
class namespace_resolver {
 public:
  namespace_resolver() = default;

  // Returns c++ namespace for the given program.
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
    return node.get_annotation("cpp.name", &node.name());
  }

  static std::string gen_namespace(const t_program& progam);
  static std::string gen_unprefixed_namespace(const t_program& progam);
  static std::vector<std::string> gen_namespace_components(
      const t_program& program);

 private:
  std::unordered_map<const t_program*, std::string> namespace_cache_;
  std::unordered_map<const t_named*, std::string> name_cache_;

  std::string gen_namespaced_name(
      const t_program& program, const t_named& node) {
    return get_namespace(program) + "::" + get_cpp_name(node);
  }
};

} // namespace cpp
} // namespace gen
} // namespace compiler
} // namespace thrift
} // namespace apache
