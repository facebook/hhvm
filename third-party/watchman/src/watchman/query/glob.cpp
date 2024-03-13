/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <folly/ScopeGuard.h>
#include <memory>
#include "watchman/CommandRegistry.h"
#include "watchman/Errors.h"
#include "watchman/query/GlobTree.h"
#include "watchman/query/Query.h"
#include "watchman/query/QueryContext.h"
#include "watchman/query/eval.h"
#include "watchman/thirdparty/wildmatch/wildmatch.h"
#include "watchman/watchman_dir.h"
#include "watchman/watchman_file.h"

using std::make_unique;

namespace watchman {

namespace {

/* The glob generator.
 * The user can specify a list of globs as the set of candidate nodes
 * for their query expression.
 * The list may feature redundant components that we desire to avoid
 * matching more times than we need.
 * For example ["some/deep/path/foo.h", "some/deep/path/bar.h"] have
 * a common path prefix that we only want to match once.
 *
 * To deal with this we compile the set of glob patterns into a tree
 * structure, splitting the pattern by the unix directory separator.
 *
 * At execution time we walk down the watchman_dir tree and the pattern
 * tree concurrently.  If the watchman_dir tree has no matching component
 * then we can terminate evaluation of that portion of the pattern tree
 * early.
 */

W_CAP_REG("glob_generator")

// Look ahead in pattern; we want to find the directory separator.
// While we are looking, check for wildmatch special characters.
// If we do not find a directory separator, return NULL.
const char* find_sep_and_specials(
    const char* pattern,
    const char* end,
    bool* had_specials) {
  *had_specials = false;
  while (pattern < end) {
    switch (*pattern) {
      case '*':
      case '?':
      case '[':
      case '\\':
        *had_specials = true;
        break;
      case '/':
        return pattern;
    }
    ++pattern;
  }
  // No separator found
  return nullptr;
}

// Simple brute force lookup of pattern within a node.
// This is run at compile time and most glob sets are low enough cardinality
// that this doesn't turn out to be a hot spot in practice.
GlobTree* lookup_node_child(
    std::vector<std::unique_ptr<GlobTree>>* vec,
    const char* pattern,
    uint32_t pattern_len) {
  for (auto& kid : *vec) {
    if (kid->pattern.size() == pattern_len &&
        memcmp(kid->pattern.data(), pattern, pattern_len) == 0) {
      return kid.get();
    }
  }
  return nullptr;
}

// Compile and add a new glob pattern to the tree.
// Compilation splits a pattern into nodes, with one node for each directory
// separator separated path component.
bool add_glob(GlobTree* tree, const w_string& glob_str) {
  GlobTree* parent = tree;
  const char* pattern = glob_str.data();
  const char* pattern_end = pattern + glob_str.size();
  bool had_specials;

  if (glob_str.piece().pathIsAbsolute()) {
    throw QueryParseError(fmt::format(
        "glob `{}` is an absolute path.  All globs must be relative paths!",
        glob_str));
  }

  while (pattern < pattern_end) {
    const char* sep =
        find_sep_and_specials(pattern, pattern_end, &had_specials);
    const char* end;
    GlobTree* node;
    bool is_doublestar = false;
    auto* container = &parent->children;

    end = sep ? sep : pattern_end;

    // If a node uses double-star (recursive glob) then we take the remainder
    // of the pattern string, regardless of whether we found a separator or
    // not, because the ** forces us to walk the entire sub-tree and try the
    // match for every possible node.
    if (had_specials && end - pattern >= 2 && pattern[0] == '*' &&
        pattern[1] == '*') {
      end = pattern_end;
      is_doublestar = true;

      // Queue this up for the doublestar code path
      container = &parent->doublestar_children;
    }

    // If we can re-use an existing node, we just saved ourselves from a
    // redundant match at execution time!
    node = lookup_node_child(container, pattern, (uint32_t)(end - pattern));
    if (!node) {
      // This is a new matching possibility.
      container->emplace_back(
          make_unique<GlobTree>(pattern, (uint32_t)(end - pattern)));
      node = container->back().get();
      node->had_specials = had_specials;
      node->is_doublestar = is_doublestar;
    }

    // If we didn't find a separator in the remainder of this pattern, it
    // means that we expect it to be able to match files (it is therefore the
    // "leaf" of the pattern path).  Remember that fact as it can help us avoid
    // matching files when the pattern can only match dirs.
    if (!sep) {
      node->is_leaf = true;
    }

    pattern = end + 1; // skip separator
    parent = node; // the next iteration uses this node as its parent
  }

  return true;
}

} // namespace

void parse_globs(Query* res, const json_ref& query) {
  size_t i;

  auto globs = query.get_optional("glob");
  if (!globs) {
    return;
  }

  if (!globs->isArray()) {
    throw QueryParseError("'glob' must be an array");
  }

  // Globs implicitly enable dedup_results mode
  res->dedup_results = true;

  auto noescape = query.get_default("glob_noescape", json_false());
  if (!noescape.isBool()) {
    throw QueryParseError("glob_noescape must be a boolean");
  }

  auto includedotfiles =
      query.get_default("glob_includedotfiles", json_false());
  if (!includedotfiles.isBool()) {
    throw QueryParseError("glob_includedotfiles must be a boolean");
  }

  res->glob_flags = (includedotfiles.asBool() ? 0 : WM_PERIOD) |
      (noescape.asBool() ? WM_NOESCAPE : 0);

  res->glob_tree = make_unique<GlobTree>("", 0);
  for (i = 0; i < json_array_size(*globs); i++) {
    const auto& ele = globs->at(i);
    const auto& pattern = json_to_w_string(ele);

    if (!add_glob(res->glob_tree.get(), pattern)) {
      throw QueryParseError("failed to compile multi-glob");
    }
  }
}

static w_string parse_suffix(const json_ref& ele) {
  if (!ele.isString()) {
    throw QueryParseError("'suffix' must be a string or an array of strings");
  }

  auto str = json_to_w_string(ele);

  return str.piece().asLowerCase(str.type());
}

void parse_suffixes(Query* res, const json_ref& query) {
  auto suffixes = query.get_optional("suffix");
  if (!suffixes) {
    return;
  }

  std::vector<json_ref> suffixArray;
  if (suffixes->isString()) {
    suffixArray.push_back(*suffixes);
  } else if (suffixes->isArray()) {
    suffixArray = suffixes->array();
  } else {
    throw QueryParseError("'suffix' must be a string or an array of strings");
  }

  if (query.get_optional("glob")) {
    throw QueryParseError(
        "'suffix' cannot be used together with the 'glob' generator");
  }

  // Globs implicitly enable dedup_results mode
  res->dedup_results = true;
  // Suffix queries are defined as being case insensitive
  res->glob_flags = WM_CASEFOLD;
  res->glob_tree = make_unique<GlobTree>("", 0);

  for (auto& ele : suffixArray) {
    if (!ele.isString()) {
      throw QueryParseError("'suffix' must be a string or an array of strings");
    }

    auto suff = parse_suffix(ele);
    auto pattern = w_string::build("**/*.", suff);
    if (!add_glob(res->glob_tree.get(), pattern)) {
      throw QueryParseError("failed to compile multi-glob");
    }
  }
}

} // namespace watchman
