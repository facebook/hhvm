/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "watchman/query/Query.h"
#include "watchman/query/GlobTree.h"
#include "watchman/query/QueryExpr.h"

namespace watchman {

Query::~Query() = default;

bool Query::isFieldRequested(w_string_piece name) const {
  for (auto& f : fieldList) {
    if (f->name.piece() == name) {
      return true;
    }
  }
  return false;
}

} // namespace watchman
