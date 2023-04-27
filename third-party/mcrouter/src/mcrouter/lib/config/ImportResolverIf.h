/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <string>

#include <folly/Range.h>

namespace facebook {
namespace memcache {

/**
 * Used by ConfigPreprocessor. Implementation should load additional files
 * by path passed to @import macro.
 */
class ImportResolverIf {
 public:
  /**
   * @param path parameter passed to @import macro
   *
   * @return JSON with macros
   */
  virtual std::string import(folly::StringPiece path) = 0;

  virtual ~ImportResolverIf() {}
};
} // namespace memcache
} // namespace facebook
