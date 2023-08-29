/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <folly/IntrusiveList.h>
#include <folly/io/async/HHWheelTimer.h>
#include <proxygen/lib/http/codec/HTTP2Framer.h>
#include <proxygen/lib/http/codec/HTTPCodec.h>
#include <proxygen/lib/utils/WheelTimerInstance.h>

#include <deque>
#include <list>
#include <unordered_map>

namespace proxygen {

class HTTPTransaction;

class HTTP2PriorityQueueBase : public HTTPCodec::PriorityQueue {
 public:
  class BaseNode {
   public:
    virtual ~BaseNode() {
    }
    virtual bool isEnqueued() const = 0;
    virtual uint64_t calculateDepth(bool includeVirtual = true) const = 0;
  };

  using Handle = BaseNode*;

  explicit HTTP2PriorityQueueBase(HTTPCodec::StreamID rootNodeId)
      : rootNodeId_(rootNodeId) {
  }

  virtual Handle addTransaction(HTTPCodec::StreamID id,
                                http2::PriorityUpdate pri,
                                HTTPTransaction* txn,
                                bool permanent = false,
                                uint64_t* depth = nullptr) = 0;

  // update the priority of an existing node
  virtual Handle updatePriority(Handle handle,
                                http2::PriorityUpdate pri,
                                uint64_t* depth = nullptr) = 0;

  // Remove the transaction from the priority tree
  virtual void removeTransaction(Handle handle) = 0;

  // Notify the queue when a transaction has egress
  virtual void signalPendingEgress(Handle h) = 0;

  // Notify the queue when a transaction no longer has egress
  virtual void clearPendingEgress(Handle h) = 0;

  HTTPCodec::StreamID getRootId() {
    return rootNodeId_;
  }

 protected:
  HTTPCodec::StreamID rootNodeId_{0};
};

class HTTP2PriorityQueue : public HTTP2PriorityQueueBase {

 private:
  class Node;

  static const size_t kNumBuckets = 100;

 public:
  HTTP2PriorityQueue(HTTPCodec::StreamID rootNodeId = 0)
      : HTTP2PriorityQueueBase(rootNodeId),
        root_(*this, nullptr, rootNodeId, 1, nullptr) {
    root_.setPermanent();
  }

  explicit HTTP2PriorityQueue(const WheelTimerInstance& timeout,
                              HTTPCodec::StreamID rootNodeId = 0)
      : HTTP2PriorityQueueBase(rootNodeId),
        root_(*this, nullptr, rootNodeId, 1, nullptr),
        timeout_(timeout) {
    root_.setPermanent();
  }

  void attachThreadLocals(const WheelTimerInstance& timeout);

  void detachThreadLocals();

  void setMaxVirtualNodes(uint32_t maxVirtualNodes) {
    maxVirtualNodes_ = maxVirtualNodes;
  }

  // Notify the queue when a transaction has egress
  void signalPendingEgress(Handle h) override;

  // Notify the queue when a transaction no longer has egress
  void clearPendingEgress(Handle h) override;

  void addPriorityNode(HTTPCodec::StreamID id,
                       HTTPCodec::StreamID parent) override {
    addTransaction(id, {parent, false, 0}, nullptr, true);
  }

  void addOrUpdatePriorityNode(HTTPCodec::StreamID id,
                               http2::PriorityUpdate pri);

  void dropPriorityNodes() {
    root_.dropPriorityNodes();
  }

  // adds new transaction (possibly nullptr) to the priority tree
  Handle addTransaction(HTTPCodec::StreamID id,
                        http2::PriorityUpdate pri,
                        HTTPTransaction* txn,
                        bool permanent = false,
                        uint64_t* depth = nullptr) override;

  // update the priority of an existing node
  Handle updatePriority(Handle handle,
                        http2::PriorityUpdate pri,
                        uint64_t* depth = nullptr) override;

  // Remove the transaction from the priority tree
  void removeTransaction(Handle handle) override;

  // Returns true if there are no transaction with pending egress
  bool empty() const {
    return activeCount_ == 0;
  }

  // The number with pending egress
  uint64_t numPendingEgress() const {
    return activeCount_;
  }

  uint64_t numVirtualNodes() const {
    return numVirtualNodes_;
  }

