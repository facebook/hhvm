/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "watchman/query/GlobTree.h"

#include <fmt/core.h>
#include <folly/Range.h>

namespace watchman {

GlobTree::GlobTree(const char* pattern, uint32_t pattern_len)
    : pattern(pattern, pattern_len),
      is_leaf(0),
      had_specials(0),
      is_doublestar(0) {}

std::vector<std::string> GlobTree::unparse() const {
  std::vector<std::string> result;
  unparse_into(result, "");
  return result;
}

// Performs the heavy lifting for reversing the parse process
// to compute a list of glob strings.
// `globStrings` is the target array for the glob expressions.
// `relative` is the glob-expression-so-far that the current
// node will append to when it produces its glob string output.
// This function recurses down the glob tree calling unparse_into
// on its children.
void GlobTree::unparse_into(
    std::vector<std::string>& globStrings,
    std::string_view relative) const {
  auto needSlash =
      !relative.empty() && !folly::StringPiece{relative}.endsWith('/');
  auto optSlash = needSlash ? "/" : "";

  auto join = [&]() -> std::string {
    return fmt::format("{}{}{}", relative, optSlash, pattern);
  };

  // If there are no children of this node, it is effectively a leaf
  // node. Leaves correspond to a concrete glob string that we need
  // to emit, so here's where we do that.
  if (is_leaf || children.size() + doublestar_children.size() == 0) {
    globStrings.push_back(join());
  }

  for (auto& child : children) {
    child->unparse_into(globStrings, join());
  }
  for (auto& child : doublestar_children) {
    child->unparse_into(globStrings, join());
  }
}

} // namespace watchman
