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

#include <thrift/compiler/ast/t_named.h>

#include <thrift/compiler/ast/t_const.h>
#include <thrift/compiler/ast/t_program.h>
#include <thrift/compiler/ast/t_type.h>
#include <thrift/compiler/lib/uri.h>

namespace apache {
namespace thrift {
namespace compiler {

t_named::t_named(const t_program* program, std::string name)
    : name_(std::move(name)), program_(program) {}

t_named::t_named(const t_named& named)
    : t_node::t_node(named),
      name_(named.name_),
      program_(named.program_),
      generated_(named.generated_),
      uri_(named.uri_),
      explicit_uri_(named.explicit_uri_) {
  for (const auto& annotation : named.structured_annotations_) {
    add_structured_annotation(annotation->clone());
  }
}

// NOTE: Must be defined here for t_const's destructor defintion.
t_named::~t_named() = default;

void t_named::add_structured_annotation(std::unique_ptr<t_const> annot) {
  assert(annot->type() != nullptr);
  structured_annotations_.emplace_back(std::move(annot));
}

const t_const* t_named::find_structured_annotation_or_null(
    const char* uri) const {
  for (const auto& annotation : structured_annotations_) {
    const t_type& annotation_type = *annotation->type();
    if (annotation_type.uri() == uri) {
      return annotation.get();
    }
    if (is_transitive_annotation(annotation_type)) {
      if (const t_const* annot =
              annotation_type.find_structured_annotation_or_null(uri)) {
        return annot;
      }
    }
  }
  return nullptr;
}

bool is_transitive_annotation(const t_named& node) {
  for (const auto& annotation : node.structured_annotations()) {
    if (annotation.type()->uri() == kTransitiveUri) {
      return true;
    }
  }
  return false;
}

} // namespace compiler
} // namespace thrift
} // namespace apache