  void iterate(const std::function<
                   bool(HTTPCodec::StreamID, HTTPTransaction*, double)>& fn,
               const std::function<bool()>& stopFn,
               bool all) {
    updateEnqueuedWeight();
    root_.iterate(fn, stopFn, all);
  }

  // stopFn is only evaluated once per level
  void iterateBFS(
      const std::function<bool(
          HTTP2PriorityQueue&, HTTPCodec::StreamID, HTTPTransaction*, double)>&
          fn,
      const std::function<bool()>& stopFn,
      bool all);

  using NextEgressResult = std::vector<std::pair<HTTPTransaction*, double>>;

  void nextEgress(NextEgressResult& result, bool spdyMode = false);

  static void setNodeLifetime(std::chrono::milliseconds lifetime) {
    kNodeLifetime_ = lifetime;
  }

  /// Error handling code
  // Rebuilds tree by making all non-root nodes direct children of the root and
  // weight reset to the default 16
  void rebuildTree();
  uint32_t getRebuildCount() const {
    return rebuildCount_;
  }
  bool isRebuilt() const {
    return rebuildCount_ > 0;
  }

 private:
  static Node* nodeFromBaseNode(BaseNode* bnode);

  // Find the node in priority tree
  Node* find(HTTPCodec::StreamID id, uint64_t* depth = nullptr);

  Node* findInternal(HTTPCodec::StreamID id) {
    if (id == rootNodeId_) {
      return &root_;
    }
    return find(id);
  }

  bool allowDanglingNodes() const {
    return timeout_ && kNodeLifetime_.count() > 0;
  }

  void scheduleNodeExpiration(Node* node) {
    if (timeout_) {
      VLOG(5) << "scheduling expiration for node=" << node->getID();
      DCHECK_GT(kNodeLifetime_.count(), 0);
      timeout_.scheduleTimeout(node, kNodeLifetime_);
    }
  }

  static bool nextEgressResult(HTTP2PriorityQueue& queue,
                               HTTPCodec::StreamID id,
                               HTTPTransaction* txn,
                               double r);

  void updateEnqueuedWeight();

 private:
  typedef boost::intrusive::link_mode<boost::intrusive::auto_unlink> link_mode;

