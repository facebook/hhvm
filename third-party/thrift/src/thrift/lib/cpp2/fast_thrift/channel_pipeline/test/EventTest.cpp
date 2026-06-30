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

#include <cstdint>
#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace apache::thrift::fast_thrift::channel_pipeline::test {

namespace {

// Test event enum: a uint32_t-backed enum class with a trailing Count
// sentinel, as required by the EventEnum concept.
enum class Ev : std::uint32_t {
  Alpha,
  Beta,
  Gamma,
  Count,
};

// Optional event payload; most events are payload-less (empty box).
struct Payload {
  int value{0};
};

// Internal handler that subscribes to a compile-time set of event types and
// records every event it receives. Data-plane methods are pass-through.
template <Ev... Evs>
class SubHandler {
 public:
  static constexpr Subscriptions<Evs...> kSubscribedEvents{};

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

  void onEvent(detail::ContextImpl&, Ev ev, const TypeErasedBox&) noexcept {
    received_.push_back(ev);
  }

  const std::vector<Ev>& received() const noexcept { return received_; }

 private:
  std::vector<Ev> received_;
};

// Head endpoint that subscribes to a compile-time set of event types.
template <Ev... Evs>
class SubHead {
 public:
  static constexpr Subscriptions<Evs...> kSubscribedEvents{};

  Result onWrite(TypeErasedBox&&) noexcept { return Result::Success; }
  void onException(folly::exception_wrapper&&) noexcept {}
  void handlerAdded() noexcept {}
  void handlerRemoved() noexcept {}
  void onPipelineActive() noexcept {}
  void onPipelineInactive() noexcept {}
  void onReadReady() noexcept {}

  void onEvent(Ev ev, const TypeErasedBox&) noexcept {
    received_.push_back(ev);
  }

  const std::vector<Ev>& received() const noexcept { return received_; }

 private:
  std::vector<Ev> received_;
};

// Tail endpoint that subscribes to a compile-time set of event types.
template <Ev... Evs>
class SubTail {
 public:
  static constexpr Subscriptions<Evs...> kSubscribedEvents{};

  Result onRead(TypeErasedBox&&) noexcept { return Result::Success; }
  void onException(folly::exception_wrapper&&) noexcept {}
  void handlerAdded() noexcept {}
  void handlerRemoved() noexcept {}
  void onPipelineActive() noexcept {}
  void onPipelineInactive() noexcept {}
  void onWriteReady() noexcept {}

  void onEvent(Ev ev, const TypeErasedBox&) noexcept {
    received_.push_back(ev);
  }

  const std::vector<Ev>& received() const noexcept { return received_; }

 private:
  std::vector<Ev> received_;
};

// Subscriber that reads the typed payload off the event box.
class PayloadHandler {
 public:
  static constexpr Subscriptions<Ev::Alpha> kSubscribedEvents{};
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
  void onEvent(detail::ContextImpl&, Ev, const TypeErasedBox& evt) noexcept {
    value_ = evt.get<Payload>().value;
  }
  int value() const noexcept { return value_; }

 private:
  int value_{-1};
};

// Internal handler that records its name when its subscribed event fires.
// Used to verify per-event iteration order.
class NamedHandler {
 public:
  static constexpr Subscriptions<Ev::Alpha> kSubscribedEvents{};
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
  void onEvent(detail::ContextImpl&, Ev, const TypeErasedBox&) noexcept {
    order_.push_back(name_);
  }

 private:
  std::string name_;
  std::vector<std::string>& order_;
};

// Alpha-subscribed endpoints that record their position in the iteration order.
class OrderHead : public SubHead<Ev::Alpha> {
 public:
  explicit OrderHead(std::vector<std::string>& order) noexcept
      : order_(order) {}
  void onEvent(Ev, const TypeErasedBox&) noexcept {
    order_.emplace_back("head");
  }

