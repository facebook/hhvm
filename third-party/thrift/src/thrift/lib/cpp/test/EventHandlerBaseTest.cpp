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

#include <thrift/lib/cpp/EventHandlerBase.h>

#include <gtest/gtest.h>

#include <thrift/lib/cpp2/runtime/Init.h>

using namespace std;
using namespace folly;
using namespace apache::thrift;

class lulz : public exception {
 public:
  explicit lulz(string message) noexcept : message_(std::move(message)) {}
  const char* what() const noexcept override { return message_.c_str(); }

 private:
  string message_;
};

namespace {

class EventHandler : public TProcessorEventHandler {
 public:
  string ex_type;
  string ex_what;
  void userException(
      void*,
      std::string_view,
      std::string_view type,
      std::string_view what) override {
    ex_type = std::string(type);
    ex_what = std::string(what);
  }
};

template <class E>
std::exception_ptr to_eptr(const E& e) {
  try {
    throw e;
  } catch (E&) {
    return std::current_exception();
  }
}

class TProcessorEventHandlerTest : public testing::Test {};

class TClientTester : public TClientBase {
 public:
  static std::size_t count() {
    TClientTester tct;
    return tct.getEventHandlers().size();
  }
  static std::size_t countOf(
      const std::shared_ptr<TProcessorEventHandler>& handler) {
    TClientTester tct;
    return std::count(
        tct.getEventHandlers().begin(), tct.getEventHandlers().end(), handler);
  }
};

} // namespace

TEST_F(TProcessorEventHandlerTest, with_full_wrapped_eptr) {
  auto wrap = exception_wrapper(lulz("hello"));
  auto eptr = wrap.to_exception_ptr();
  EXPECT_TRUE(wrap.has_exception_ptr());
  EXPECT_EQ("lulz", wrap.class_name().toStdString());
  EXPECT_EQ("lulz: hello", wrap.what().toStdString());

  EventHandler eh;
  eh.userExceptionWrapped(nullptr, "", false, wrap);
  EXPECT_EQ("lulz", eh.ex_type);
  EXPECT_EQ("lulz: hello", eh.ex_what);
}

TEST_F(TProcessorEventHandlerTest, with_wrap_surprise) {
  auto wrap = exception_wrapper(lulz("hello"));
  EXPECT_EQ("lulz", wrap.class_name().toStdString());
  EXPECT_EQ("lulz: hello", wrap.what().toStdString());

  EventHandler eh;
  eh.userExceptionWrapped(nullptr, "", false, wrap);
  EXPECT_EQ("lulz", eh.ex_type);
  EXPECT_EQ("lulz: hello", eh.ex_what);
}

TEST_F(TProcessorEventHandlerTest, with_wrap_declared) {
  auto wrap = exception_wrapper(lulz("hello"));
  EXPECT_EQ("lulz", wrap.class_name().toStdString());
  EXPECT_EQ("lulz: hello", wrap.what().toStdString());

  EventHandler eh;
  eh.userExceptionWrapped(nullptr, "", true, wrap);
  EXPECT_EQ("lulz", eh.ex_type);
  EXPECT_EQ("hello", eh.ex_what);
}

TEST_F(TProcessorEventHandlerTest, registerProcessorHandler) {
  auto countProcessor = [](const auto& h) {
    return std::count(
        TProcessorBase::getHandlers().begin(),
        TProcessorBase::getHandlers().end(),
        h);
  };

  auto h = std::make_shared<EventHandler>();
  EXPECT_FALSE(countProcessor(h));
  EXPECT_FALSE(TClientTester::countOf(h));
  TProcessorBase::addProcessorEventHandler_deprecated(h);
  EXPECT_TRUE(countProcessor(h));
  EXPECT_FALSE(TClientTester::countOf(h));
  TClientBase::addClientEventHandler(h);
  EXPECT_TRUE(countProcessor(h));
  EXPECT_TRUE(TClientTester::countOf(h));
  TProcessorBase::removeProcessorEventHandler(h);
  EXPECT_FALSE(countProcessor(h));
  EXPECT_TRUE(TClientTester::countOf(h));
  TClientBase::removeClientEventHandler(h);
  EXPECT_FALSE(countProcessor(h));
  EXPECT_FALSE(TClientTester::countOf(h));
}

