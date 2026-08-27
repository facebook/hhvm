/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <atomic>
#include <chrono>
#include <future>
#include <memory>
#include <thread>

#include <gtest/gtest.h>

#include <folly/ScopeGuard.h>

#include "mcrouter/CarbonRouterInstance.h"
// defaultTestOptions(); its own header does not build when included directly.
#include "mcrouter/options.h"
#include "mcrouter/routes/McrouterRouteHandle.h"

using namespace facebook::memcache;
using namespace facebook::memcache::mcrouter;

namespace {

// One thread to hold hostage, one to observe.
constexpr size_t kNumProxies = 2;

// create(), not init(): each test must own and tear down its own router.
std::shared_ptr<CarbonRouterInstance<McrouterRouterInfo>> makeRouter(
    size_t numProxies) {
  McrouterOptions opts = defaultTestOptions();
  opts.config = "{ \"route\": \"NullRoute\" }";
  opts.num_proxies = numProxies;
  return CarbonRouterInstance<McrouterRouterInfo>::create(std::move(opts));
}

} // namespace

// Hold proxy 1's thread hostage and check what a hop resolved from it sees.
// Teardown stalls on that thread either way, so the task observes the state
// teardown reached before it needed proxy 1.
TEST(CarbonRouterInstanceShutdownTest, ShutdownDrainsProxiesBeforeFreeingThem) {
  auto router = makeRouter(kNumProxies);
  ASSERT_NE(router, nullptr);
  ASSERT_NE(router->getProxy(1), nullptr);

  // Index arithmetic only; off a proxy thread, so do not copy this shape.
  for (size_t hash = 0; hash < 2 * kNumProxies; ++hash) {
    EXPECT_EQ(
        router->getProxyFromHash(hash), router->getProxy(hash % kNumProxies));
  }

  std::promise<void> taskRunning;
  std::future<void> taskRunningFuture = taskRunning.get_future();
  std::promise<bool> observed;
  std::future<bool> observedFuture = observed.get_future();

  router->getProxy(1)->eventBase().getEventBase().runInEventBaseThread(
      [&router, &taskRunning, &observed] {
        taskRunning.set_value();
        bool draining = false;
        // Bounded so a regression fails instead of pinning this thread.
        for (int i = 0; i < 1000 && !draining; ++i) {
          draining = router->getProxyFromHash(0) == nullptr;
          if (!draining) {
            /* sleep override */
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
          }
        }
        observed.set_value(draining);
      });

  taskRunningFuture.wait();

  std::thread shutdownThread([&router] { router->shutdown(); });
  SCOPE_EXIT {
    shutdownThread.join();
  };

  EXPECT_TRUE(observedFuture.get())
      << "shutdown() began tearing proxies down without first marking them as "
         "draining, so a thread jump could still resolve a proxy that is "
         "about to be freed";
}

// No proxy may be destroyed while any proxy thread is still running: a hop
// that resolved a target just before the flag flipped is still inline on its
// own thread. Occupy proxy 1 and watch proxy 0.
TEST(CarbonRouterInstanceShutdownTest, ShutdownFencesBeforeDestroyingAnyProxy) {
  auto router = makeRouter(kNumProxies);
  ASSERT_NE(router, nullptr);
  ASSERT_NE(router->getProxy(0), nullptr);
  ASSERT_NE(router->getProxy(1), nullptr);

  std::atomic<bool> proxy0Destroyed{false};
  router->getProxy(0)->eventBase().runOnDestruction(
      [&proxy0Destroyed] { proxy0Destroyed.store(true); });

  std::promise<void> taskRunning;
  std::future<void> taskRunningFuture = taskRunning.get_future();
  std::promise<void> release;
  std::shared_future<void> releaseFuture = release.get_future();

  router->getProxy(1)->eventBase().getEventBase().runInEventBaseThread(
      [&taskRunning, releaseFuture] {
        taskRunning.set_value();
        releaseFuture.wait();
      });
  taskRunningFuture.wait();

  std::thread shutdownThread([&router] { router->shutdown(); });

  // Fallback for early returns; the happy path joins inline below.
  bool released = false;
  SCOPE_EXIT {
    if (!released) {
      release.set_value();
      shutdownThread.join();
    }
  };

  // Bounded negative wait; can only pass spuriously, never fail spuriously.
  // Kept short: this pins a proxy thread and other tests in this binary are
  // latency-sensitive. Without the fence, clear() frees proxy 0 immediately.
  /* sleep override */
  std::this_thread::sleep_for(std::chrono::milliseconds(150));
  EXPECT_FALSE(proxy0Destroyed.load())
      << "shutdown() destroyed a proxy while another proxy thread was still "
         "running, so a hop that had already resolved that proxy as its target "
         "could still be about to use it";

  released = true;
  release.set_value();
  shutdownThread.join();
  EXPECT_TRUE(proxy0Destroyed.load()); // not vacuous
}
