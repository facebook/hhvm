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
#include <memory>
#include <ostream>
#include <string>
#include <string_view>

#include <fmt/core.h>

// This file implements building blocks that can be used to print a tree
// structure to an output stream.
//
// The format of the tree is loosely based on clang's `-ast-dump`:
//   https://clang.llvm.org/docs/IntroductionToTheClangAST.html#examining-the-ast
//
// The building blocks here do not include any traversal logic and so is not
// tied to any particular data structure. Instead, the data types here can be
// used to track tree depth which informs the indentation of the output.
// Furthermore, the lines of printed tree can be one of {node, property} (see
// below).
namespace whisker::tree_printer {

/**
 * Escapes a string of common special characters, making the output suitable for
 * printing within a tree.
 */
std::string escape(std::string_view str);

/**
 * A scope represents a new line with a new degree of indentation
 * within the output string. A scope can be one of {node, property}. In the
 * output node scope are emphasized so that the tree structure is
 * clearer.
 *
 * A node scope is a scope which represents an actual node type in
 * the tree. For example, in Whisker's AST, a section-block is printed via a
 * node scope but the "^" indicating an inversion is via property
 * scope.
 *
 * A property scope represents some data attached to a "real" node,
 * but is not a node itself.
 */
class scope {
 private:
  /**
   * An object representing the current level of indentation (including the
   * history) for printing the prefix for new lines of the output.
   */
  struct nesting_context
      : public std::enable_shared_from_this<nesting_context> {
    enum class kind { property, node };

    std::shared_ptr<const nesting_context> parent_;
    kind kind_;

    explicit nesting_context(
        std::shared_ptr<const nesting_context> parent, kind kind)
        : parent_(std::move(parent)), kind_(kind) {}

    static std::shared_ptr<const nesting_context> make_root() {
      return std::make_shared<nesting_context>(nullptr, kind::node);
    }

    std::shared_ptr<const nesting_context> open_node() const {
      return std::make_shared<nesting_context>(shared_from_this(), kind::node);
    }

    std::shared_ptr<const nesting_context> open_property() const {
      return std::make_shared<nesting_context>(
          shared_from_this(), kind::property);
    }
  };

 public:
  scope open_node() const {
    return scope(*out_, nesting_context_->open_node());
  }
  scope open_property() const {
    return scope(*out_, nesting_context_->open_property());
  }

  template <typename... T>
  void println(fmt::format_string<T...> msg, T&&... args) {
    *out_ << *nesting_context_ << fmt::format(msg, std::forward<T>(args)...)
          << '\n';
  }

  static scope make_root(std::ostream& out) {
    return scope(out, nesting_context::make_root());
  }

 private:
  explicit scope(std::ostream& out, std::shared_ptr<const nesting_context> ctx)
      : out_(&out), nesting_context_(std::move(ctx)) {
    assert(nesting_context_ != nullptr);
  }

  std::ostream* out_;
  std::shared_ptr<const nesting_context> nesting_context_;

  friend std::ostream& operator<<(
      std::ostream& out, const nesting_context& self);
};

} // namespace whisker::tree_printer
