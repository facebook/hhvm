/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <string>

#include <folly/Range.h>

#include "mcrouter/lib/config/ImportResolverIf.h"

namespace facebook {
namespace memcache {
namespace mcrouter {

class ConfigApiIf;

/**
 * ImportResolverIf implementation. Can load config files for
 * @import macro from configerator/file
 */
class McImportResolver : public ImportResolverIf {
 public:
  explicit McImportResolver(ConfigApiIf& configApi);

  /**
   * @throws std::runtime_error if can not load file
   */
  std::string import(folly::StringPiece path) override;

 private:
  ConfigApiIf& configApi_;
};
} // namespace mcrouter
} // namespace memcache
} // namespace facebook
