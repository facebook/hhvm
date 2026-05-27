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

#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/HandlerTag.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/PipelineBuilder.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/test/MockAdapters.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/test/MockHandler.h>

#include <folly/io/async/EventBase.h>
#include <folly/portability/GTest.h>

#include <memory>
#include <utility>
#include <vector>

namespace apache::thrift::fast_thrift::channel_pipeline::test {

namespace {

// Marker payload for a broadcast event. Identity matters; value doesn't.
struct TestEvent {
  int tag{0};
};

// Endpoint mocks that opt into user-event broadcast by implementing
// `onEvent(const TypeErasedBox&)`. Otherwise identical to the generic
// Mock{Head,Tail}Handler endpoints.
class EventAwareHeadHandler {
 public:
  // HeadEndpoint required surface
  Result onWrite(TypeErasedBox&& msg) noexcept {
    writes_.push_back(std::move(msg));
    return Result::Success;
  }
  void onException(folly::exception_wrapper&&) noexcept {}
  void handlerAdded() noexcept {}
  void handlerRemoved() noexcept {}
  void onPipelineActive() noexcept {}
  void onPipelineInactive() noexcept {}
  void onReadReady() noexcept {}

  // Opt-in broadcast event receiver.
  void onEvent(const TypeErasedBox& evt) noexcept {
    events_.push_back(evt.get<TestEvent>().tag);
  }

  const std::vector<int>& events() const noexcept { return events_; }

 private:
  std::vector<TypeErasedBox> writes_;
  std::vector<int> events_;
};

class EventAwareTailHandler {
 public:
  // TailEndpoint required surface
  Result onRead(TypeErasedBox&& msg) noexcept {
    reads_.push_back(std::move(msg));
    return Result::Success;
  }
  void onException(folly::exception_wrapper&&) noexcept {}
  void handlerAdded() noexcept {}
  void handlerRemoved() noexcept {}
  void onPipelineActive() noexcept {}
  void onPipelineInactive() noexcept {}
  void onWriteReady() noexcept {}

  // Opt-in broadcast event receiver.
  void onEvent(const TypeErasedBox& evt) noexcept {
    events_.push_back(evt.get<TestEvent>().tag);
  }

  const std::vector<int>& events() const noexcept { return events_; }

 private:
  std::vector<TypeErasedBox> reads_;
  std::vector<int> events_;
};

// Internal handler that opts into broadcast events.
class EventAwareHandler {
 public:
  // === HandlerLifecycle ===
  void handlerAdded(detail::ContextImpl&) noexcept {}
  void handlerRemoved(detail::ContextImpl&) noexcept {}

  // === InboundHandler (pass-through for data plane) ===
  void onPipelineActive(detail::ContextImpl&) noexcept {}
  Result onRead(detail::ContextImpl& ctx, TypeErasedBox&& msg) noexcept {
    return ctx.fireRead(std::move(msg));
  }
  void onReadReady(detail::ContextImpl&) noexcept {}
  void onException(
      detail::ContextImpl& ctx, folly::exception_wrapper&& e) noexcept {
    ctx.fireException(std::move(e));
  }

  // === OutboundHandler (pass-through for data plane) ===
  Result onWrite(detail::ContextImpl& ctx, TypeErasedBox&& msg) noexcept {
    return ctx.fireWrite(std::move(msg));
  }
  void onWriteReady(detail::ContextImpl&) noexcept {}
  void onPipelineInactive(detail::ContextImpl&) noexcept {}

  // === Broadcast event receiver ===
  void onEvent(detail::ContextImpl&, const TypeErasedBox& evt) noexcept {
    events_.push_back(evt.get<TestEvent>().tag);
  }

  const std::vector<int>& events() const noexcept { return events_; }

