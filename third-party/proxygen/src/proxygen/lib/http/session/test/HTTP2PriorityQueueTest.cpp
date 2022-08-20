/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <list>
#include <map>
#include <thread>

#include <folly/Random.h>
#include <folly/io/async/test/MockTimeoutManager.h>
#include <folly/io/async/test/UndelayedDestruction.h>
#include <folly/portability/GTest.h>
#include <proxygen/lib/http/session/HTTP2PriorityQueue.h>

using namespace std::placeholders;
using namespace testing;
using folly::HHWheelTimer;
using folly::Random;
using folly::test::MockTimeoutManager;

namespace {
static char* fakeTxn = (char*)0xface0000;

const proxygen::HTTPCodec::StreamID kRootNodeId =
    std::numeric_limits<uint64_t>::max();

proxygen::HTTPTransaction* makeFakeTxn(proxygen::HTTPCodec::StreamID id) {
  return (proxygen::HTTPTransaction*)(fakeTxn + id);
}

proxygen::HTTPCodec::StreamID getTxnID(proxygen::HTTPTransaction* txn) {
  return (proxygen::HTTPCodec::StreamID)((char*)txn - fakeTxn);
}

// folly::Random::rand32 is broken because it takes RNG by value
uint32_t rand32(uint32_t max, folly::Random::DefaultGenerator& rng) {
  if (max == 0) {
    return 0;
  }

  return std::uniform_int_distribution<uint32_t>(0, max - 1)(rng);
}

} // namespace

namespace proxygen {

using IDList = std::list<std::pair<HTTPCodec::StreamID, uint8_t>>;

class QueueTest : public testing::Test {
 public:
  explicit QueueTest(HHWheelTimer* timer = nullptr)
      : q_(WheelTimerInstance(timer), kRootNodeId) {
  }

 protected:
  void addTransaction(HTTPCodec::StreamID id,
                      http2::PriorityUpdate pri,
                      bool pnode = false,
                      uint64_t* depth = nullptr) {
    HTTP2PriorityQueue::Handle h = q_.addTransaction(
        id, pri, pnode ? nullptr : makeFakeTxn(id), false, depth);
    // Blow away the old handle.  Hopefully the caller knows what they are doing
    handles_.erase(id);
    handles_.insert(std::make_pair(id, h));
    if (!pnode) {
      signalEgress(id, 1);
    }
  }

  void removeTransaction(HTTPCodec::StreamID id) {
    q_.removeTransaction(handles_[id]);
  }

  void updatePriority(HTTPCodec::StreamID id,
                      http2::PriorityUpdate pri,
                      uint64_t* depth = nullptr) {
    handles_[id] = q_.updatePriority(handles_[id], pri, depth);
  }

  void signalEgress(HTTPCodec::StreamID id, bool mark) {
    if (mark) {
      q_.signalPendingEgress(handles_[id]);
    } else {
      q_.clearPendingEgress(handles_[id]);
    }
  }

  void buildSimpleTree() {
    addTransaction(0, {kRootNodeId, false, 15});
    addTransaction(3, {0, false, 3});
    addTransaction(5, {0, false, 3});
    addTransaction(7, {0, false, 7});
    addTransaction(9, {5, false, 7});
  }

  bool visitNode(HTTP2PriorityQueue&,
                 HTTPCodec::StreamID id,
                 HTTPTransaction*,
                 double r) {
    nodes_.push_back(std::make_pair(id, r * 100));
    return false;
  }

  void dump() {
    nodes_.clear();
    q_.iterate(
        std::bind(&QueueTest::visitNode, this, std::ref(q_), _1, _2, _3),
        [] { return false; },
        true);
  }

  void dumpBFS(const std::function<bool()>& stopFn) {
    nodes_.clear();
    q_.iterateBFS(
        std::bind(&QueueTest::visitNode, this, _1, _2, _3, _4), stopFn, true);
  }

