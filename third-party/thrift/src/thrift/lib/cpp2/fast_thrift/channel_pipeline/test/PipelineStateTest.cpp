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

#include <functional>
#include <string>

#include <folly/ExceptionWrapper.h>
#include <folly/io/async/EventBase.h>
#include <folly/portability/GTest.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/PipelineBuilder.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/detail/TypedContext.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/test/MockAdapters.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/test/MockHandler.h>

using namespace apache::thrift::fast_thrift::channel_pipeline;
using namespace apache::thrift::fast_thrift::channel_pipeline::test;

namespace {

HANDLER_TAG(handler_a);
HANDLER_TAG(handler_b);
HANDLER_TAG(handler_c);
HANDLER_TAG(handler_d);

struct CounterState {
  int value{0};
};

struct LabelState {
  std::string label;
};

struct ConfigState {
  explicit ConfigState(int initial, std::string name)
      : val(initial), label(std::move(name)) {}
  int val;
  std::string label;
};

/**
 * Minimal duplex handler that reaches a single pipeline-level state type
 * through the typed context. The read/write hooks receive the state by
 * reference so test bodies never need to name the context type.
 */
template <typename StateT>
struct StateHandler {
  std::function<void(StateT&)> onReadHook;
  std::function<void(StateT&)> onWriteHook;
  std::function<void(StateT&)> onAddedHook;

  template <typename Context>
  Result onRead(Context& ctx, TypeErasedBox&& msg) noexcept {
    if (onReadHook) {
      onReadHook(ctx.template state<StateT>());
    }
    return ctx.fireRead(std::move(msg));
  }

  template <typename Context>
  Result onWrite(Context& ctx, TypeErasedBox&& msg) noexcept {
    if (onWriteHook) {
      onWriteHook(ctx.template state<StateT>());
    }
    return ctx.fireWrite(std::move(msg));
  }

  template <typename Context>
  void onReadReady(Context&) noexcept {}
  template <typename Context>
  void onWriteReady(Context&) noexcept {}
  template <typename Context>
  void onPipelineActive(Context&) noexcept {}
  template <typename Context>
  void onPipelineInactive(Context&) noexcept {}
  template <typename Context>
  void onException(Context& ctx, folly::exception_wrapper&& e) noexcept {
    ctx.fireException(std::move(e));
  }
  template <typename Context>
  void handlerAdded(Context& ctx) noexcept {
    if (onAddedHook) {
      onAddedHook(ctx.template state<StateT>());
    }
  }
  template <typename Context>
  void handlerRemoved(Context&) noexcept {}
};

// Concept that is satisfied iff ctx.state<T>() is a well-formed expression.
// Used to prove that access to an unregistered state type is rejected at
// compile time (SFINAE), not silently reinterpreted.
template <typename Ctx, typename T>
concept CanAccessState = requires(Ctx ctx) { ctx.template state<T>(); };

// Concept satisfied iff the const ContextImpl accessors are reachable through a
// const Ctx. TypedContext is a non-owning view, so a const view must forward to
// the const accessors exactly as a plain ContextImpl& would.
template <typename Ctx>
concept ConstReachesConstAccessors = requires(const Ctx ctx) {
  ctx.handlerId();
  ctx.handlerIndex();
};

} // namespace

class PipelineStateTest : public ::testing::Test {
 protected:
  folly::EventBase evb_;
  MockHeadHandler head_;
  MockTailHandler tail_;
  TestAllocator allocator_;
};

// ---------------------------------------------------------------------------
// Compile-time rejection (the headline safety property of the mechanism):
// state<T>() is well-formed only for a T registered via addState<T>().
// ---------------------------------------------------------------------------

TEST_F(PipelineStateTest, UnregisteredStateTypeIsRejectedAtCompileTime) {
  using OneState = detail::TypedContext<std::tuple<CounterState>>;
  static_assert(CanAccessState<OneState, CounterState>);
  static_assert(!CanAccessState<OneState, LabelState>);

  using TwoStates = detail::TypedContext<std::tuple<CounterState, LabelState>>;
  static_assert(CanAccessState<TwoStates, CounterState>);
  static_assert(CanAccessState<TwoStates, LabelState>);
  static_assert(!CanAccessState<TwoStates, ConfigState>);
}