 private:
  std::vector<int> events_;
};

HANDLER_TAG(a);
HANDLER_TAG(b);
HANDLER_TAG(c);

} // namespace

// =============================================================================
// Broadcast semantics
// =============================================================================

TEST(EventTest, FireEventReachesAllRegisteredHandlersAndBothEndpoints) {
  folly::EventBase evb;
  EventAwareHeadHandler head;
  EventAwareTailHandler tail;
  TestAllocator alloc;

  auto h1 = std::make_unique<EventAwareHandler>();
  auto* h1Ptr = h1.get();
  auto h2 = std::make_unique<EventAwareHandler>();
  auto* h2Ptr = h2.get();

  auto pipeline = PipelineBuilder<
                      EventAwareHeadHandler,
                      EventAwareTailHandler,
                      TestAllocator>()
                      .setEventBase(&evb)
                      .setHead(&head)
                      .setTail(&tail)
                      .setAllocator(&alloc)
                      .addNextDuplex<EventAwareHandler>(a_tag, std::move(h1))
                      .addNextDuplex<EventAwareHandler>(b_tag, std::move(h2))
                      .build();

  pipeline->fireEvent(TypeErasedBox(TestEvent{42}));

  EXPECT_EQ(tail.events(), std::vector<int>{42});
  EXPECT_EQ(h1Ptr->events(), std::vector<int>{42});
  EXPECT_EQ(h2Ptr->events(), std::vector<int>{42});
  EXPECT_EQ(head.events(), std::vector<int>{42});
}

TEST(EventTest, HandlersWithoutOnEventAreSkipped) {
  // Mixed pipeline: some handlers implement onEvent, some don't. The
  // sparse eventList_ should only iterate the registered ones.
  folly::EventBase evb;
  EventAwareHeadHandler head;
  EventAwareTailHandler tail;
  TestAllocator alloc;

  auto h = std::make_unique<EventAwareHandler>();
  auto* hPtr = h.get();

  auto pipeline =
      PipelineBuilder<
          EventAwareHeadHandler,
          EventAwareTailHandler,
          TestAllocator>()
          .setEventBase(&evb)
          .setHead(&head)
          .setTail(&tail)
          .setAllocator(&alloc)
          // Plain MockHandler does not implement onEvent.
          .addNextDuplex<MockHandler>(a_tag, std::make_unique<MockHandler>())
          .addNextDuplex<EventAwareHandler>(b_tag, std::move(h))
          .addNextDuplex<MockHandler>(c_tag, std::make_unique<MockHandler>())
          .build();

  pipeline->fireEvent(TypeErasedBox(TestEvent{1}));

  EXPECT_EQ(hPtr->events(), std::vector<int>{1});
  EXPECT_EQ(tail.events(), std::vector<int>{1});
  EXPECT_EQ(head.events(), std::vector<int>{1});
  // No crash, no extra calls — MockHandlers were silently skipped.
}

TEST(EventTest, EndpointsWithoutOnEventAreSkipped) {
  folly::EventBase evb;
  MockHeadHandler head;
  MockTailHandler tail;
  TestAllocator alloc;

  auto h = std::make_unique<EventAwareHandler>();
  auto* hPtr = h.get();

  auto pipeline =
      PipelineBuilder<MockHeadHandler, MockTailHandler, TestAllocator>()
          .setEventBase(&evb)
          .setHead(&head)
          .setTail(&tail)
          .setAllocator(&alloc)
          .addNextDuplex<EventAwareHandler>(a_tag, std::move(h))
          .build();

  pipeline->fireEvent(TypeErasedBox(TestEvent{1}));

  EXPECT_EQ(hPtr->events(), std::vector<int>{1});
  // Endpoints have no onEvent — pipeline silently doesn't call them.
}

TEST(EventTest, EmptyPipelineDeliversToBothEndpoints) {
  folly::EventBase evb;
  EventAwareHeadHandler head;
  EventAwareTailHandler tail;
  TestAllocator alloc;

  auto pipeline = PipelineBuilder<
                      EventAwareHeadHandler,
                      EventAwareTailHandler,
                      TestAllocator>()
                      .setEventBase(&evb)
                      .setHead(&head)
                      .setTail(&tail)
                      .setAllocator(&alloc)
                      .build();

  pipeline->fireEvent(TypeErasedBox(TestEvent{7}));

  EXPECT_EQ(tail.events(), std::vector<int>{7});
  EXPECT_EQ(head.events(), std::vector<int>{7});
}

TEST(EventTest, NoOpWhenNothingIsRegistered) {
  folly::EventBase evb;
  MockHeadHandler head;
  MockTailHandler tail;
  TestAllocator alloc;

  auto pipeline =
      PipelineBuilder<MockHeadHandler, MockTailHandler, TestAllocator>()
          .setEventBase(&evb)
          .setHead(&head)
          .setTail(&tail)
          .setAllocator(&alloc)
          .build();

  // No handlers, no opt-in endpoints. Should be a clean no-op.
  pipeline->fireEvent(TypeErasedBox(TestEvent{1}));
  EXPECT_EQ(head.writeCount(), 0);
  EXPECT_EQ(tail.readCount(), 0);
}

// =============================================================================
// Iteration order
// =============================================================================

TEST(EventTest, IterationOrderIsTailEndpointThenInternalTailToHeadThenHead) {
  // Record observation order globally — verify tail endpoint runs first,
  // then internal handlers in tail→head (descending-index) order, then
  // head endpoint last.
  folly::EventBase evb;
  EventAwareHeadHandler head;
  EventAwareTailHandler tail;
  TestAllocator alloc;

  std::vector<std::string> order;

  // Tiny inline handler that records its own name on every event.
  class NamedHandler {
   public:
    NamedHandler(std::string name, std::vector<std::string>& order) noexcept
        : name_(std::move(name)), order_(order) {}

    void handlerAdded(detail::ContextImpl&) noexcept {}
    void handlerRemoved(detail::ContextImpl&) noexcept {}
    void onPipelineActive(detail::ContextImpl&) noexcept {}
    Result onRead(detail::ContextImpl& ctx, TypeErasedBox&& msg) noexcept {
      return ctx.fireRead(std::move(msg));
    }
    void onReadReady(detail::ContextImpl&) noexcept {}
    void onException(
        detail::ContextImpl& ctx, folly::exception_wrapper&& e) noexcept {
      ctx.fireException(std::move(e));
    }
    Result onWrite(detail::ContextImpl& ctx, TypeErasedBox&& msg) noexcept {
      return ctx.fireWrite(std::move(msg));
    }
    void onWriteReady(detail::ContextImpl&) noexcept {}
    void onPipelineInactive(detail::ContextImpl&) noexcept {}

    void onEvent(detail::ContextImpl&, const TypeErasedBox&) noexcept {
      order_.push_back(name_);
    }

   private:
    std::string name_;
    std::vector<std::string>& order_;
  };

  // Wrap the endpoints' onEvent so they record too.
  class OrderedTail : public EventAwareTailHandler {
   public:
    explicit OrderedTail(std::vector<std::string>& order) noexcept
        : order_(order) {}
    void onEvent(const TypeErasedBox& evt) noexcept {
      order_.emplace_back("tail");
      EventAwareTailHandler::onEvent(evt);
    }

   private:
    std::vector<std::string>& order_;
  };
  class OrderedHead : public EventAwareHeadHandler {
   public:
    explicit OrderedHead(std::vector<std::string>& order) noexcept
        : order_(order) {}
    void onEvent(const TypeErasedBox& evt) noexcept {
      order_.emplace_back("head");
      EventAwareHeadHandler::onEvent(evt);
    }

   private:
    std::vector<std::string>& order_;
  };

  OrderedHead h{order};
  OrderedTail t{order};

  auto pipeline = PipelineBuilder<OrderedHead, OrderedTail, TestAllocator>()
                      .setEventBase(&evb)
                      .setHead(&h)
                      .setTail(&t)
                      .setAllocator(&alloc)
                      // Three internal handlers added in pipeline order (a → b
                      // → c from tail toward head). Iteration is reverse-index,
                      // so expected internal order is c, b, a.
                      .addNextDuplex<NamedHandler>(
                          a_tag, std::make_unique<NamedHandler>("a", order))
                      .addNextDuplex<NamedHandler>(
                          b_tag, std::make_unique<NamedHandler>("b", order))
                      .addNextDuplex<NamedHandler>(
                          c_tag, std::make_unique<NamedHandler>("c", order))
                      .build();

  pipeline->fireEvent(TypeErasedBox(TestEvent{1}));

  const std::vector<std::string> expected{"tail", "c", "b", "a", "head"};
  EXPECT_EQ(order, expected);
}

} // namespace apache::thrift::fast_thrift::channel_pipeline::test
