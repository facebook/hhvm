/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <folly/io/IOBuf.h>
#include <folly/io/async/DelayedDestruction.h>
#include <folly/io/async/EventBase.h>
#include <folly/portability/GTest.h>
#include <thrift/lib/cpp2/async/MessageChannel.h>
#include <thrift/lib/cpp2/transport/rocket/server/detail/WriteBatcher.h>

namespace apache::thrift::rocket {
namespace {

template <typename Owner>
class TestConnectionAdapter {
 public:
  explicit TestConnectionAdapter(Owner& owner) : owner_(&owner) {}

  auto getDestructorGuard() { return owner_->getDestructorGuard(); }

  bool getDestroyPending() const { return owner_->getDestroyPending(); }

  size_t numObservers() const { return 0; }

  folly::EventBase& getEventBase() { return owner_->eventBase; }

  void flushWrites(
      std::unique_ptr<folly::IOBuf>,
      apache::thrift::rocket::WriteBatchContext&&) {}

  void flushWritesWithFds(
      std::unique_ptr<folly::IOBuf>,
      apache::thrift::rocket::WriteBatchContext&&,
      apache::thrift::rocket::FdsAndOffsets&&) {}

 private:
  Owner* owner_;
};

class BatcherOwner : public folly::DelayedDestruction {
 public:
  explicit BatcherOwner(bool& destroyed)
      : destroyed_(&destroyed),
        adapter(*this),
        batcher(
            adapter,
            std::chrono::milliseconds::zero(),
            1024,
            0) {}

  DestructorGuard getDestructorGuard() { return DestructorGuard(this); }

  bool* destroyed_;
  folly::EventBase eventBase;
  TestConnectionAdapter<BatcherOwner> adapter;
  WriteBatcher<BatcherOwner, TestConnectionAdapter> batcher;

 private:
  ~BatcherOwner() override { *destroyed_ = true; }
};

class DestroyFromSendQueuedCallback
    : public apache::thrift::MessageChannel::SendCallback {
 public:
  DestroyFromSendQueuedCallback(
      BatcherOwner& owner, int& queued, int& errors)
      : owner_(&owner), queued_(&queued), errors_(&errors) {}

  void sendQueued() override {
    ++*queued_;
    owner_->destroy();
  }

  void messageSent() override {
    ADD_FAILURE() << "message should not be sent after owner destruction";
    delete this;
  }

  void messageSendError(folly::exception_wrapper&&) override {
    ++*errors_;
    delete this;
  }

 private:
  BatcherOwner* owner_;
  int* queued_;
  int* errors_;
};

TEST(WriteBatcherTest, DestroyOwnerFromSendQueued) {
  bool destroyed = false;
  int queued = 0;
  int errors = 0;
  auto* owner = new BatcherOwner(destroyed);

  owner->batcher.enqueueWrite(
      folly::IOBuf::copyBuffer("payload"),
      apache::thrift::MessageChannel::SendCallbackPtr(
          new DestroyFromSendQueuedCallback(*owner, queued, errors)),
      StreamId{1},
      folly::SocketFds{});

  EXPECT_TRUE(destroyed);
  EXPECT_EQ(queued, 1);
  EXPECT_EQ(errors, 1);
}

} // namespace
} // namespace apache::thrift::rocket