// The forwarded ContextImpl API must be callable through a const TypedContext&,
// so a const view reaches the const accessors (handlerId, handlerIndex) just as
// a plain ContextImpl& would. Regression guard for the forwarders being
// const-qualified; a bare ContextImpl trivially satisfies this too.
TEST_F(PipelineStateTest, ConstTypedContextReachesConstAccessors) {
  using OneState = detail::TypedContext<std::tuple<CounterState>>;
  static_assert(ConstReachesConstAccessors<OneState>);
  static_assert(ConstReachesConstAccessors<detail::ContextImpl>);
}

// ---------------------------------------------------------------------------
// Multiple coexisting state types are independently accessible and isolated.
// ---------------------------------------------------------------------------

TEST_F(PipelineStateTest, MultipleStateTypesAreIndependent) {
  // handler_a writes CounterState, handler_b writes LabelState; handler_c
  // reads both back. Proves the two registered types are distinct slots,
  // each visible across handlers, neither disturbing the other.
  auto counterWriter = std::make_unique<StateHandler<CounterState>>();
  counterWriter->onReadHook = [](CounterState& s) { s.value = 7; };
  auto labelWriter = std::make_unique<StateHandler<LabelState>>();
  labelWriter->onReadHook = [](LabelState& s) { s.label = "seven"; };

  int observedCounter = -1;
  std::string observedLabel;
  auto counterReader = std::make_unique<StateHandler<CounterState>>();
  counterReader->onReadHook = [&](CounterState& s) {
    observedCounter = s.value;
  };
  auto labelReader = std::make_unique<StateHandler<LabelState>>();
  labelReader->onReadHook = [&](LabelState& s) { observedLabel = s.label; };

  auto pipeline =
      PipelineBuilder<MockHeadHandler, MockTailHandler, TestAllocator>()
          .setEventBase(&evb_)
          .setHead(&head_)
          .setTail(&tail_)
          .setAllocator(&allocator_)
          .addState<CounterState>()
          .addState<LabelState>()
          .addNextDuplex<StateHandler<CounterState>>(
              handler_a_tag, std::move(counterWriter))
          .addNextDuplex<StateHandler<LabelState>>(
              handler_b_tag, std::move(labelWriter))
          .addNextDuplex<StateHandler<CounterState>>(
              handler_c_tag, std::move(counterReader))
          .addNextDuplex<StateHandler<LabelState>>(
              handler_d_tag, std::move(labelReader))
          .build();

  (void)pipeline->fireRead(erase_and_box(folly::IOBuf::create(0)));

  EXPECT_EQ(observedCounter, 7);
  EXPECT_EQ(observedLabel, "seven");
}

// ---------------------------------------------------------------------------
// Behavioral contract (preserved from the single-type mechanism).
// ---------------------------------------------------------------------------

TEST_F(PipelineStateTest, HandlerCanAccessState) {
  int seen = -1;
  auto handler = std::make_unique<StateHandler<CounterState>>();
  handler->onReadHook = [&](CounterState& s) { seen = s.value; };

  auto pipeline =
      PipelineBuilder<MockHeadHandler, MockTailHandler, TestAllocator>()
          .setEventBase(&evb_)
          .setHead(&head_)
          .setTail(&tail_)
          .setAllocator(&allocator_)
          .addState<CounterState>()
          .addNextDuplex<StateHandler<CounterState>>(
              handler_a_tag, std::move(handler))
          .build();

  (void)pipeline->fireRead(erase_and_box(folly::IOBuf::create(0)));
  EXPECT_EQ(seen, 0);
}

