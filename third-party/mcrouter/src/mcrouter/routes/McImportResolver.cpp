/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "McImportResolver.h"

#include "mcrouter/ConfigApiIf.h"
#include "mcrouter/lib/config/ImportResolverIf.h"

namespace facebook {
namespace memcache {
namespace mcrouter {

McImportResolver::McImportResolver(ConfigApiIf& configApi)
    : configApi_(configApi) {}

std::string McImportResolver::import(folly::StringPiece path) {
  std::string ret;
  if (!configApi_.get(ConfigType::ConfigImport, path.str(), ret)) {
    throw std::runtime_error("Can not read " + path.str());
  }
  return ret;
}
} // namespace mcrouter
} // namespace memcache
} // namespace facebook