 private:
  std::vector<std::string>& order_;
};
class OrderTail : public SubTail<Ev::Alpha> {
 public:
  explicit OrderTail(std::vector<std::string>& order) noexcept
      : order_(order) {}
  void onEvent(Ev, const TypeErasedBox&) noexcept {
    order_.emplace_back("tail");
  }

 private:
  std::vector<std::string>& order_;
};

// Anchored layer enums: an upper layer continues the lower layer's id space, so
// every layer shares one collision-free id space while staying its own type.
enum class LTransportEvent : std::uint32_t { WriteComplete, Heartbeat, Count };
enum class LRocketEvent : std::uint32_t {
  ConnClose = static_cast<std::uint32_t>(LTransportEvent::Count), // 2
  Count, // 3 (cumulative: the full id space)
};

// A common handler that subscribes in its OWN lower-layer enum and never names
// the pipeline's top enum. Data-plane methods are pass-through.
class LayeredTransportHandler {
 public:
  static constexpr Subscriptions<LTransportEvent::WriteComplete>
      kSubscribedEvents{};

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

  void onEvent(
      detail::ContextImpl&, LTransportEvent ev, const TypeErasedBox&) noexcept {
    received_.push_back(ev);
  }

  const std::vector<LTransportEvent>& received() const noexcept {
    return received_;
  }

 private:
  std::vector<LTransportEvent> received_;
};

// A rocket-layer handler that subscribes to events from MULTIPLE layers at once
// — its own (LRocketEvent) AND a lower one (LTransportEvent) — with a typed
// onEvent overload per layer. This is the cross-layer listening requirement.
class LayeredMultiHandler {
 public:
  static constexpr Subscriptions<
      LTransportEvent::WriteComplete,
      LRocketEvent::ConnClose>
      kSubscribedEvents{};

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

  void onEvent(
      detail::ContextImpl&, LTransportEvent ev, const TypeErasedBox&) noexcept {
    transport_.push_back(ev);
  }
  void onEvent(
      detail::ContextImpl&, LRocketEvent ev, const TypeErasedBox&) noexcept {
    rocket_.push_back(ev);
  }

  const std::vector<LTransportEvent>& transport() const noexcept {
    return transport_;
  }
  const std::vector<LRocketEvent>& rocket() const noexcept { return rocket_; }

 private:
  std::vector<LTransportEvent> transport_;
  std::vector<LRocketEvent> rocket_;
};

// Layer enums that expose Base (the anchor) so the tiling check can read it.
enum class VTransport : std::uint32_t { Base = 0, A = Base, B, Count }; // [0,2)
enum class VFrame : std::uint32_t {
  Base = kLayerBaseAfter<VTransport>,
  C = Base,
  Count
}; // [2,3)
enum class VRocket : std::uint32_t {
  Base = kLayerBaseAfter<VFrame>,
  D = Base,
  Count
}; // [3,4)

// A correctly anchored stack tiles [0, VRocket::Count) with no gaps/overlaps.
static_assert(kLayersTile<VTransport, VFrame, VRocket>);
static_assert(kLayersTile<VTransport>); // single layer is trivially contiguous

// Mis-anchorings are rejected at compile time.
enum class WStart : std::uint32_t {
  Base = 5,
  X = Base,
  Count
}; // not anchored at 0
static_assert(!kLayersTile<WStart>);
enum class WGap : std::uint32_t {
  Base = 99,
  Y = Base,
  Count
}; // gap above VTransport
static_assert(!kLayersTile<VTransport, WGap>);
enum class WOverlap : std::uint32_t {
  Base = 1,
  Z = Base,
  Count
}; // overlaps VTransport
static_assert(!kLayersTile<VTransport, WOverlap>);
static_assert(!kLayersTile<VTransport, VRocket, VFrame>); // reordered layers

HANDLER_TAG(a);
HANDLER_TAG(b);
HANDLER_TAG(c);

} // namespace