TEST_F(PipelineStateTest, StateModificationsVisibleAcrossHandlers) {
  auto writer = std::make_unique<StateHandler<CounterState>>();
  writer->onReadHook = [](CounterState& s) { s.value = 42; };

  int seen = -1;
  auto reader = std::make_unique<StateHandler<CounterState>>();
  reader->onReadHook = [&](CounterState& s) { seen = s.value; };

  auto pipeline =
      PipelineBuilder<MockHeadHandler, MockTailHandler, TestAllocator>()
          .setEventBase(&evb_)
          .setHead(&head_)
          .setTail(&tail_)
          .setAllocator(&allocator_)
          .addState<CounterState>()
          .addNextDuplex<StateHandler<CounterState>>(
              handler_a_tag, std::move(writer))
          .addNextDuplex<StateHandler<CounterState>>(
              handler_b_tag, std::move(reader))
          .build();

  (void)pipeline->fireRead(erase_and_box(folly::IOBuf::create(0)));
  EXPECT_EQ(seen, 42);
}

TEST_F(PipelineStateTest, StatePersistsAcrossMessages) {
  auto handler = std::make_unique<StateHandler<CounterState>>();
  handler->onReadHook = [](CounterState& s) { s.value++; };

  int seen = -1;
  auto reader = std::make_unique<StateHandler<CounterState>>();
  reader->onReadHook = [&](CounterState& s) { seen = s.value; };

  auto pipeline =
      PipelineBuilder<MockHeadHandler, MockTailHandler, TestAllocator>()
          .setEventBase(&evb_)
          .setHead(&head_)
          .setTail(&tail_)
          .setAllocator(&allocator_)
          .addState<CounterState>()
          .addNextDuplex<StateHandler<CounterState>>(
              handler_a_tag, std::move(handler))
          .addNextDuplex<StateHandler<CounterState>>(
              handler_b_tag, std::move(reader))
          .build();

  (void)pipeline->fireRead(erase_and_box(folly::IOBuf::create(0)));
  (void)pipeline->fireRead(erase_and_box(folly::IOBuf::create(0)));
  (void)pipeline->fireRead(erase_and_box(folly::IOBuf::create(0)));
  EXPECT_EQ(seen, 3);
}

TEST_F(PipelineStateTest, StateAccessibleFromWritePath) {
  auto handler = std::make_unique<StateHandler<CounterState>>();
  handler->onReadHook = [](CounterState& s) { s.value = 99; };
  int seenOnWrite = -1;
  handler->onWriteHook = [&](CounterState& s) { seenOnWrite = s.value; };

  head_.setOnWriteCallback([](TypeErasedBox&&) { return Result::Success; });

  auto pipeline =
      PipelineBuilder<MockHeadHandler, MockTailHandler, TestAllocator>()
          .setEventBase(&evb_)
          .setHead(&head_)
          .setTail(&tail_)
          .setAllocator(&allocator_)
          .addState<CounterState>()
          .addNextDuplex<StateHandler<CounterState>>(
              handler_a_tag, std::move(handler))
          .build();

  (void)pipeline->fireRead(erase_and_box(folly::IOBuf::create(0)));
  (void)pipeline->fireWrite(erase_and_box(folly::IOBuf::create(0)));
  EXPECT_EQ(seenOnWrite, 99);
}

TEST_F(PipelineStateTest, StateConstructedWithArguments) {
  int seenVal = -1;
  std::string seenLabel;
  auto handler = std::make_unique<StateHandler<ConfigState>>();
  handler->onReadHook = [&](ConfigState& s) {
    seenVal = s.val;
    seenLabel = s.label;
  };

  auto pipeline =
      PipelineBuilder<MockHeadHandler, MockTailHandler, TestAllocator>()
          .setEventBase(&evb_)
          .setHead(&head_)
          .setTail(&tail_)
          .setAllocator(&allocator_)
          .addState<ConfigState>(100, "test")
          .addNextDuplex<StateHandler<ConfigState>>(
              handler_a_tag, std::move(handler))
          .build();

  (void)pipeline->fireRead(erase_and_box(folly::IOBuf::create(0)));
  EXPECT_EQ(seenVal, 100);
  EXPECT_EQ(seenLabel, "test");
}

