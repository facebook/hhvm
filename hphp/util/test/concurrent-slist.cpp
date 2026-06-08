/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-present Facebook, Inc. (http://www.facebook.com)  |
   +----------------------------------------------------------------------+
   | This source file is subject to version 3.01 of the PHP license,      |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | http://www.php.net/license/3_01.txt                                  |
   | If you did not receive a copy of the PHP license and are unable to   |
   | obtain it through the world-wide-web, please send a note to          |
   | license@php.net so we can mail you a copy immediately.               |
   +----------------------------------------------------------------------+
*/

#include "hphp/util/concurrent-slist.h"

#include <mutex>
#include <thread>
#include <vector>

#include <folly/portability/GTest.h>

using namespace HPHP;

namespace {

struct WorkItem : ConcurrentSListNode {
  int producer;
  int seq;
  bool in_list{false};
};

} // namespace

TEST(ConcurrentSList, BasicInsertAndIterate) {
  ConcurrentSList<WorkItem> list;
  WorkItem a, b, c;
  a.seq = 1;
  b.seq = 2;
  c.seq = 3;

  list.insert_head(a);
  list.insert_head(b);
  list.insert_head(c);

  std::vector<int> seen;
  auto it = list.begin();
  while (it.valid()) {
    seen.push_back(it.data().seq);
    it.advance();
  }

  // insert_head puts newest first
  EXPECT_EQ(seen, (std::vector<int>{3, 2, 1}));
}

TEST(ConcurrentSList, SelectiveRemoval) {
  ConcurrentSList<WorkItem> list;
  WorkItem items[5];
  for (int i = 0; i < 5; ++i) {
    items[i].seq = i;
    items[i].producer = i % 2;
    list.insert_head(items[i]);
  }

  // Remove only even-producer items
  auto it = list.begin();
  while (it.valid()) {
    if (it.data().producer == 0) {
      it.advance(true);
    } else {
      it.advance();
    }
  }

  std::vector<int> remaining;
  it = list.begin();
  while (it.valid()) {
    remaining.push_back(it.data().seq);
    it.advance();
  }

  // Only odd-producer items (seq 1, 3) remain, in reverse insertion order
  EXPECT_EQ(remaining, (std::vector<int>{3, 1}));
}

TEST(ConcurrentSList, RemoveHead) {
  ConcurrentSList<WorkItem> list;
  WorkItem a, b;
  a.seq = 1;
  b.seq = 2;

  list.insert_head(a);
  list.insert_head(b);

  // Remove the head (b)
  auto it = list.begin();
  EXPECT_EQ(it.data().seq, 2);
  it.advance(true);
  EXPECT_TRUE(it.valid());
  EXPECT_EQ(it.data().seq, 1);
  it.advance();
  EXPECT_FALSE(it.valid());

  // Only a remains
  it = list.begin();
  EXPECT_TRUE(it.valid());
  EXPECT_EQ(it.data().seq, 1);
  it.advance();
  EXPECT_FALSE(it.valid());
}

TEST(ConcurrentSList, RemoveAll) {
  ConcurrentSList<WorkItem> list;
  WorkItem a, b, c;
  a.seq = 1;
  b.seq = 2;
  c.seq = 3;

  list.insert_head(a);
  list.insert_head(b);
  list.insert_head(c);

  auto it = list.begin();
  while (it.valid()) {
    it.advance(true);
  }

  it = list.begin();
  EXPECT_FALSE(it.valid());
}

TEST(ConcurrentSList, ConcurrentStress) {
  static constexpr int kThreads = 8;
  static constexpr int kItemsPerThread = 500'000;
  static constexpr int kBatchSize = 128;

  ConcurrentSList<WorkItem> list;
  std::mutex walk_mtx;

  std::vector<WorkItem> items(kThreads * kItemsPerThread);
  int removed_per_thread[kThreads] = {};

  std::vector<std::thread> threads;
  for (int p = 0; p < kThreads; ++p) {
    threads.emplace_back([&, p] {
      int base = p * kItemsPerThread;
      int next_insert = 0;
      int my_removed = 0;

      while (my_removed < kItemsPerThread) {
        int batch_end = next_insert + kBatchSize;
        if (batch_end > kItemsPerThread) batch_end = kItemsPerThread;

        for (int i = next_insert; i < batch_end; ++i) {
          auto& item = items[base + i];
          item.producer = p;
          item.seq = i;
          item.in_list = true;
          list.insert_head(item);
        }
        next_insert = batch_end;

        {
          std::lock_guard<std::mutex> lock(walk_mtx);
          auto it = list.begin();
          while (it.valid()) {
            if (it.data().producer == p) {
              it.data().in_list = false;
              it.advance(true);
              my_removed++;
            } else {
              it.advance();
            }
          }
        }
      }

      removed_per_thread[p] = my_removed;
    });
  }

  for (auto& t : threads) t.join();

  int total_removed = 0;
  for (int p = 0; p < kThreads; ++p) {
    total_removed += removed_per_thread[p];
  }

  int still_in_list = 0;
  auto it = list.begin();
  while (it.valid()) {
    still_in_list++;
    it.advance();
  }

  int total = kThreads * kItemsPerThread;
  EXPECT_EQ(total_removed, total);
  EXPECT_EQ(still_in_list, 0);
}
