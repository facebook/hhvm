/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "watchman/query/FileResult.h"

namespace watchman {

FileResult::~FileResult() {}

std::optional<DType> FileResult::dtype() {
  auto statInfo = stat();
  if (!statInfo.has_value()) {
    return std::nullopt;
  }
  return statInfo->dtype();
}

} // namespace watchman
