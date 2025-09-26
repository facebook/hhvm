/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "watchman/config.h" // @donotremove

#if defined(HAVE_CORESERVICES_CORESERVICES_H) && defined(__APPLE__)

#include <folly/portability/GTest.h>

#include <CoreServices/CoreServices.h> // @manual
#include <string_view>
#include "watchman/CommandRegistry.h"
#include "watchman/QueryableView.h"
#include "watchman/WatchmanConfig.h"
#include "watchman/test/lib/FakeFileSystem.h"
#include "watchman/watcher/WatcherRegistry.h"

using namespace watchman;

namespace {
Configuration getConfiguration(bool perferSplit) {
  json_ref json = json_object();
  json_object_set(
      json, "prefer_split_fsevents_watcher", json_boolean(perferSplit));
  json_object_set(json, "watcher", typed_string_to_json("auto"));
  return Configuration{std::move(json)};
}

class WatcherSelectionDarwinTest
    : public testing::TestWithParam<bool /* split */> {
 public:
  const w_string root_str{FAKEFS_ROOT "root"};
  const w_string fs_type{"apfs"};
  Configuration config = getConfiguration(GetParam());
};

#if defined(__ENVIRONMENT_MAC_OS_X_VERSION_MIN_REQUIRED__) && \
    (__ENVIRONMENT_MAC_OS_X_VERSION_MIN_REQUIRED__ < 1070)

TEST_P(WatcherSelectionDarwinTest, selectionTest) {
  if (GetParam()) {
    // prefer_split_fsevents_watcher but it isn't an option, so use kqueue.
    auto view = WatcherRegistry::initWatcher(root_str, fs_type, config);
    EXPECT_EQ("kqueue", view->getName());
  } else {
    // prefer_split_fsevents_watcher isnt an option and doen't want
    auto view = WatcherRegistry::initWatcher(root_str, fs_type, config);
    EXPECT_EQ("kqueue", view->getName());
  }
}
#else
TEST_P(WatcherSelectionDarwinTest, selectionTest) {
  if (GetParam()) {
    // prefer_split_fsevents_watcher an option and want
    auto view = WatcherRegistry::initWatcher(root_str, fs_type, config);
    EXPECT_EQ("kqueue+fsevents", view->getName());
  } else {
    // prefer_split_fsevents_watcher an option but doen't want
    auto view = WatcherRegistry::initWatcher(root_str, fs_type, config);
    EXPECT_EQ("fsevents", view->getName());
  }
}
#endif

INSTANTIATE_TEST_CASE_P(
    WatcherSelectionConfig,
    WatcherSelectionDarwinTest,
    testing::Values(true, false));

} // namespace

#endif // HAVE_CORESERVICES_CORESERVICES_H && __APPLE__