  void nextEgress(bool spdyMode = false) {
    HTTP2PriorityQueue::NextEgressResult nextEgressResults;
    q_.nextEgress(nextEgressResults, spdyMode);
    nodes_.clear();
    for (auto p : nextEgressResults) {
      nodes_.push_back(std::make_pair(getTxnID(p.first), p.second * 100));
    }
  }

  HTTP2PriorityQueue q_;
  std::map<HTTPCodec::StreamID, HTTP2PriorityQueue::Handle> handles_;
  IDList nodes_;
};

TEST_F(QueueTest, Basic) {
  buildSimpleTree();
  dump();
  EXPECT_EQ(nodes_, IDList({{0, 100}, {3, 25}, {5, 25}, {9, 100}, {7, 50}}));

  // Add another node, make sure we get the correct depth.
  uint64_t depth;
  addTransaction(11, {7, false, 15}, false, &depth);
  EXPECT_EQ(depth, 3);
}

TEST_F(QueueTest, RemoveLeaf) {
  buildSimpleTree();

  removeTransaction(3);
  dump();

  EXPECT_EQ(nodes_, IDList({{0, 100}, {5, 33}, {9, 100}, {7, 66}}));
}

TEST_F(QueueTest, RemoveParent) {
  buildSimpleTree();

  removeTransaction(5);
  dump();

  EXPECT_EQ(nodes_, IDList({{0, 100}, {3, 25}, {7, 50}, {9, 25}}));
}

TEST_F(QueueTest, RemoveParentWeights) {
  // weight_ / totalChildWeight_ < 1
  addTransaction(0, {kRootNodeId, false, 0});
  addTransaction(3, {0, false, 255});
  addTransaction(5, {0, false, 255});

  removeTransaction(0);
  dump();

  EXPECT_EQ(nodes_, IDList({{3, 50}, {5, 50}}));
}

TEST_F(QueueTest, NodeDepth) {
  uint64_t depth{33}; // initialize to some wrong value
  addTransaction(0, {kRootNodeId, false, 15}, false, &depth);
  EXPECT_EQ(depth, 1);

  addTransaction(3, {0, false, 3}, false, &depth);
  EXPECT_EQ(depth, 2);

  addTransaction(5, {3, true, 7}, false, &depth);
  EXPECT_EQ(depth, 3);

  addTransaction(9, {0, false, 3}, true, &depth);
  EXPECT_EQ(depth, 2);
  EXPECT_EQ(q_.numPendingEgress(), 3);
  EXPECT_EQ(q_.numVirtualNodes(), 1);

  depth = 55; // some unlikely depth
  addTransaction(9, {0, false, 31}, false, &depth);
  EXPECT_EQ(depth, 2);
  EXPECT_EQ(q_.numPendingEgress(), 4);
  EXPECT_EQ(q_.numVirtualNodes(), 0);

  addTransaction(11, {0, true, 7}, false, &depth);
  EXPECT_EQ(depth, 2);
  EXPECT_EQ(q_.numPendingEgress(), 5);
  EXPECT_EQ(q_.numVirtualNodes(), 0);

  addTransaction(13, {kRootNodeId, true, 23}, true, &depth);
  EXPECT_EQ(depth, 1);
  EXPECT_EQ(q_.numPendingEgress(), 5);
  EXPECT_EQ(q_.numVirtualNodes(), 1);

  depth = 77; // some unlikely depth
  addTransaction(13, {kRootNodeId, true, 33}, false, &depth);
  EXPECT_EQ(depth, 1);
  EXPECT_EQ(q_.numPendingEgress(), 6);
  EXPECT_EQ(q_.numVirtualNodes(), 0);
}

TEST_F(QueueTest, UpdateWeight) {
  buildSimpleTree();

  uint64_t depth = 0;
  updatePriority(5, {0, false, 7}, &depth);
  dump();

  EXPECT_EQ(nodes_, IDList({{0, 100}, {3, 20}, {5, 40}, {9, 100}, {7, 40}}));
  EXPECT_EQ(depth, 2);
}

// Previously the code would allow duplicate entries in the priority tree under
// certain circumstances.
TEST_F(QueueTest, DuplicateID) {
  q_.addOrUpdatePriorityNode(0, {kRootNodeId, false, 15});
  addTransaction(0, {kRootNodeId, true, 15});
  q_.addOrUpdatePriorityNode(3, {0, false, 15});
  addTransaction(5, {3, false, 15});
  addTransaction(3, {5, false, 15});
  removeTransaction(5);
  auto stopFn = [] { return false; };

  dumpBFS(stopFn);
  EXPECT_EQ(nodes_, IDList({{0, 100}, {3, 100}}));
}

TEST_F(QueueTest, UpdateWeightNotEnqueued) {
  addTransaction(0, {kRootNodeId, false, 7});
  addTransaction(3, {0, false, 7});

  signalEgress(0, false);
  signalEgress(3, false);
  uint64_t depth = 0;
  updatePriority(0, {3, false, 7}, &depth);
  dump();

  EXPECT_EQ(nodes_, IDList({{3, 100}, {0, 100}}));
  EXPECT_EQ(depth, 2);
}

TEST_F(QueueTest, UpdateWeightExcl) {
  buildSimpleTree();

  updatePriority(5, {0, true, 7});
  dump();

  EXPECT_EQ(nodes_, IDList({{0, 100}, {5, 100}, {9, 40}, {3, 20}, {7, 40}}));
  signalEgress(0, false);
  nextEgress();
  EXPECT_EQ(nodes_, IDList({{5, 100}}));
}

TEST_F(QueueTest, UpdateWeightExclDequeued) {
  buildSimpleTree();

  signalEgress(5, false);
  updatePriority(5, {0, true, 7});
  signalEgress(0, false);
  nextEgress();

  EXPECT_EQ(nodes_, IDList({{9, 40}, {7, 40}, {3, 20}}));
}

TEST_F(QueueTest, UpdateWeightUnknownParent) {
  buildSimpleTree();

  uint64_t depth = 0;
  updatePriority(5, {97, false, 15}, &depth);
  dump();

  EXPECT_EQ(nodes_,
            IDList({{0, 50}, {3, 33}, {7, 66}, {97, 50}, {5, 100}, {9, 100}}));
  EXPECT_EQ(depth, 2);

  depth = 0;
  updatePriority(9, {99, false, 15}, &depth);
  dump();

  EXPECT_EQ(
      nodes_,
      IDList(
          {{0, 33}, {3, 33}, {7, 66}, {97, 33}, {5, 100}, {99, 33}, {9, 100}}));
  EXPECT_EQ(depth, 2);
}

TEST_F(QueueTest, UpdateParentSibling) {
  buildSimpleTree();

  updatePriority(5, {3, false, 3});
  dump();

  EXPECT_EQ(nodes_, IDList({{0, 100}, {3, 33}, {5, 100}, {9, 100}, {7, 66}}));
  signalEgress(0, false);
  nextEgress();
  EXPECT_EQ(nodes_, IDList({{7, 66}, {3, 33}}));

  // Clear 5's egress (so it is only in the tree because 9 has egress) and move
  // it back.  Hit's a slightly different code path in reparent
  signalEgress(5, false);
  updatePriority(5, {0, false, 3});
  dump();

  EXPECT_EQ(nodes_, IDList({{0, 100}, {3, 25}, {7, 50}, {5, 25}, {9, 100}}));

  nextEgress();
  EXPECT_EQ(nodes_, IDList({{7, 50}, {3, 25}, {9, 25}}));
}

TEST_F(QueueTest, UpdateParentSiblingExcl) {
  buildSimpleTree();

  updatePriority(7, {5, true, 3});
  dump();

  EXPECT_EQ(nodes_, IDList({{0, 100}, {3, 50}, {5, 50}, {7, 100}, {9, 100}}));
  signalEgress(0, false);
  signalEgress(3, false);
  signalEgress(5, false);
  nextEgress();
  EXPECT_EQ(nodes_, IDList({{7, 100}}));
}

TEST_F(QueueTest, UpdateParentAncestor) {
  buildSimpleTree();

  updatePriority(9, {kRootNodeId, false, 15});
  dump();

  EXPECT_EQ(nodes_, IDList({{0, 50}, {3, 25}, {5, 25}, {7, 50}, {9, 50}}));
  nextEgress();
  EXPECT_EQ(nodes_, IDList({{0, 50}, {9, 50}}));
}

TEST_F(QueueTest, UpdateParentAncestorExcl) {
  buildSimpleTree();

  updatePriority(9, {kRootNodeId, true, 15});
  dump();

  EXPECT_EQ(nodes_, IDList({{9, 100}, {0, 100}, {3, 25}, {5, 25}, {7, 50}}));
  nextEgress();
  EXPECT_EQ(nodes_, IDList({{9, 100}}));
}

TEST_F(QueueTest, UpdateParentDescendant) {
  buildSimpleTree();

  updatePriority(0, {5, false, 7});
  dump();

  EXPECT_EQ(nodes_, IDList({{5, 100}, {9, 50}, {0, 50}, {3, 33}, {7, 66}}));
  nextEgress();
  EXPECT_EQ(nodes_, IDList({{5, 100}}));
  signalEgress(5, false);
  nextEgress();
  EXPECT_EQ(nodes_, IDList({{9, 50}, {0, 50}}));
}

TEST_F(QueueTest, UpdateParentDescendantExcl) {
  buildSimpleTree();

  updatePriority(0, {5, true, 7});
  dump();

  EXPECT_EQ(nodes_, IDList({{5, 100}, {0, 100}, {3, 20}, {7, 40}, {9, 40}}));
  nextEgress();
  EXPECT_EQ(nodes_, IDList({{5, 100}}));
  signalEgress(5, false);
  signalEgress(0, false);
  nextEgress();
  EXPECT_EQ(nodes_, IDList({{7, 40}, {9, 40}, {3, 20}}));
}

TEST_F(QueueTest, ExclusiveAdd) {
  buildSimpleTree();

  addTransaction(11, {0, true, 100});

  dump();
  EXPECT_EQ(nodes_,
            IDList({{0, 100}, {11, 100}, {3, 25}, {5, 25}, {9, 100}, {7, 50}}));
}

TEST_F(QueueTest, AddUnknown) {
  buildSimpleTree();

  addTransaction(11, {75, false, 15});

  dump();
  EXPECT_EQ(
      nodes_,
      IDList(
          {{0, 50}, {3, 25}, {5, 25}, {9, 100}, {7, 50}, {75, 50}, {11, 100}}));

  // Now let's add the missing parent node and check if it was
  // relocated properly
  addTransaction(75, {0, false, 7});

  dump();
  EXPECT_EQ(nodes_,
            IDList({{0, 100},
                    {3, 16},
                    {5, 16},
                    {9, 100},
                    {7, 33},
                    {75, 33},
                    {11, 100}}));
}

TEST_F(QueueTest, AddMax) {
  addTransaction(0, {kRootNodeId, false, 255});

  nextEgress();
  EXPECT_EQ(nodes_, IDList({{0, 100}}));
}

TEST_F(QueueTest, Misc) {
  buildSimpleTree();

  EXPECT_FALSE(q_.empty());
  EXPECT_EQ(q_.numPendingEgress(), 5);
  signalEgress(0, false);
  EXPECT_EQ(q_.numPendingEgress(), 4);
  EXPECT_FALSE(q_.empty());
  removeTransaction(9);
  removeTransaction(0);
  dump();
  EXPECT_EQ(nodes_, IDList({{3, 25}, {5, 25}, {7, 50}}));
}

TEST_F(QueueTest, IterateBFS) {
  buildSimpleTree();

  auto stopFn = [this] { return nodes_.size() > 2; };

  dumpBFS(stopFn);
  EXPECT_EQ(nodes_, IDList({{0, 100}, {3, 25}, {5, 25}, {7, 50}}));
}

TEST_F(QueueTest, NextEgress) {
  buildSimpleTree();

  nextEgress();
  EXPECT_EQ(nodes_, IDList({{0, 100}}));

  addTransaction(11, {7, false, 15});
  signalEgress(0, false);

  nextEgress();
  EXPECT_EQ(nodes_, IDList({{7, 50}, {3, 25}, {5, 25}}));

  signalEgress(5, false);
  nextEgress();
  EXPECT_EQ(nodes_, IDList({{7, 50}, {3, 25}, {9, 25}}));
  signalEgress(5, true);

  signalEgress(3, false);
  nextEgress();
  EXPECT_EQ(nodes_, IDList({{7, 66}, {5, 33}}));

  signalEgress(5, false);
  nextEgress();
  EXPECT_EQ(nodes_, IDList({{7, 66}, {9, 33}}));

  signalEgress(7, false);
  nextEgress();
  EXPECT_EQ(nodes_, IDList({{11, 66}, {9, 33}}));

  signalEgress(9, false);
  nextEgress();
  EXPECT_EQ(nodes_, IDList({{11, 100}}));

  signalEgress(3, true);
  signalEgress(7, true);
  signalEgress(9, true);
  nextEgress();
  EXPECT_EQ(nodes_, IDList({{7, 50}, {3, 25}, {9, 25}}));
}

TEST_F(QueueTest, NextEgressExclusiveAdd) {
  buildSimpleTree();

  // clear all egress
  signalEgress(0, false);
  signalEgress(3, false);
  signalEgress(5, false);
  signalEgress(7, false);
  signalEgress(9, false);

  // Add a transaction with exclusive dependency, clear its egress
  addTransaction(11, {0, true, 100});
  signalEgress(11, false);

  // signal egress for a child that got moved via exclusive dep
  signalEgress(3, true);
  nextEgress();
  EXPECT_EQ(nodes_, IDList({{3, 100}}));
  EXPECT_EQ(q_.numPendingEgress(), 1);
}

TEST_F(QueueTest, NextEgressExclusiveAddWithEgress) {
  buildSimpleTree();

  // clear all egress, except 3
  signalEgress(0, false);
  signalEgress(5, false);
  signalEgress(7, false);
  signalEgress(9, false);

  // Add a transaction with exclusive dependency, clear its egress
  addTransaction(11, {0, true, 100});
  signalEgress(11, false);
  nextEgress();
  EXPECT_EQ(nodes_, IDList({{3, 100}}));
  EXPECT_EQ(q_.numPendingEgress(), 1);
}

TEST_F(QueueTest, UpdatePriorityReparentSubtree) {
  buildSimpleTree();

  // clear all egress, except 9
  signalEgress(0, false);
  signalEgress(3, false);
  signalEgress(5, false);
  signalEgress(7, false);

  // Update priority of non-enqueued but in egress tree node
  updatePriority(5, {0, false, 14}, nullptr);

  // update 9's weight and reparent
  updatePriority(9, {3, false, 14}, nullptr);

  nextEgress();
  EXPECT_EQ(nodes_, IDList({{9, 100}}));
}

TEST_F(QueueTest, NextEgressRemoveParent) {
  buildSimpleTree();

  // Clear egress for all except txn=9
  signalEgress(0, false);
  signalEgress(3, false);
  signalEgress(5, false);
  signalEgress(7, false);

  // Remove parent of 9 (5)
  removeTransaction(5);
  nextEgress();
  EXPECT_EQ(nodes_, IDList({{9, 100}}));

  // signal egress for 9's new siblings to verify weights
  signalEgress(3, true);
  signalEgress(7, true);

  nextEgress();
  EXPECT_EQ(nodes_, IDList({{7, 50}, {9, 25}, {3, 25}}));
}

TEST_F(QueueTest, AddExclusiveDescendantEnqueued) {
  addTransaction(0, {kRootNodeId, false, 100});
  addTransaction(3, {0, false, 100});
  addTransaction(5, {3, false, 100});
  signalEgress(0, false);
  signalEgress(3, false);
  // add a new exclusive child of 1.  1's child 3 is not enqueued but is in the
  // the egress tree.
  addTransaction(7, {0, true, 100});
  nextEgress();
  EXPECT_EQ(nodes_, IDList({{7, 100}}));
}

TEST_F(QueueTest, NextEgressRemoveParentEnqueued) {
  addTransaction(0, {kRootNodeId, false, 100});
  addTransaction(3, {0, false, 100});
  addTransaction(5, {3, false, 100});
  signalEgress(3, false);
  // When 3's children (5) are added to 1, both are already in the egress tree
  // and the signal does not need to propagate
  removeTransaction(3);
  signalEgress(0, false);
  nextEgress();
  EXPECT_EQ(nodes_, IDList({{5, 100}}));
}

TEST_F(QueueTest, NextEgressRemoveParentEnqueuedIndirect) {
  addTransaction(0, {kRootNodeId, false, 100});
  addTransaction(3, {0, false, 100});
  addTransaction(5, {3, false, 100});
  addTransaction(7, {0, false, 100});
  signalEgress(3, false);
  signalEgress(0, false);
  // When 3's children (5) are added to 1, both are already in the egress tree
  // and the signal does not need to propagate
  removeTransaction(3);
  nextEgress();
  EXPECT_EQ(nodes_, IDList({{7, 50}, {5, 50}}));
}

TEST_F(QueueTest, ChromeTest) {
  // Tries to simulate Chrome's current behavior by performing pseudo-random
  // add-exclusive, signal, clear and remove with 3 insertion points
  // (hi,mid,low).  Note the test uses rand32() with a particular seed so the
  // output is predictable.
  HTTPCodec::StreamID pris[3] = {0, 3, 5};
  addTransaction(0, {kRootNodeId, true, 99});
  signalEgress(0, false);
  addTransaction(3, {0, true, 99});
  signalEgress(3, false);
  addTransaction(5, {3, true, 99});
  signalEgress(5, false);

  std::vector<HTTPCodec::StreamID> txns;
  std::vector<HTTPCodec::StreamID> active;
  std::vector<HTTPCodec::StreamID> inactive;
  HTTPCodec::StreamID txn = 0;
  uint64_t idx = 0;
  HTTPCodec::StreamID nextId = 7;
  auto gen = Random::create();
  gen.seed(12345); // luggage combo
  for (auto i = 4; i < 1000; i++) {
    uint8_t action = rand32(4, gen);
    if (action == 0) {
      // add exclusive on pseudo-random priority anchor
      uint8_t pri = rand32(3, gen);
      HTTPCodec::StreamID dep = pris[pri];
      txn = nextId;
      nextId += 2;
      VLOG(2) << "Adding txn=" << txn << " with dep=" << dep;
      addTransaction(txn, {(uint32_t)dep, true, 99});
      txns.push_back(txn);
      active.push_back(txn);
    } else if (action == 1 && !inactive.empty()) {
      // signal an inactive txn
      idx = rand32(inactive.size(), gen);
      txn = inactive[idx];
      VLOG(2) << "Activating txn=" << txn;
      signalEgress(txn, true);
      inactive.erase(inactive.begin() + idx);
      active.push_back(txn);
    } else if (action == 2 && !active.empty()) {
      // clear an active transaction
      idx = rand32(active.size(), gen);
      txn = active[idx];
      VLOG(2) << "Deactivating txn=" << txn;
      signalEgress(txn, false);
      active.erase(active.begin() + idx);
      inactive.push_back(txn);
    } else if (action == 3 && !txns.empty()) {
      // remove a transaction
      idx = rand32(txns.size(), gen);
      txn = txns[idx];
      VLOG(2) << "Removing txn=" << txn;
      removeTransaction(txn);
      txns.erase(txns.begin() + idx);
      auto it = std::find(active.begin(), active.end(), txn);
      if (it != active.end()) {
        active.erase(it);
      }
      it = std::find(inactive.begin(), inactive.end(), txn);
      if (it != inactive.end()) {
        inactive.erase(it);
      }
    }
    VLOG(2) << "Active nodes=" << q_.numPendingEgress();
    if (!q_.empty()) {
      nextEgress();
      EXPECT_GT(nodes_.size(), 0);
    }
  }
}

TEST_F(QueueTest, NextEgressSpdy) {
  // 0 and 3 are vnodes representing pri 0 and 1
  addTransaction(0, {kRootNodeId, false, 0}, true);
  addTransaction(3, {0, false, 0}, true);

  // 7 and 9 are pri 0, 11 and 13 are pri 1
  addTransaction(7, {0, false, 15});
  addTransaction(9, {0, false, 15});
  addTransaction(11, {3, false, 15});
  addTransaction(13, {3, false, 15});

  nextEgress(true);
  EXPECT_EQ(nodes_, IDList({{7, 50}, {9, 50}}));

  signalEgress(7, false);
  nextEgress(true);
  EXPECT_EQ(nodes_, IDList({{9, 100}}));

  signalEgress(9, false);
  nextEgress(true);
  EXPECT_EQ(nodes_, IDList({{11, 50}, {13, 50}}));
}

TEST_F(QueueTest, AddOrUpdate) {
  q_.addOrUpdatePriorityNode(0, {kRootNodeId, false, 15});
  q_.addOrUpdatePriorityNode(3, {kRootNodeId, false, 15});
  dump();
  EXPECT_EQ(nodes_, IDList({{0, 50}, {3, 50}}));
  q_.addOrUpdatePriorityNode(0, {kRootNodeId, false, 3});
  dump();
  EXPECT_EQ(nodes_, IDList({{0, 20}, {3, 80}}));
}

class DanglingQueueTestBase {
 public:
  DanglingQueueTestBase() {
    // Just under two ticks
    HTTP2PriorityQueue::setNodeLifetime(
        std::chrono::milliseconds(2 * HHWheelTimer::DEFAULT_TICK_INTERVAL - 1));
    EXPECT_CALL(timeoutManager_, scheduleTimeout(_, _))
        .WillRepeatedly(Invoke(
            [this](folly::AsyncTimeout* timeout, std::chrono::milliseconds) {
              timeouts_.push_back(timeout);
              return true;
            }));
  }

