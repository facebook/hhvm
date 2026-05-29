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

#include <thrift/compiler/ast/t_const.h>
#include <thrift/compiler/ast/t_program.h>
#include <thrift/compiler/ast/uri.h>
#include <thrift/compiler/generate/cpp/reference_type.h>

namespace apache::thrift::compiler::gen::cpp {

namespace {
// Since we can not include `thrift/annotation/cpp.thrift`
// This is a copy of apache::thrift::annotation::RefType
enum class RefType {
  Unique = 0,
  Shared = 1,
  SharedMutable = 2,
};

} // namespace

reference_type find_ref_type(const t_field& node) {
  if (node.has_unstructured_annotation({"cpp.box", "thrift.box"}) ||
      node.has_structured_annotation(kBoxUri)) {
    return reference_type::boxed;
  }

  if (node.has_structured_annotation(kInternBoxUri)) {
    return reference_type::boxed_intern;
  }

  if (const t_const* anno =
          node.find_structured_annotation_or_null(kCppRefUri)) {
    for (const auto& kv : anno->value()->get_map()) {
      if (kv.first->get_string() == "type") {
        switch (static_cast<RefType>(kv.second->get_integer())) {
          case RefType::Unique:
            return reference_type::unique;
          case RefType::Shared:
            return reference_type::shared_const;
          case RefType::SharedMutable:
            return reference_type::shared_mutable;
          default:
            throw std::runtime_error(
                "Unrecognized ref type: " +
                std::to_string(kv.second->get_integer()));
        }
      }
    }

    throw std::runtime_error("cpp.Ref with unspecified type");
  }

  return reference_type::none;
}

bool is_field_accessor_template(const t_field& node) {
  // There are downstream plugin code generators which take the address of
  // field accessor functions. They need to be able to accurately determine if
  // an accessor is emitted as a template or regular function, because taking
  // the address of a template function requires that their codegen emit
  // "&f<>" and a non-template function requires them to emit "&f".
  auto ref_type = gen::cpp::find_ref_type(node);
  return ref_type == gen::cpp::reference_type::none ||
      ref_type == gen::cpp::reference_type::boxed ||
      ref_type == gen::cpp::reference_type::boxed_intern;
}

} // namespace apache::thrift::compiler::gen::cpp
