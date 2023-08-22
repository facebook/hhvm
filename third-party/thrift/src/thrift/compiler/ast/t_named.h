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

#include <thrift/compiler/ast/node_list.h>
#include <thrift/compiler/ast/t_node.h>
#include <thrift/compiler/lib/uri.h>

namespace apache {
namespace thrift {
namespace compiler {

class t_const;
class t_program;

/**
 * The (pre)release state for a given t_named.
 *
 * Currently configured via the standard annotations:
 *  @thrift.Testing
 *  @thrift.Experimental
 *  @thrift.Beta
 *  @thrift.Released
 *  @thrift.Deprecated
 *  @thrift.Legacy
 *
 * See thrift/annotation/thrift.thrift for more details.
 */
enum class t_release_state {
  testing = -3,
  experimental = -2,
  beta = -1,
  unspecified = 0,
  released = 1,
  deprecated = 2,
  legacy = 3,

  end,
  begin = -3,
};

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

  // The release state of this node.
  t_release_state release_state() const noexcept { return release_state_; }
  void set_release_state(t_release_state state) { release_state_ = state; }

  // The program this node appears in. Will be null for base types.
  //  Also null for definitions where no one has found value in tracking this
  //  yet, like enum values.
  const t_program* program() const { return program_; }

 protected:
  explicit t_named(const t_program* program = nullptr, std::string name = "");
  t_named(const t_named& named);

  // TODO(afuller): make private.
  std::string name_;
  const t_program* program_;

 private:
  bool generated_ = false;
  t_release_state release_state_ = t_release_state::unspecified;
  std::string uri_;
  bool explicit_uri_ = false;
  std::vector<std::unique_ptr<t_const>> structured_annotations_;

  // TODO(afuller): Remove everything below this comment. It is only provided
  // for backwards compatibility.
 public:
  const std::string& get_name() const { return name_; }
};

// Returns true iff the node is a definition of a transitive annotation,
// i.e. it has the @scope.Transitive annotation itself.
bool is_transitive_annotation(const t_named& node);

constexpr const char* get_release_state_uri(t_release_state state) {
  switch (state) {
    case t_release_state::testing:
      return "facebook.com/thrift/annotation/Testing";
    case t_release_state::experimental:
      return "facebook.com/thrift/annotation/Experimental";
    case t_release_state::beta:
      return "facebook.com/thrift/annotation/Beta";
    case t_release_state::unspecified:
      break;
    case t_release_state::released:
      return "facebook.com/thrift/annotation/Released";
    case t_release_state::deprecated:
      return "facebook.com/thrift/annotation/Deprecated";
    case t_release_state::legacy:
      return "facebook.com/thrift/annotation/Legacy";
    default:
      break;
  }
  return "";
}

constexpr t_release_state next(t_release_state cur) {
  return static_cast<t_release_state>(static_cast<int>(cur) + 1);
}
constexpr t_release_state prev(t_release_state cur) {
  return static_cast<t_release_state>(static_cast<int>(cur) - 1);
}

} // namespace compiler
} // namespace thrift
} // namespace apache
