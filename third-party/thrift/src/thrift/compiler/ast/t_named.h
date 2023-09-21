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

#include <map>
#include <memory>
#include <string>
#include <vector>

#include <boost/optional.hpp>

#include <thrift/compiler/ast/node_list.h>
#include <thrift/compiler/ast/t_node.h>
#include <thrift/compiler/lib/uri.h>

namespace apache {
namespace thrift {
namespace compiler {

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

  node_list_view<const t_const> structured_annotations() const {
    return structured_annotations_;
  }
  node_list_view<t_const> structured_annotations() {
    return structured_annotations_;
  }
  void add_structured_annotation(std::unique_ptr<t_const> annot);

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
  const std::string& doc() const { return doc_ ? doc_->value : kEmptyString; }

  bool has_doc() const { return !!doc_; }

  source_range doc_range() const { return doc_ ? doc_->range : source_range{}; }

  void set_doc(std::string doc, source_range range) {
    doc_ = node_doc{std::move(doc), range};
  }

 protected:
  explicit t_named(const t_program* program = nullptr, std::string name = "");
  t_named(const t_named& named);

  // TODO(afuller): make private.
  std::string name_;
  const t_program* program_;

 private:
  bool generated_ = false;
  std::string uri_;
  bool explicit_uri_ = false;
  std::vector<std::unique_ptr<t_const>> structured_annotations_;

  struct node_doc {
    std::string value;
    source_range range;
  };
  boost::optional<node_doc> doc_;

  // TODO(afuller): Remove everything below this comment. It is only provided
  // for backwards compatibility.
 public:
  const std::string& get_name() const { return name_; }
};

// Returns true iff the node is a definition of a transitive annotation,
// i.e. it has the @scope.Transitive annotation itself.
bool is_transitive_annotation(const t_named& node);

} // namespace compiler
} // namespace thrift
} // namespace apache