// =============================================================================
// Per-event routing
// =============================================================================

TEST(EventTest, EventReachesOnlyItsSubscribers) {
  // head subscribes Alpha, tail subscribes Beta; h1 subscribes Alpha, h2 Beta.
  folly::EventBase evb;
  SubHead<Ev::Alpha> head;
  SubTail<Ev::Beta> tail;
  TestAllocator alloc;

  auto h1 = std::make_unique<SubHandler<Ev::Alpha>>();
  auto* h1Ptr = h1.get();
  auto h2 = std::make_unique<SubHandler<Ev::Beta>>();
  auto* h2Ptr = h2.get();

  auto pipeline =
      PipelineBuilder<
          SubHead<Ev::Alpha>,
          SubTail<Ev::Beta>,
          TestAllocator,
          Ev>()
          .setEventBase(&evb)
          .setHead(&head)
          .setTail(&tail)
          .setAllocator(&alloc)
          .addNextDuplex<SubHandler<Ev::Alpha>>(a_tag, std::move(h1))
          .addNextDuplex<SubHandler<Ev::Beta>>(b_tag, std::move(h2))
          .build();

  pipeline->fireEvent(Ev::Alpha, TypeErasedBox{});
  EXPECT_EQ(head.received(), std::vector<Ev>{Ev::Alpha});
  EXPECT_EQ(h1Ptr->received(), std::vector<Ev>{Ev::Alpha});
  EXPECT_TRUE(tail.received().empty());
  EXPECT_TRUE(h2Ptr->received().empty());

  pipeline->fireEvent(Ev::Beta, TypeErasedBox{});
  EXPECT_EQ(tail.received(), std::vector<Ev>{Ev::Beta});
  EXPECT_EQ(h2Ptr->received(), std::vector<Ev>{Ev::Beta});
  // Alpha subscribers unchanged.
  EXPECT_EQ(head.received(), std::vector<Ev>{Ev::Alpha});
  EXPECT_EQ(h1Ptr->received(), std::vector<Ev>{Ev::Alpha});
}

TEST(EventTest, HandlerSubscribedToMultipleEventsReceivesEach) {
  folly::EventBase evb;
  MockHeadHandler head;
  MockTailHandler tail;
  TestAllocator alloc;

  auto h = std::make_unique<SubHandler<Ev::Alpha, Ev::Gamma>>();
  auto* hPtr = h.get();

  auto pipeline =
      PipelineBuilder<MockHeadHandler, MockTailHandler, TestAllocator, Ev>()
          .setEventBase(&evb)
          .setHead(&head)
          .setTail(&tail)
          .setAllocator(&alloc)
          .addNextDuplex<SubHandler<Ev::Alpha, Ev::Gamma>>(a_tag, std::move(h))
          .build();

  pipeline->fireEvent(Ev::Alpha, TypeErasedBox{});
  pipeline->fireEvent(Ev::Beta, TypeErasedBox{}); // not subscribed
  pipeline->fireEvent(Ev::Gamma, TypeErasedBox{});

  EXPECT_EQ(hPtr->received(), (std::vector<Ev>{Ev::Alpha, Ev::Gamma}));
}

TEST(EventTest, NonSubscribersAreNeverInvoked) {
  // Mixed pipeline: a plain MockHandler (no subscription) and an Alpha
  // subscriber. Firing Alpha reaches only the subscriber; MockHandler is
  // never touched (no crash, no spurious data-plane calls).
  folly::EventBase evb;
  MockHeadHandler head;
  MockTailHandler tail;
  TestAllocator alloc;

  auto h = std::make_unique<SubHandler<Ev::Alpha>>();
  auto* hPtr = h.get();

  auto pipeline =
      PipelineBuilder<MockHeadHandler, MockTailHandler, TestAllocator, Ev>()
          .setEventBase(&evb)
          .setHead(&head)
          .setTail(&tail)
          .setAllocator(&alloc)
          .addNextDuplex<MockHandler>(a_tag, std::make_unique<MockHandler>())
          .addNextDuplex<SubHandler<Ev::Alpha>>(b_tag, std::move(h))
          .build();

  pipeline->fireEvent(Ev::Alpha, TypeErasedBox{});

  EXPECT_EQ(hPtr->received(), std::vector<Ev>{Ev::Alpha});
  EXPECT_EQ(head.writeCount(), 0);
  EXPECT_EQ(tail.readCount(), 0);
}

