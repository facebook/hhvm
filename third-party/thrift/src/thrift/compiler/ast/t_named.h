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

#include <memory>
#include <optional>
#include <string>
#include <vector>

#include <thrift/compiler/ast/node_list.h>
#include <thrift/compiler/ast/t_node.h>
#include <thrift/compiler/ast/uri.h>

namespace apache::thrift::compiler {

class t_const;
class t_program;

/**
 * A base class for any named AST node such as a definition.
 *
 * Anything that is named, can be annotated.
 */
class t_named : public t_node {
 public:
  ~t_named() override;

  const std::string& name() const noexcept { return name_; }
  void set_name(const std::string& name) { name_ = name; }

  // Returns a string in the format "program_name.type_name"
  std::string get_scoped_name() const;

  node_list_view<const t_const> structured_annotations() const {
    return structured_annotations_;
  }
  node_list_view<t_const> structured_annotations() {
    return structured_annotations_;
  }
  void add_structured_annotation(std::unique_ptr<t_const> annot);

  bool has_structured_annotation(const char* uri) const;

  const t_const* find_structured_annotation_or_null(const char* uri) const;

  // The 'universal' name for this node.
  const std::string& uri() const noexcept { return uri_; }
  // If the uri was assigned explicitly, rather than implied by context.
  bool explicit_uri() const noexcept { return explicit_uri_; }
  void set_uri(std::string uri, bool is_explicit = true) {
    uri_ = std::move(uri);
    explicit_uri_ = is_explicit;
  }

  // If this struct was generated rather than defined directly in the IDL.
  bool generated() const noexcept { return generated_; }
  void set_generated(bool value = true) { generated_ = value; }

  // The program this node appears in. Will be null for base types.
  //  Also null for definitions where no one has found value in tracking this
  //  yet, like enum values.
  const t_program* program() const { return program_; }

  // Returns the documentation comment.
  const std::string& doc() const { return doc_ ? doc_->value : emptyString(); }

  bool has_doc() const { return !!doc_; }

  source_range doc_range() const { return doc_ ? doc_->range : source_range{}; }

  void set_doc(std::string doc, source_range range) {
    doc_ = node_doc{std::move(doc), range};
  }

  /**
   * Returns the range, in Thrift source, where this instances name is defined,
   * if any (i.e., if previously set by `set_name_range()`.
   */
  std::optional<source_range> name_range() const { return name_range_; }

  /**
   * Sets the range, in the source Thrift IDL code, corresponding to this
   * instance's `name()`.
   */
  void set_name_range(source_range name_range) { name_range_ = name_range; }

 protected:
  explicit t_named(const t_program* program = nullptr, std::string name = "");
  t_named(const t_named& named);
  t_named& operator=(const t_named&) = delete;
  t_named(t_named&&) = delete;
  t_named& operator=(t_named&&) = delete;

  /**
   * Set the program for this node. This is primarily exposed for the purpose of
   * placeholder typedefs, where the initial program that triggers placeholder
   * generation may not be the final program the resolved type belongs to.
   */
  void set_program(const t_program* program) { program_ = program; }

 private:
  std::string name_;
  const t_program* program_;

  bool generated_ = false;
  std::string uri_;
  bool explicit_uri_ = false;
  std::vector<std::unique_ptr<t_const>> structured_annotations_;

  struct node_doc {
    std::string value;
    source_range range;
  };
  std::optional<node_doc> doc_;

  std::optional<source_range> name_range_;
};

// Returns true iff the node is a definition of a transitive annotation,
// i.e. it has the @scope.Transitive annotation itself.
bool is_transitive_annotation(const t_named& node);

} // namespace apache::thrift::compiler
