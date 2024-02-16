/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "watchman/cppclient/WatchmanClient.h"

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <mutex>
#include <thread>

#include <folly/experimental/TestUtil.h>
#include <folly/experimental/io/FsUtil.h>
#include <folly/init/Init.h>
#include <folly/io/async/EventBaseThread.h>
#include <folly/json/json.h>
#include <glog/logging.h>

using namespace folly;
using namespace watchman;
using namespace std::chrono;

int main(int argc, char** argv) {
  folly::init(&argc, &argv);

  system("rm -f hit");

  folly::EventBaseThread ebt;
  auto eb = ebt.getEventBase();

  Promise<Unit> errorCallbackTrigger;
  WatchmanClient c(
      eb, std::nullopt, nullptr, [&errorCallbackTrigger](exception_wrapper&) {
        LOG(INFO) << "Expected global error caught";
        errorCallbackTrigger.setValue();
      });
  c.connect().get();
  LOG(INFO) << "Connected to watchman";

  std::mutex mutex;
  std::condition_variable cv;
  std::atomic_bool hit(false);

  auto current_dir = fs::current_path().string();
  WatchPathPtr current_dir_ptr = c.watch(current_dir).get();
  dynamic query = dynamic::object("fields", dynamic::array("name"))(
      "expression", dynamic::array("name", "hit"));
  auto sub = c.subscribe(
                  query,
                  current_dir,
                  eb,
                  [&](Try<dynamic>&& data) {
                    // Skip "state-enter" and "state-leave" updates that
                    // don't describe filesystem changes
                    if (data->get_ptr("files") == nullptr) {
                      return;
                    }
                    if ((*data)["is_fresh_instance"].getBool()) {
                      return;
                    } else {
                      if ((*data)["files"][0].getString().find("hit") !=
                          std::string::npos) {
                        LOG(INFO) << "Got hit";
                        std::unique_lock<std::mutex> lock(mutex);
                        hit = true;
                        cv.notify_all();
                      }
                    }
                  })
                 .wait()
                 .value();

  {
    // By creating a subscription above, we ensured that Watchman is already
    // watching the current directory. Now create and watch a subdirectory.
    // Watchman will reuse the watch root, but return results as though it were
    // watching only the subdirectory.
    LOG(INFO) << "Testing relative paths";
    auto subdir = folly::test::TemporaryDirectory{
        /*namePrefix=*/"",
        /*dir=*/current_dir};
    LOG(INFO) << "Created " << subdir.path();
    auto empty_file_path = subdir.path() / "empty_file";
    auto empty_file_relative_path =
        fs::relative(empty_file_path, subdir.path());
    {
      auto empty_file = std::ofstream{empty_file_path};
      if (!empty_file) {
        LOG(ERROR) << "Failed to create " << empty_file_path;
        return 1;
      }
      LOG(INFO) << "Created " << empty_file_path;
    }
    dynamic relative_query = dynamic::object("fields", dynamic::array("name"))(
        "expression",
        dynamic::array("name", empty_file_relative_path.string()));
    auto subdir_ptr = c.watch(subdir.path().string()).get();
    auto result = c.query(relative_query, subdir_ptr).get();
    if (result.raw_["files"].empty()) {
      LOG(ERROR) << "FAIL: No files found in " << folly::toJson(result.raw_);
      return 1;
    }
    if (result.raw_["files"][0].getString() != empty_file_relative_path) {
      LOG(ERROR) << "FAIL: Expected a file named " << empty_file_relative_path
                 << ", got " << fs::path{result.raw_["files"][0].getString()}
                 << " in " << folly::toJson(result.raw_);
      return 1;
    }
    LOG(INFO) << "PASS: Watchman returned " << empty_file_relative_path
              << " and not " << fs::relative(empty_file_path, current_dir);
  }

  LOG(INFO) << "Triggering subscription";
  auto clock_before_hit = c.getClock(current_dir_ptr).get();
  system("touch hit");
  LOG(INFO) << "Waiting for hit.";
  std::unique_lock<std::mutex> lock(mutex);
  auto now = std::chrono::system_clock::now();
  if (!cv.wait_until(lock, now + seconds(5), [&]() { return (bool)hit; })) {
    LOG(ERROR) << "FAIL: timeout/no hit";
    return 1;
  }
  hit = false;

  LOG(INFO) << "Testing one-off query";
  auto data =
      c.query(
           dynamic::object("expression", dynamic::array("name", "hit"))(
               "fields", dynamic::array("name"))("since", clock_before_hit),
           current_dir_ptr)
          .get();
  if (data.raw_["files"][0].getString().find("hit") == std::string::npos) {
    LOG(ERROR) << "FAIL: one-off query missed the hit file";
    return 1;
  } else {
    LOG(INFO) << "PASS: one-off query saw the touched hit file";
  }

  LOG(INFO) << "Flushing subscription";
  auto flush_res =
      c.flushSubscription(sub, std::chrono::milliseconds(1000)).wait().value();
  if (flush_res.find("no_sync_needed") == flush_res.items().end() ||
      !flush_res.find("no_sync_needed")->second.isArray() ||
      !(flush_res.find("no_sync_needed")->second.size() == 1) ||
      !(flush_res.find("no_sync_needed")->second[0] == "sub1")) {
    LOG(ERROR) << "FAIL: unexpected flush result " << toJson(flush_res);
    return 1;
  }
  LOG(INFO) << "PASS: flush response looks okay";

  LOG(INFO) << "Unsubscribing";
  c.unsubscribe(sub).wait();
  LOG(INFO) << "Trying to falsely trigger subscription";
  system("rm hit");
  /* sleep override */ std::this_thread::sleep_for(std::chrono::seconds(3));
  if (hit) {
    LOG(ERROR) << "FAIL: still got a hit";
    return 1;
  }
  LOG(INFO) << "PASS: didn't see false trigger after 3 seconds";

  LOG(INFO) << "Testing error handling";
  Promise<Unit> subErrorCallbackTrigger;
  c.subscribe(
       query,
       current_dir,
       eb,
       [&](folly::Try<dynamic>&& data) {
         if (data.hasException()) {
           LOG(INFO) << "Expected subcription error caught";
           subErrorCallbackTrigger.setValue();
         }
       })
      .wait()
      .value();
  c.getConnection().forceEOF();
  try {
    errorCallbackTrigger.getFuture().within(seconds(1)).wait().value();
  } catch (FutureTimeout&) {
    LOG(ERROR) << "FAIL: did not get callback from global error handler";
    return 1;
  }
  try {
    subErrorCallbackTrigger.getFuture().within(seconds(1)).wait().value();
  } catch (FutureTimeout&) {
    LOG(ERROR) << "FAIL: did not get subscription error";
    return 1;
  }
  LOG(INFO) << "PASS: caught expected errors";

  return 0;
}