TEST(EventTest, FiringEventWithNoSubscribersIsNoOp) {
  folly::EventBase evb;
  MockHeadHandler head;
  MockTailHandler tail;
  TestAllocator alloc;

  auto h = std::make_unique<SubHandler<Ev::Alpha>>();
  auto* hPtr = h.get();

  auto pipeline =
      PipelineBuilder<MockHeadHandler, MockTailHandler, TestAllocator, Ev>()
          .setEventBase(&evb)
          .setHead(&head)
          .setTail(&tail)
          .setAllocator(&alloc)
          .addNextDuplex<SubHandler<Ev::Alpha>>(a_tag, std::move(h))
          .build();

  // Nobody subscribes to Gamma — clean no-op.
  pipeline->fireEvent(Ev::Gamma, TypeErasedBox{});
  EXPECT_TRUE(hPtr->received().empty());
}

TEST(EventTest, EventPayloadIsDeliveredToSubscriber) {
  folly::EventBase evb;
  MockHeadHandler head;
  MockTailHandler tail;
  TestAllocator alloc;

  auto h = std::make_unique<PayloadHandler>();
  auto* hPtr = h.get();

  auto pipeline =
      PipelineBuilder<MockHeadHandler, MockTailHandler, TestAllocator, Ev>()
          .setEventBase(&evb)
          .setHead(&head)
          .setTail(&tail)
          .setAllocator(&alloc)
          .addNextDuplex<PayloadHandler>(a_tag, std::move(h))
          .build();

  pipeline->fireEvent(Ev::Alpha, TypeErasedBox(Payload{42}));
  EXPECT_EQ(hPtr->value(), 42);
}

// =============================================================================
// Iteration order within an event's list
// =============================================================================

TEST(EventTest, IterationOrderIsTailEndpointThenInternalTailToHeadThenHead) {
  folly::EventBase evb;
  TestAllocator alloc;
  std::vector<std::string> order;

  OrderHead h{order};
  OrderTail t{order};

  auto pipeline = PipelineBuilder<OrderHead, OrderTail, TestAllocator, Ev>()
                      .setEventBase(&evb)
                      .setHead(&h)
                      .setTail(&t)
                      .setAllocator(&alloc)
                      // Added a → b → c (tail toward head). Iteration is
                      // reverse-index, so expected internal order is c, b, a.
                      .addNextDuplex<NamedHandler>(
                          a_tag, std::make_unique<NamedHandler>("a", order))
                      .addNextDuplex<NamedHandler>(
                          b_tag, std::make_unique<NamedHandler>("b", order))
                      .addNextDuplex<NamedHandler>(
                          c_tag, std::make_unique<NamedHandler>("c", order))
                      .build();

  pipeline->fireEvent(Ev::Alpha, TypeErasedBox{});

  const std::vector<std::string> expected{"tail", "c", "b", "a", "head"};
  EXPECT_EQ(order, expected);
}

// =============================================================================
// Disabled (NoEvent) pipelines
// =============================================================================

