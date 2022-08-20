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

#include <thrift/compiler/ast/t_node.h>
#include <thrift/compiler/lib/uri.h>

namespace apache {
namespace thrift {
namespace compiler {

class t_const;

/**
 * class t_named
 *
 * Base class for any named AST node.
 * Anything that is named, can be annotated.
 */
class t_named : public t_node {
 public:
  ~t_named() override;

  void set_name(const std::string& name) { name_ = name; }
  const std::string& name() const { return name_; }

  const std::vector<const t_const*>& structured_annotations() const {
    return structured_annotations_raw_;
  }
  void add_structured_annotation(std::unique_ptr<t_const> annot);

  const t_const* find_structured_annotation_or_null(const char* uri) const;

  const std::string& uri() const { return uri_; }

  void set_uri(std::string uri, bool is_explicit = true) {
    uri_ = std::move(uri);
    explicit_uri_ = is_explicit;
  }
  // If the uri was assigned explicitly, rather than implied by context.
  bool explicit_uri() const { return explicit_uri_; }

  // If this struct was generated rather than defined directly in the IDL.
  bool generated() const noexcept { return generated_; }
  void set_generated(bool value = true) { generated_ = value; }

 protected:
  // t_named is abstract.
  t_named() = default;
  explicit t_named(std::string name) : name_(std::move(name)) {}

  // TODO(afuller): make private.
  std::string name_;

 private:
  bool generated_ = false;
  std::string uri_;
  bool explicit_uri_ = false;
  std::vector<std::shared_ptr<const t_const>> structured_annotations_;

  // TODO(ytj): use thrift.uri --> t_const map for structured annotation
  std::vector<const t_const*> structured_annotations_raw_;

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
