/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "watchman/root/Root.h"
#include <folly/portability/GMock.h>
#include <folly/portability/GTest.h>

namespace {

using namespace watchman;

TEST(RootTest, IgnoreSet_includes_ignore_vcs_if_no_ignore_dirs) {
  json_ref val = json_object({
      {"ignore_vcs", json_array({w_string_to_json(".hg")})},
  });
  Configuration config(val);

  auto ignores = computeIgnoreSet(w_string{"root"}, config);
  EXPECT_TRUE(ignores.isIgnoreVCS(w_string{"root/.hg"}));
}

} // namespace