TEST_F(TProcessorEventHandlerTest, registrationActivity) {
  auto count = []() { return TProcessorBase::getHandlers().size(); };

  auto h1 = std::make_shared<EventHandler>();
  auto h2 = std::make_shared<EventHandler>();
  auto h3 = std::make_shared<EventHandler>();

  EXPECT_EQ(count(), 0);
  TProcessorBase::addProcessorEventHandler_deprecated(h1);
  EXPECT_EQ(count(), 1);
  TProcessorBase::removeProcessorEventHandler(h1);
  EXPECT_EQ(count(), 0);
  TProcessorBase::addProcessorEventHandler_deprecated(h1);
  EXPECT_EQ(count(), 1);
  TProcessorBase::addProcessorEventHandler_deprecated(h2);
  EXPECT_EQ(count(), 2);
  TProcessorBase::removeProcessorEventHandler(h1);
  EXPECT_EQ(count(), 1);
  TProcessorBase::addProcessorEventHandler_deprecated(h3);
  EXPECT_EQ(count(), 2);
  TProcessorBase::removeProcessorEventHandler(h3);
  EXPECT_EQ(count(), 1);
  TProcessorBase::removeProcessorEventHandler(h2);
  EXPECT_EQ(count(), 0);
}

TEST_F(TProcessorEventHandlerTest, RuntimeInitClientHandlers) {
  auto h1 = std::make_shared<EventHandler>();
  auto h2 = std::make_shared<EventHandler>();

  apache::thrift::runtime::InitOptions options;
  options.legacyClientEventHandlers.push_back(h1);
  apache::thrift::runtime::init(std::move(options));

  EXPECT_EQ(TClientTester::count(), 1);
  EXPECT_EQ(TClientTester::countOf(h1), 1);

  TClientBase::addClientEventHandler(h2);
  EXPECT_EQ(TClientTester::count(), 2);
  EXPECT_EQ(TClientTester::countOf(h1), 1);
  EXPECT_EQ(TClientTester::countOf(h2), 1);
}

class EventHandlerSharingTest : public testing::Test {
 protected:
  // Concrete subclass to test EventHandlerBase methods.
  class TestProcessor : public TProcessorBase {
   public:
    TestProcessor() : TProcessorBase(IgnoreGlobalEventHandlers{}) {}
    using EventHandlerBase::getEventHandlers;
    using EventHandlerBase::getEventHandlersSharedPtr;
  };

  static std::shared_ptr<
      const std::vector<std::shared_ptr<TProcessorEventHandler>>>
  makeSharedHandlers(
      std::initializer_list<std::shared_ptr<TProcessorEventHandler>> handlers) {
    return std::make_shared<
        const std::vector<std::shared_ptr<TProcessorEventHandler>>>(handlers);
  }
};

TEST_F(EventHandlerSharingTest, SetSharedEventHandlers) {
  auto h1 = std::make_shared<EventHandler>();
  auto h2 = std::make_shared<EventHandler>();
  auto shared = makeSharedHandlers({h1, h2});

  TestProcessor proc;
  proc.setSharedEventHandlers(shared);

  EXPECT_EQ(proc.getEventHandlers().size(), 2);
  EXPECT_EQ(proc.getEventHandlers()[0], h1);
  EXPECT_EQ(proc.getEventHandlers()[1], h2);
}

TEST_F(EventHandlerSharingTest, SharedHandlersShareSamePointer) {
  auto h1 = std::make_shared<EventHandler>();
  auto shared = makeSharedHandlers({h1});

  TestProcessor proc1;
  TestProcessor proc2;
  proc1.setSharedEventHandlers(shared);
  proc2.setSharedEventHandlers(shared);

  // Both processors should share the same underlying vector.
  EXPECT_EQ(
      proc1.getEventHandlersSharedPtr().get(),
      proc2.getEventHandlersSharedPtr().get());
}

TEST_F(EventHandlerSharingTest, CopyOnWriteAfterSharing) {
  auto h1 = std::make_shared<EventHandler>();
  auto h2 = std::make_shared<EventHandler>();
  auto shared = makeSharedHandlers({h1});

  TestProcessor proc1;
  TestProcessor proc2;
  proc1.setSharedEventHandlers(shared);
  proc2.setSharedEventHandlers(shared);

  // Adding a handler to proc1 should trigger COW — proc1 gets a private copy.
  proc1.addEventHandler(h2);

  // proc1 now has 2 handlers in its own copy.
  EXPECT_EQ(proc1.getEventHandlers().size(), 2);
  EXPECT_EQ(proc1.getEventHandlers()[0], h1);
  EXPECT_EQ(proc1.getEventHandlers()[1], h2);

  // proc2 still has the original shared vector with 1 handler.
  EXPECT_EQ(proc2.getEventHandlers().size(), 1);
  EXPECT_EQ(proc2.getEventHandlers()[0], h1);

  // The underlying pointers should now be different.
  EXPECT_NE(
      proc1.getEventHandlersSharedPtr().get(),
      proc2.getEventHandlersSharedPtr().get());
}