  class Node
      : public BaseNode
      , public folly::HHWheelTimer::Callback {
   public:
    static const uint16_t kDefaultWeight = 16;

    Node(HTTP2PriorityQueue& queue,
         Node* inParent,
         HTTPCodec::StreamID id,
         uint8_t weight,
         HTTPTransaction* txn);

    ~Node() override;

    void setPermanent() {
      isPermanent_ = true;
    }

    Node* getParent() const {
      return parent_;
    }

    HTTPCodec::StreamID getID() const {
      return id_;
    }

    HTTPCodec::StreamID parentID() const {
      if (parent_) {
        return parent_->id_;
      }
      return queue_.getRootId();
    }

    HTTPTransaction* getTransaction() const {
      return txn_;
    }

    void clearTransaction() {
      txn_ = nullptr;
    }

    // Add a new node as a child of this node
    Node* emplaceNode(std::unique_ptr<Node> node, bool exclusive);

    // Removes the node from the tree
    void removeFromTree();

    void signalPendingEgress();

    void clearPendingEgress();

    uint16_t getWeight() const {
      return weight_;
    }

    // Set a new weight for this node
    void updateWeight(uint8_t weight);

    Node* reparent(Node* newParent, bool exclusive);

    // Returns true if this is a descendant of node
    bool isDescendantOf(Node* node) const;

    // True if this Node is in the egress queue
    bool isEnqueued() const override {
      return (txn_ != nullptr && enqueued_);
    }

    // True if this Node is in the egress tree even if the node itself is
    // virtual but has enqueued descendants.
    bool inEgressTree() const {
      return isEnqueued() || totalEnqueuedWeight_ > 0;
    }

    double getRelativeWeight() const {
      if (!parent_) {
        return 1.0;
      }

      return static_cast<double>(weight_) /
             static_cast<double>(parent_->totalChildWeight_);
    }

    double getRelativeEnqueuedWeight() const {
      if (!parent_) {
        return 1.0;
      }

      if (parent_->totalEnqueuedWeight_ == 0) {
        return 0.0;
      }

      return static_cast<double>(weight_) /
             static_cast<double>(parent_->totalEnqueuedWeight_);
    }

    /* Execute the given function on this node and all child nodes presently
     * enqueued, until one of them asks to stop, or the stop function returns
     * true.
     *
     * The all parameter visits every node, even the ones not currently
     * enqueued.
     *
     * The arguments to the function are
     *   txn - HTTPTransaction for the node
     *   ratio - weight of this txn relative to all peers (not just enequeued)
     */
    bool iterate(const std::function<
                     bool(HTTPCodec::StreamID, HTTPTransaction*, double)>& fn,
                 const std::function<bool()>& stopFn,
                 bool all);

    struct PendingNode {
      HTTPCodec::StreamID id;
      Node* node;
      double ratio;
      PendingNode(HTTPCodec::StreamID i, Node* n, double r)
          : id(i), node(n), ratio(r) {
      }
    };

    using PendingList = std::deque<PendingNode>;
    bool visitBFS(double relativeParentWeight,
                  const std::function<bool(HTTP2PriorityQueue& queue,
                                           HTTPCodec::StreamID,
                                           HTTPTransaction*,
                                           double)>& fn,
                  bool all,
                  PendingList& pendingNodes,
                  bool enqueuedChildren);

    void updateEnqueuedWeight(bool activeNodes);

    void dropPriorityNodes();

    void convertVirtualNode(HTTPTransaction* txn);

    uint64_t calculateDepth(bool includeVirtual = true) const override;

    // Internal error recovery
    void flattenSubtree();
    void flattenSubtreeDFS(Node* subtreeRoot);
    static void addChildToNewSubtreeRoot(std::unique_ptr<Node> child,
                                         Node* subtreeRoot);

   private:
    Node* addChild(std::unique_ptr<Node> child);

    void addChildren(std::list<std::unique_ptr<Node>>&& children);

    std::unique_ptr<Node> detachChild(Node* node);

    void addEnqueuedChild(HTTP2PriorityQueue::Node* node);

    void removeEnqueuedChild(HTTP2PriorityQueue::Node* node);

    static void propagatePendingEgressSignal(Node* node);

    static void propagatePendingEgressClear(Node* node);

    void timeoutExpired() noexcept override {
      VLOG(5) << "Node=" << id_ << " expired";
      CHECK(txn_ == nullptr);
      queue_.pendingWeightChange_ = true;
      removeFromTree();
    }

    void refreshTimeout() {
      if (!txn_ && !isPermanent_ && isScheduled()) {
        queue_.scheduleNodeExpiration(this);
      }
    }

    HTTP2PriorityQueue& queue_;
    Node* parent_{nullptr};
    HTTPCodec::StreamID id_;
    uint16_t weight_{kDefaultWeight};
    HTTPTransaction* txn_{nullptr};
    bool isPermanent_{false};
    bool enqueued_{false};
#ifndef NDEBUG
    uint64_t totalEnqueuedWeightCheck_{0};
#endif
    uint64_t totalEnqueuedWeight_{0};
    uint64_t totalChildWeight_{0};
    std::list<std::unique_ptr<Node>> children_;
    std::list<std::unique_ptr<Node>>::iterator self_;
    // enqueuedChildren_ includes all children that are themselves enqueued_
    // or have enqueued descendants. Therefore, enqueuedChildren_ may contain
    // direct children that have enqueued_ == false
    folly::IntrusiveListHook enqueuedHook_;
    folly::IntrusiveList<Node, &Node::enqueuedHook_> enqueuedChildren_;
  };

  using NodeMap = folly::F14FastMap<HTTPCodec::StreamID, Node*>;

  NodeMap nodes_;
  Node root_;
  uint32_t rebuildCount_{0};
  static uint32_t kMaxRebuilds_;
  uint64_t activeCount_{0};
  uint32_t maxVirtualNodes_{50};
  uint32_t numVirtualNodes_{0};
  folly::Optional<HTTPCodec::StreamID> largestId_;
  bool pendingWeightChange_{false};
  WheelTimerInstance timeout_;

  NextEgressResult* nextEgressResults_{nullptr};
  static std::chrono::milliseconds kNodeLifetime_;
};

} // namespace proxygen
