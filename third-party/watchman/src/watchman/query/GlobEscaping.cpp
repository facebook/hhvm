/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "watchman/query/GlobEscaping.h"
#include <string>

namespace watchman {
w_string convertLiteralPathToGlob(w_string_piece literal) {
  std::string pattern;
  pattern.reserve(literal.size());
  for (auto ch : literal.view()) {
    switch (ch) {
      case '\\':
      case '*':
      case '[':
      case '?':
        pattern.push_back('\\');
        break;
    }
    pattern.push_back(ch);
  }
  return w_string{pattern};
}

w_string convertNoEscapeGlobToGlob(w_string_piece noescapePattern) {
  std::string pattern;
  pattern.reserve(noescapePattern.size());
  for (auto ch : noescapePattern.view()) {
    switch (ch) {
      case '\\':
        pattern.push_back('\\');
        break;
    }
    pattern.push_back(ch);
  }
  return w_string{pattern};
}
} // namespace watchman