TEST_F(EventHandlerSharingTest, CopyOnWritePreservesOriginal) {
  auto h1 = std::make_shared<EventHandler>();
  auto h2 = std::make_shared<EventHandler>();
  auto shared = makeSharedHandlers({h1});

  // Keep a reference to the original shared vector.
  auto originalPtr = shared.get();

  TestProcessor proc;
  proc.setSharedEventHandlers(shared);
  proc.addEventHandler(h2);

  // The original shared vector should be unchanged.
  EXPECT_EQ(shared->size(), 1);
  EXPECT_EQ((*shared)[0], h1);
  EXPECT_EQ(shared.get(), originalPtr);
}

TEST_F(EventHandlerSharingTest, ClearEventHandlersResetsSharing) {
  auto h1 = std::make_shared<EventHandler>();
  auto shared = makeSharedHandlers({h1});

  TestProcessor proc;
  proc.setSharedEventHandlers(shared);
  EXPECT_EQ(proc.getEventHandlers().size(), 1);

  proc.clearEventHandlers();
  EXPECT_EQ(proc.getEventHandlers().size(), 0);

  // After clear, adding a handler should create a fresh vector, not COW.
  auto h2 = std::make_shared<EventHandler>();
  proc.addEventHandler(h2);
  EXPECT_EQ(proc.getEventHandlers().size(), 1);
  EXPECT_EQ(proc.getEventHandlers()[0], h2);
}

TEST_F(EventHandlerSharingTest, SetSharedReplacesExistingHandlers) {
  auto h1 = std::make_shared<EventHandler>();
  auto h2 = std::make_shared<EventHandler>();

  TestProcessor proc;
  proc.addEventHandler(h1);
  EXPECT_EQ(proc.getEventHandlers().size(), 1);

  // setSharedEventHandlers replaces the entire handler list.
  auto shared = makeSharedHandlers({h2});
  proc.setSharedEventHandlers(shared);
  EXPECT_EQ(proc.getEventHandlers().size(), 1);
  EXPECT_EQ(proc.getEventHandlers()[0], h2);
  // The underlying vector is the shared one (h1 is gone).
  EXPECT_EQ(proc.getEventHandlersSharedPtr().get(), shared.get());
}

TEST_F(EventHandlerSharingTest, SetSharedOnEmptyProcessorShares) {
  auto h1 = std::make_shared<EventHandler>();
  auto shared = makeSharedHandlers({h1});

  TestProcessor proc;
  proc.setSharedEventHandlers(shared);

  // Empty processor should share the vector directly.
  EXPECT_EQ(proc.getEventHandlers().size(), 1);
  EXPECT_EQ(proc.getEventHandlersSharedPtr().get(), shared.get());
}

TEST_F(EventHandlerSharingTest, PreMergedVectorSharedAcrossProcessors) {
  // Simulates the real flow: processors start with global handlers from
  // constructor, then setSharedEventHandlers replaces with a pre-merged
  // [globals + server-scoped] vector. All processors should share the same
  // underlying vector.
  auto globalH = std::make_shared<EventHandler>();
  auto serverH = std::make_shared<EventHandler>();
  auto preMerged = makeSharedHandlers({globalH, serverH});

  // Create multiple processors with pre-existing handlers (simulating
  // constructor-installed global handlers).
  TestProcessor proc1;
  TestProcessor proc2;
  TestProcessor proc3;
  proc1.addEventHandler(globalH);
  proc2.addEventHandler(globalH);
  proc3.addEventHandler(globalH);

  // setSharedEventHandlers replaces — all processors share the same vector.
  proc1.setSharedEventHandlers(preMerged);
  proc2.setSharedEventHandlers(preMerged);
  proc3.setSharedEventHandlers(preMerged);

  auto* underlying = preMerged.get();
  EXPECT_EQ(proc1.getEventHandlersSharedPtr().get(), underlying);
  EXPECT_EQ(proc2.getEventHandlersSharedPtr().get(), underlying);
  EXPECT_EQ(proc3.getEventHandlersSharedPtr().get(), underlying);

  // All have the complete pre-merged set.
  EXPECT_EQ(proc1.getEventHandlers().size(), 2);
  EXPECT_EQ(proc2.getEventHandlers().size(), 2);
  EXPECT_EQ(proc3.getEventHandlers().size(), 2);
}