TEST_F(PipelineStateTest, StateCleanedUpOnPipelineDestruction) {
  // Live-instance counter (robust to the moves that carry the state through
  // tuple accumulation into the pipeline-owned allocation): a moved-from
  // instance nulls its pointer so only the surviving owner decrements.
  int live = 0;
  struct Tracked {
    int* live;
    explicit Tracked(int* l) : live(l) { ++*live; }
    Tracked(Tracked&& o) noexcept : live(o.live) { o.live = nullptr; }
    Tracked(const Tracked&) = delete;
    Tracked& operator=(Tracked&&) = delete;
    Tracked& operator=(const Tracked&) = delete;
    ~Tracked() {
      if (live) {
        --*live;
      }
    }
  };

  {
    auto handler = std::make_unique<MockHandler>();
    auto pipeline =
        PipelineBuilder<MockHeadHandler, MockTailHandler, TestAllocator>()
            .setEventBase(&evb_)
            .setHead(&head_)
            .setTail(&tail_)
            .setAllocator(&allocator_)
            .addState<Tracked>(&live)
            .addNextDuplex<MockHandler>(handler_a_tag, std::move(handler))
            .build();
    // Exactly one live instance, owned by the pipeline.
    EXPECT_EQ(live, 1);
  }

  // Destroyed together with the pipeline.
  EXPECT_EQ(live, 0);
}

TEST_F(PipelineStateTest, PipelineWithoutStateWorks) {
  auto handler = std::make_unique<MockHandler>();
  auto* handlerPtr = handler.get();
  auto pipeline =
      PipelineBuilder<MockHeadHandler, MockTailHandler, TestAllocator>()
          .setEventBase(&evb_)
          .setHead(&head_)
          .setTail(&tail_)
          .setAllocator(&allocator_)
          .addNextDuplex<MockHandler>(handler_a_tag, std::move(handler))
          .build();

  (void)pipeline->fireRead(erase_and_box(folly::IOBuf::create(0)));
  EXPECT_EQ(handlerPtr->readCount(), 1);
}

// State must be registered before any handler; interleaving addState after a
// handler is rejected so every handler sees the final registered state set.
TEST_F(PipelineStateTest, AddStateAfterHandlerThrows) {
  auto handler = std::make_unique<MockHandler>();
  auto interleave = [&] {
    return PipelineBuilder<MockHeadHandler, MockTailHandler, TestAllocator>()
        .setEventBase(&evb_)
        .setHead(&head_)
        .setTail(&tail_)
        .setAllocator(&allocator_)
        .addNextDuplex<MockHandler>(handler_a_tag, std::move(handler))
        .addState<CounterState>();
  };
  EXPECT_THROW(interleave(), std::runtime_error);
}

// State is a pipeline property fixed before any handler is added, so state<T>()
// is available in lifecycle callbacks too — not just the message path. Here a
// handler writes state in handlerAdded and a later handler reads it on the
// first message.
TEST_F(PipelineStateTest, StateAccessibleInLifecycleCallback) {
  auto writer = std::make_unique<StateHandler<CounterState>>();
  writer->onAddedHook = [](CounterState& s) { s.value = 123; };

  int seen = -1;
  auto reader = std::make_unique<StateHandler<CounterState>>();
  reader->onReadHook = [&](CounterState& s) { seen = s.value; };

  auto pipeline =
      PipelineBuilder<MockHeadHandler, MockTailHandler, TestAllocator>()
          .setEventBase(&evb_)
          .setHead(&head_)
          .setTail(&tail_)
          .setAllocator(&allocator_)
          .addState<CounterState>()
          .addNextDuplex<StateHandler<CounterState>>(
              handler_a_tag, std::move(writer))
          .addNextDuplex<StateHandler<CounterState>>(
              handler_b_tag, std::move(reader))
          .build();

  (void)pipeline->fireRead(erase_and_box(folly::IOBuf::create(0)));
  EXPECT_EQ(seen, 123);
}
