/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <proxygen/lib/stats/PeriodicStats.h>

#include <chrono>
#include <folly/portability/GTest.h>
#include <folly/synchronization/Baton.h>

using namespace proxygen;

template <class T>
class PeriodicStatsTestHelper {
 public:
  PeriodicStatsTestHelper(PeriodicStats<T>* psUT) {
    psUT_ = psUT;

    std::function<void()> refreshCb(
        std::bind(&PeriodicStatsTestHelper::refreshCb, this));
    psUT_->setRefreshCB(refreshCb);
  }

  struct WaitForRefreshParams {
    bool startRefresh{true};
    bool waitResult{true};

    std::chrono::milliseconds waitDuration{1000};
    std::chrono::milliseconds refreshPeriod{30000};
    std::chrono::milliseconds refreshInitialDelay{0};
  };

  // Uses folly batons to synchrously wait, as specified, for the next refresh
  // of the underlying data.
  void waitForRefresh(WaitForRefreshParams params) {
    folly::Baton<> updateCachedDataBaton;
    updateCachedDataBaton_ = &updateCachedDataBaton;
    if (params.startRefresh) {
      psUT_->refreshWithPeriod(
          std::chrono::milliseconds(params.refreshPeriod),
          std::chrono::milliseconds(params.refreshInitialDelay));
    }
    EXPECT_EQ(updateCachedDataBaton.try_wait_for(params.waitDuration),
              params.waitResult);
  }

 private:
  // Refresh cob invoked on refresh of underlying data.
  void refreshCb() {
    auto updateCachedDataBaton = updateCachedDataBaton_.exchange(nullptr);
    if (updateCachedDataBaton) {
      updateCachedDataBaton->post();
    }
  }

  // A synchronization primitive that may be used by tests to block until the
  // next refresh.
  std::atomic<folly::Baton<>*> updateCachedDataBaton_{nullptr};

  // Wrapped PeriodicStats insance under test.
  PeriodicStats<T>* psUT_;
};
