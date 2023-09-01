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

#include <cassert>
#include <map>
#include <string>
#include <vector>

#include <boost/optional.hpp>

#include <thrift/compiler/ast/alias_span.h>
#include <thrift/compiler/source_location.h>

namespace apache {
namespace thrift {
namespace compiler {

struct annotation_value {
  source_range src_range;
  std::string value;
};

/**
 * A Thrift AST node.
 */
class t_node {
 public:
  virtual ~t_node() = default;

  // A source range `[begin, end)` where `begin` and `end` are location of the
  // first and one past the last code units of this node in source respectively.
  const source_range& src_range() const { return range_; }
  void set_src_range(const source_range& r) { range_ = r; }

  // TODO: move to t_named.
  const std::string& doc() const { return doc_ ? doc_->value : kEmptyString; }
  bool has_doc() const { return !!doc_; }
  void set_doc(std::string doc, source_range range) {
    doc_ = node_doc{std::move(doc), range};
  }
  source_range doc_range() const { return doc_ ? doc_->range : source_range{}; }

  // The annotations declared directly on this node.
  const auto& annotations() const { return annotations_; }

  // Returns true if there exists an annotation with the given name.
  bool has_annotation(alias_span name) const {
    return find_annotation_or_null(name) != nullptr;
  }
  bool has_annotation(const char* name) const {
    return has_annotation(alias_span{name});
  }

  // Returns the pointer to the value of the first annotation found with the
  // given name.
  //
  // If not found returns nullptr.
  const std::string* find_annotation_or_null(alias_span name) const;
  const std::string* find_annotation_or_null(const char* name) const {
    return find_annotation_or_null(alias_span{name});
  }

  // Returns the value of an annotation with the given name.
  //
  // If not found returns the provided default or "".
  // TODO(dokwon): Refactor get_annotation to use string_view.
  template <
      typename T = std::vector<std::string>,
      typename D = const std::string*>
  decltype(auto) get_annotation(
      const T& name, D&& default_value = nullptr) const {
    return annotation_or(
        find_annotation_or_null(alias_span{name}),
        std::forward<D>(default_value));
  }

  void reset_annotations(std::map<std::string, annotation_value> annotations) {
    annotations_ = std::move(annotations);
  }

  void set_annotation(
      const std::string& key,
      const std::string& value = {},
      const source_range& range = {}) {
    annotations_[key] = {range, value};
  }

 protected:
  // t_node is abstract.
  t_node() = default;

  static const std::string kEmptyString;

  template <typename D>
  static std::string annotation_or(const std::string* val, D&& def) {
    if (val != nullptr) {
      return *val;
    }
    return std::forward<D>(def);
  }

  static const std::string& annotation_or(
      const std::string* val, const std::string* def) {
    return val ? *val : (def ? *def : kEmptyString);
  }

  static const std::string& annotation_or(
      const std::string* val, std::string* def) {
    return val ? *val : (def ? *def : kEmptyString);
  }

 private:
  source_range range_;

  struct node_doc {
    std::string value;
    source_range range;
  };
  boost::optional<node_doc> doc_;

  std::map<std::string, annotation_value> annotations_;
};

using t_annotation = std::map<std::string, annotation_value>::value_type;

} // namespace compiler
} // namespace thrift
} // namespace apache
