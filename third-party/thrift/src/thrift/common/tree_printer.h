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

#include <iosfwd>
#include <ostream>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include <fmt/core.h>

namespace apache::thrift::tree_printer {

/**
 * This class is used to print a hierarchical data as a tree structure to an
 * output stream. A scope object represents a node within the tree structure. It
 * includes one line of text, and (optionally) children scopes.
 *
 * Example:
 *
 *     auto root = tree_printer::scope::make_root("encyclopedia");
 *     {
 *       tree_printer::scope& culture = root.make_child("culture");
 *       culture.make_child("art");
 *       culture.make_child("craft");
 *     }
 *     {
 *       tree_printer::scope& science = root.make_child("science");
 *       science.make_child("physics");
 *       science.make_child("chemistry");
 *     }
 *     tree_printer::to_string(root)
 *
 * Produces the following output:
 *
 *     encyclopedia
 *     ├─ culture
 *     │  ├─ art
 *     │  ╰─ craft
 *     ╰─ science
 *        ├─ physics
 *        ╰─ chemistry
 *
 * The format of the output is often referred to as a "tree view":
 *  https://en.wikipedia.org/wiki/Tree_structure#Outlines_and_tree_views
 */
class scope {
 public:
  scope(scope&&) = default;
  scope& operator=(scope&&) = default;

  /**
   * Returns the depth of this scope in the tree being printed. This can be used
   * to prune the tree (i.e. deep nodes are condensed to "...") if necessary.
   */
  unsigned depth() const { return depth_; }

  /**
   * Prints a formatted string to the current scope's text.
   *
   * WARNING: newlines in the formatted string will cause the printed tree
   * structure to be malformed.
   */
  template <typename... T>
  void print(fmt::format_string<T...> msg, T&&... args) {
    fmt::format_to(std::back_inserter(data_), msg, std::forward<T>(args)...);
  }

  /**
   * Opens a new child scope with one additional level of depth.
   */
  scope& make_child() { return children_.emplace_back(scope(depth_ + 1)); }
  /**
   * Opens a new child scope and prints a formatted string to the its line.
   */
  template <typename... T>
  scope& make_child(fmt::format_string<T...> msg, T&&... args) {
    scope& child = make_child();
    child.print(msg, std::forward<T>(args)...);
    return child;
  }

  /**
   * Create a scope that represents the root of a new tree.
   */
  static scope make_root() { return scope(0 /* depth */); }
  /**
   * Create a scope and print a formatted string to its line.
   */
  template <typename... T>
  static scope make_root(fmt::format_string<T...> msg, T&&... args) {
    scope root = make_root();
    root.print(msg, std::forward<T>(args)...);
    return root;
  }

  friend std::ostream& operator<<(std::ostream& out, const scope&);

 private:
  explicit scope(unsigned depth) : depth_(depth) {}

  void print_recursively(
      std::ostream&,
      std::size_t indent,
      std::vector<std::size_t>& active,
      bool last_child) const;

  unsigned depth_;
  std::string data_;
  std::vector<scope> children_;
};

std::string to_string(const scope&);

/**
 * Escapes a string of common special characters, making the output suitable for
 * printing within a tree.
 */
std::string escape(std::string_view str);

} // namespace apache::thrift::tree_printer