 protected:
  void expireNodes() {
    std::this_thread::sleep_for(
        std::chrono::milliseconds(2 * HHWheelTimer::DEFAULT_TICK_INTERVAL));
    // Node lifetime it just under two ticks, so firing twice expires all nodes
    tick();
    tick();
  }

  void tick() {
    std::list<folly::AsyncTimeout*> timeouts;
    std::swap(timeouts_, timeouts);
    for (auto timeout : timeouts) {
      timeout->timeoutExpired();
    }
  }

  std::list<folly::AsyncTimeout*> timeouts_;
  testing::NiceMock<MockTimeoutManager> timeoutManager_;
  folly::UndelayedDestruction<HHWheelTimer> timer_{&timeoutManager_};
};

// Order declaration of the base classes for this fixture matters here: we want
// to pass the timer initialized as part of DanglingQueueTest into to QueueTest,
// so it must be initialized first.
class DanglingQueueTest
    : public DanglingQueueTestBase
    , public QueueTest {
 public:
  DanglingQueueTest() : DanglingQueueTestBase(), QueueTest(&timer_) {
  }
};

TEST_F(DanglingQueueTest, Basic) {
  addTransaction(0, {kRootNodeId, false, 15});
  removeTransaction(0);
  dump();
  EXPECT_EQ(nodes_, IDList({{0, 100}}));
  expireNodes();
  dump();
  EXPECT_EQ(nodes_, IDList({}));
}

TEST_F(DanglingQueueTest, Chain) {
  addTransaction(0, {kRootNodeId, false, 15}, true);
  addTransaction(3, {0, false, 15}, true);
  addTransaction(5, {3, false, 15}, true);
  dump();
  EXPECT_EQ(nodes_, IDList({{0, 100}, {3, 100}, {5, 100}}));
  expireNodes();
  dump();
  EXPECT_EQ(nodes_, IDList({{0, 100}, {3, 100}}));
  expireNodes();
  dump();
  EXPECT_EQ(nodes_, IDList({{0, 100}}));
  expireNodes();
  dump();
  EXPECT_EQ(nodes_, IDList({}));
}

TEST_F(DanglingQueueTest, Drop) {
  addTransaction(0, {kRootNodeId, false, 15}, true);
  addTransaction(3, {0, false, 15}, true);
  addTransaction(5, {0, false, 15}, true);
  dump();
  q_.dropPriorityNodes();
  dump();
  EXPECT_EQ(nodes_, IDList({}));
}

TEST_F(DanglingQueueTest, ExpireParentOfMismatchedTwins) {
  addTransaction(0, {kRootNodeId, true, 219}, false);
  addTransaction(3, {0, false, 146}, false);
  addTransaction(5, {0, false, 146}, false);
  signalEgress(3, false);
  signalEgress(5, true);
  removeTransaction(0);
  dump();
  tick();
  expireNodes();
  dump();
  EXPECT_EQ(nodes_, IDList({{3, 50}, {5, 50}}));
}

TEST_F(DanglingQueueTest, AddExpireAdd) {
  // Add a virtual node
  addTransaction(0, {kRootNodeId, true, 219}, true);
  // expire it
  expireNodes();
  dump();
  EXPECT_TRUE(q_.empty());
  // Add a real node with the same id
  addTransaction(0, {kRootNodeId, true, 219}, false);
  dump();
  EXPECT_EQ(nodes_, IDList({{0, 100}}));
}

class DummyTimeout : public HHWheelTimer::Callback {
  void timeoutExpired() noexcept override {
  }
};

TEST_F(DanglingQueueTest, Refresh) {
  // Having a long running timeout prevents HHWheelTimer::Callback::setScheduled
  // from checking the real time
  DummyTimeout t;
  timer_.scheduleTimeout(&t, std::chrono::seconds(300));
  addTransaction(0, {kRootNodeId, false, 15});
  addTransaction(3, {kRootNodeId, false, 15});
  // 0 is now virtual
  removeTransaction(0);
  dump();
  EXPECT_EQ(nodes_, IDList({{0, 50}, {3, 50}}));
  tick();
  // before 0 times out, change it's priority, should still be there
  updatePriority(0, {kRootNodeId, false, 3});
  dump();
  EXPECT_EQ(nodes_, IDList({{0, 20}, {3, 80}}));

  tick();
  dump();
  EXPECT_EQ(nodes_, IDList({{0, 20}, {3, 80}}));
  expireNodes();
  dump();
  EXPECT_EQ(nodes_, IDList({{3, 100}}));
}

TEST_F(DanglingQueueTest, Max) {
  buildSimpleTree();
  q_.setMaxVirtualNodes(3);
  for (auto i = 1; i <= 9; i += 2) {
    removeTransaction(i == 1 ? 0 : i);
  }
  dump();
  EXPECT_EQ(nodes_, IDList({{0, 100}, {3, 50}, {5, 50}}));
  // 0 expires first and it re-weights 3 and 5, which extends their lifetime
  expireNodes();
  dump();
  EXPECT_EQ(nodes_, IDList({{3, 50}, {5, 50}}));
  expireNodes();
  dump();
  EXPECT_EQ(nodes_, IDList());
}

TEST_F(QueueTest, Rebuild) {
  buildSimpleTree();
  q_.rebuildTree();
  dump();
  EXPECT_EQ(nodes_, IDList({{3, 20}, {9, 20}, {5, 20}, {7, 20}, {0, 20}}));
}

} // namespace proxygen
