/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <folly/executors/ManualExecutor.h>
#include <folly/portability/GTest.h>
#include <chrono>
#include "watchman/InMemoryView.h"
#include "watchman/fs/FSDetect.h"
#include "watchman/query/GlobTree.h"
#include "watchman/query/Query.h"
#include "watchman/root/Root.h"
#include "watchman/test/lib/FakeFileSystem.h"
#include "watchman/test/lib/FakeWatcher.h"
#include "watchman/watcher/Watcher.h"
#include "watchman/watchman_file.h"

namespace {

using namespace watchman;
using namespace std::literals::chrono_literals;
using namespace ::testing;

Configuration getConfiguration() {
  json_ref json = json_object();
  json_object_set(json, "enable_parallel_crawl", json_boolean(true));
  json_object_set(json, "inject_block_in_io_thread_start", json_boolean(true));
  return Configuration{std::move(json)};
}

class FailsToStartViewTest : public Test {
 public:
  using Continue = InMemoryView::Continue;

  const w_string root_path{FAKEFS_ROOT "root"};

  FakeFileSystem fs;
  Configuration config = getConfiguration();
  std::shared_ptr<FakeWatcher> watcher =
      std::make_shared<FakeWatcher>(fs, true);

  std::shared_ptr<InMemoryView> view =
      std::make_shared<InMemoryView>(fs, root_path, config, watcher);
  PendingCollection& pending = view->unsafeAccessPendingFromWatcher();

  FailsToStartViewTest() {
    pending.lock()->ping();
  }
};

TEST_F(FailsToStartViewTest, can_start) {
  fs.defineContents({
      FAKEFS_ROOT "root",
  });

  auto root = std::make_shared<Root>(
      fs, root_path, "fs_type", w_string_to_json("{}"), config, view, [] {});

  root->view()->startThreads(root);
  ASSERT_THROW(
      root->view()->waitUntilReadyToQuery().get(5000ms), std::runtime_error);
}

} // namespace