TEST(EventTest, EventsDisabledByDefaultDeliverNothing) {
  // Default EventEnum is NoEvent: the subscriber is never wired and fireEvent
  // is a no-op. The typed fireEvent still compiles for any event enum.
  folly::EventBase evb;
  MockHeadHandler head;
  MockTailHandler tail;
  TestAllocator alloc;

  auto h = std::make_unique<SubHandler<Ev::Alpha>>();
  auto* hPtr = h.get();

  auto pipeline =
      PipelineBuilder<MockHeadHandler, MockTailHandler, TestAllocator>()
          .setEventBase(&evb)
          .setHead(&head)
          .setTail(&tail)
          .setAllocator(&alloc)
          .addNextDuplex<SubHandler<Ev::Alpha>>(a_tag, std::move(h))
          .build();

  pipeline->fireEvent(Ev::Alpha, TypeErasedBox{});
  EXPECT_TRUE(hPtr->received().empty());
}

// =============================================================================
// Layered (anchored) events: subscriber uses its own enum, pipeline sizes on
// top
// =============================================================================

TEST(EventTest, SubscriberUsesOwnLayerEnumWhilePipelineSizesOnTopEnum) {
  // The pipeline's event enum is the TOP layer (LRocketEvent, Count == 3). A
  // common handler subscribes in its OWN lower-layer enum (LTransportEvent) and
  // never names the top enum. Anchoring places both layers in one id space, so
  // the subscribed value already carries the global id and dispatch lands on
  // it.
  folly::EventBase evb;
  MockHeadHandler head;
  MockTailHandler tail;
  TestAllocator alloc;

  auto h = std::make_unique<LayeredTransportHandler>();
  auto* hPtr = h.get();

  auto pipeline =
      PipelineBuilder<
          MockHeadHandler,
          MockTailHandler,
          TestAllocator,
          LRocketEvent>()
          .setEventBase(&evb)
          .setHead(&head)
          .setTail(&tail)
          .setAllocator(&alloc)
          .addNextDuplex<LayeredTransportHandler>(a_tag, std::move(h))
          .build();

  // A transport-layer event (the handler's own enum) is delivered, typed.
  pipeline->fireEvent(LTransportEvent::WriteComplete, TypeErasedBox{});
  EXPECT_EQ(
      hPtr->received(),
      std::vector<LTransportEvent>{LTransportEvent::WriteComplete});

  // A top-layer event the handler didn't subscribe to is not delivered, even
  // though it shares the same id space.
  pipeline->fireEvent(LRocketEvent::ConnClose, TypeErasedBox{});
  EXPECT_EQ(
      hPtr->received(),
      std::vector<LTransportEvent>{LTransportEvent::WriteComplete});
}

TEST(EventTest, HandlerListensToEventsFromOwnAndLowerLayers) {
  // A rocket-layer handler subscribes to BOTH a transport-layer event and its
  // own rocket-layer event. Each is delivered to the matching typed onEvent
  // overload, proving a handler can listen to every layer <= its own.
  folly::EventBase evb;
  MockHeadHandler head;
  MockTailHandler tail;
  TestAllocator alloc;

  auto h = std::make_unique<LayeredMultiHandler>();
  auto* hPtr = h.get();

  auto pipeline = PipelineBuilder<
                      MockHeadHandler,
                      MockTailHandler,
                      TestAllocator,
                      LRocketEvent>()
                      .setEventBase(&evb)
                      .setHead(&head)
                      .setTail(&tail)
                      .setAllocator(&alloc)
                      .addNextDuplex<LayeredMultiHandler>(a_tag, std::move(h))
                      .build();

  pipeline->fireEvent(LTransportEvent::WriteComplete, TypeErasedBox{});
  pipeline->fireEvent(LRocketEvent::ConnClose, TypeErasedBox{});
  pipeline->fireEvent(
      LTransportEvent::Heartbeat, TypeErasedBox{}); // unsubscribed

  EXPECT_EQ(
      hPtr->transport(),
      std::vector<LTransportEvent>{LTransportEvent::WriteComplete});
  EXPECT_EQ(hPtr->rocket(), std::vector<LRocketEvent>{LRocketEvent::ConnClose});
}

} // namespace apache::thrift::fast_thrift::channel_pipeline::test
