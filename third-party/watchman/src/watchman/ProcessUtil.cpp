/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "watchman/ProcessUtil.h"

#include "eden/common/utils/ProcessInfoCache.h"

namespace watchman {

using namespace facebook::eden;

namespace {
ProcessInfoCache& getProcessInfoCache() {
  static auto* pic = new ProcessInfoCache;
  return *pic;
}
} // namespace

ProcessInfoHandle lookupProcessInfo(pid_t pid) {
  return getProcessInfoCache().lookup(pid);
}

} // namespace watchman
