/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <memory>
#include <string>
#include <vector>

namespace watchman {

/**
 * A node in the tree of node matching rules.
 */
struct GlobTree {
  std::string pattern;

  // The list of child rules, excluding any ** rules
  std::vector<std::unique_ptr<GlobTree>> children;
  // The list of ** rules that exist under this node
  std::vector<std::unique_ptr<GlobTree>> doublestar_children;

  unsigned is_leaf : 1; // if true, generate files for matches
  unsigned had_specials : 1; // if false, can do simple string compare
  unsigned is_doublestar : 1; // pattern begins with **

  GlobTree(const char* pattern, uint32_t pattern_len);

  // Produces a list of globs from the glob tree, effectively
  // performing the reverse of the original parsing operation.
  std::vector<std::string> unparse() const;

  // A helper method for unparse
  void unparse_into(
      std::vector<std::string>& globStrings,
      std::string_view relative) const;
};

} // namespace watchman
