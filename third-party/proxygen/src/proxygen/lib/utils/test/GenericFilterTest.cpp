/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <deque>
#include <folly/Memory.h>
#include <folly/portability/GMock.h>
#include <folly/portability/GTest.h>
#include <map>
#include <proxygen/lib/utils/FilterChain.h>
#include <stdlib.h>

using namespace proxygen;
using namespace testing;

using std::unique_ptr;

namespace detail {

// This is defined in boost 1.53, but we only have 1.51 so far

template <typename T>
T* get_pointer(const unique_ptr<T>& ptr) {
  return ptr.get();
}

template <typename T>
T* get_pointer(T* ptr) {
  return ptr;
}

} // namespace detail

class TesterInterface {
 public:
  class Callback {
   public:
    virtual ~Callback() {
    }
    virtual void onA() = 0;
  };
  virtual ~TesterInterface() {
  }
  virtual void setCallback(Callback* cb) = 0;
  virtual void doA() = 0;
};

class MockTester : public TesterInterface {
 public:
  Callback* cb_{nullptr};
  void setCallback(Callback* cb) override {
    cb_ = cb;
  }
  MOCK_METHOD(void, doA, ());
};

class MockTesterCallback : public TesterInterface::Callback {
 public:
  MOCK_METHOD(void, onA, ());
};

template <bool Owned>
class TestFilter
    : public GenericFilter<TesterInterface,
                           TesterInterface::Callback,
                           &TesterInterface::setCallback,
                           Owned> {
 public:
  TestFilter()
      : GenericFilter<TesterInterface,
                      TesterInterface::Callback,
                      &TesterInterface::setCallback,
                      Owned>(true, true) {
  }

  TestFilter(bool calls, bool callbacks)
      : GenericFilter<TesterInterface,
                      TesterInterface::Callback,
                      &TesterInterface::setCallback,
                      Owned>(calls, callbacks) {
  }

  void setCallback(TesterInterface::Callback* cb) override {
    this->setCallbackInternal(cb);
  }
  void doA() override {
    do_++;
    this->call_->doA();
  }
  void onA() override {
    on_++;
    this->callback_->onA();
  }
  uint32_t do_{0};
  uint32_t on_{0};
  uint32_t id_{idCounter_++};
  static uint32_t idCounter_;
};
template <bool Owned>
uint32_t TestFilter<Owned>::idCounter_ = 0;

template <bool Owned>
class TestFilterNoCallback : public TestFilter<Owned> {
 public:
  TestFilterNoCallback() : TestFilter<Owned>(true, false) {
  }
};

template <bool Owned>
class TestFilterNoCall : public TestFilter<Owned> {
 public:
  TestFilterNoCall() : TestFilter<Owned>(false, true) {
  }
};

template <bool Owned>
class TestFilterNoCallbackNoCall : public TestFilter<Owned> {
 public:
  TestFilterNoCallbackNoCall() : TestFilter<Owned>(false, false) {
  }
};

template <bool Owned>
typename std::enable_if<Owned, unique_ptr<MockTester>>::type getTester() {
  return std::make_unique<MockTester>();
}

template <bool Owned>
typename std::enable_if<!Owned, MockTester*>::type getTester() {
  return new MockTester();
}

template <bool Owned>
class GenericFilterTest : public testing::Test {
 public:
  void basicTest();

  void testFilters(const std::deque<TestFilter<Owned>*>& filters,
                   MockTesterCallback* expectedCb);

  void SetUp() override {
    chain_ = std::make_unique<FilterChain<TesterInterface,
                                          TesterInterface::Callback,
                                          TestFilter<Owned>,
                                          &TesterInterface::setCallback,
                                          Owned>>(getTester<Owned>());
    chain().setCallback(&callback_);
    actor_ = CHECK_NOTNULL(static_cast<MockTester*>(chain_->call()));
  }
  FilterChain<TesterInterface,
              TesterInterface::Callback,
              TestFilter<Owned>,
              &TesterInterface::setCallback,
              Owned>&
  chain() {
    return *chain_;
  }

  template <typename FilterT>
  typename std::enable_if<!Owned, FilterT*>::type getFilter() {
    return new FilterT();
  }

  template <typename FilterT>
  typename std::enable_if<Owned, unique_ptr<FilterT>>::type getFilter() {
    return std::make_unique<FilterT>();
  }

  template <typename FilterT>
  void addFilterToChain(std::deque<TestFilter<Owned>*>& refs) {
    auto f = getFilter<FilterT>();
    refs.push_front(::detail::get_pointer(f));
    chain().addFilters(std::move(f));
  }

  std::deque<TestFilter<Owned>*> getRandomFilters(unsigned num) {
    std::deque<TestFilter<Owned>*> filters;
    srand(0);
    for (unsigned i = 0; i < num; ++i) {
      auto r = rand() % 4;
      if (r == 0) {
        addFilterToChain<TestFilter<Owned>>(filters);
      } else if (r == 1) {
        addFilterToChain<TestFilterNoCall<Owned>>(filters);
      } else if (r == 2) {
        addFilterToChain<TestFilterNoCallback<Owned>>(filters);
      } else if (r == 3) {
        addFilterToChain<TestFilterNoCallbackNoCall<Owned>>(filters);
      }
      basicTest();
    }
    return filters;
  }

  MockTesterCallback callback_;
  unique_ptr<FilterChain<TesterInterface,
                         TesterInterface::Callback,
                         TestFilter<Owned>,
                         &TesterInterface::setCallback,
                         Owned>>
      chain_;
  MockTester* actor_{nullptr};
};

template <bool Owned>
void GenericFilterTest<Owned>::basicTest() {
  InSequence enforceOrder;

  // Test call side
  EXPECT_CALL(*actor_, doA());
  chain()->doA();

  // Now poke the callback side
  EXPECT_CALL(callback_, onA());
  CHECK_NOTNULL(actor_->cb_);
  actor_->cb_->onA();
}

template <bool Owned>
void GenericFilterTest<Owned>::testFilters(
    const std::deque<TestFilter<Owned>*>& filters,
    MockTesterCallback* expectedCb) {
  for (auto f : filters) {
    f->do_ = 0;
    f->on_ = 0;
  }
  // Call
  EXPECT_CALL(*actor_, doA());
  chain()->doA();
  // Callback
  if (expectedCb) {
    EXPECT_CALL(*expectedCb, onA());
    CHECK_NOTNULL(actor_->cb_);
    actor_->cb_->onA();
  }
  for (auto f : filters) {
    if (f->kWantsCalls_) {
      EXPECT_EQ(f->do_, 1);
    } else {
      EXPECT_EQ(f->do_, 0);
    }
    if (f->kWantsCallbacks_) {
      EXPECT_EQ(f->on_, expectedCb ? 1 : 0);
    } else {
      EXPECT_EQ(f->on_, 0);
    }
  }
}

using OwnedGenericFilterTest = GenericFilterTest<true>;
using UnownedGenericFilterTest = GenericFilterTest<false>;

TEST_F(OwnedGenericFilterTest, EmptyChain) {
  basicTest();
}

TEST_F(OwnedGenericFilterTest, SingleElemChain) {
  auto filterUnique = std::make_unique<TestFilter<true>>();
  auto filter = filterUnique.get();
  chain().addFilters(std::move(filterUnique));
  EXPECT_EQ(filter->do_, 0);
  EXPECT_EQ(filter->on_, 0);
  basicTest();
  EXPECT_EQ(filter->do_, 1);
  EXPECT_EQ(filter->on_, 1);
}

TEST_F(OwnedGenericFilterTest, MultiElemChain) {
  auto f1 = std::make_unique<TestFilter<true>>();
  auto f2 = std::make_unique<TestFilter<true>>();
  auto f3 = std::make_unique<TestFilter<true>>();
  TestFilter<true>* fp1 = f1.get();
  TestFilter<true>* fp2 = f2.get();
  TestFilter<true>* fp3 = f3.get();
  chain().addFilters(std::move(f1), std::move(f2), std::move(f3));
  basicTest();
  EXPECT_EQ(fp1->do_, 1);
  EXPECT_EQ(fp1->on_, 1);
  EXPECT_EQ(fp2->do_, 1);
  EXPECT_EQ(fp2->on_, 1);
  EXPECT_EQ(fp3->do_, 1);
  EXPECT_EQ(fp3->on_, 1);
}

TEST_F(OwnedGenericFilterTest, MultiElemMultiAdd) {
  std::deque<TestFilter<true>*> filters;
  for (unsigned i = 0; i < 10; ++i) {
    auto filter = std::make_unique<TestFilter<true>>();
    filters.push_back(filter.get());
    chain().addFilters(std::move(filter));
  }
  basicTest();
  for (auto filter : filters) {
    EXPECT_EQ(filter->do_, 1);
    EXPECT_EQ(filter->on_, 1);
  }
}

TEST_F(OwnedGenericFilterTest, Wants) {
  auto f1 = std::make_unique<TestFilter<true>>();
  auto f2 = std::make_unique<TestFilterNoCallback<true>>();
  auto f3 = std::make_unique<TestFilterNoCall<true>>();
  auto f4 = std::make_unique<TestFilterNoCallbackNoCall<true>>();
  TestFilter<true>* fp1 = f1.get();
  TestFilter<true>* fp2 = f2.get();
  TestFilter<true>* fp3 = f3.get();
  TestFilter<true>* fp4 = f4.get();
  chain().addFilters(
      std::move(f1), std::move(f2), std::move(f3), std::move(f4));
  basicTest();
  EXPECT_EQ(fp1->do_, 1);
  EXPECT_EQ(fp1->on_, 1);
  // Only calls
  EXPECT_EQ(fp2->do_, 1);
  EXPECT_EQ(fp2->on_, 0);
  // Only callbacks
  EXPECT_EQ(fp3->do_, 0);
  EXPECT_EQ(fp3->on_, 1);
  // No callbacks or calls
  EXPECT_EQ(fp4->do_, 0);
  EXPECT_EQ(fp4->on_, 0);
}

TEST_F(OwnedGenericFilterTest, WantsMultiAdd) {
  auto f1 = std::make_unique<TestFilterNoCallback<true>>();
  auto f2 = std::make_unique<TestFilterNoCall<true>>();
  TestFilter<true>* fp1 = f1.get();
  TestFilter<true>* fp2 = f2.get();
  chain().addFilters(std::move(f1));
  basicTest();

  EXPECT_EQ(fp1->do_, 1);
  EXPECT_EQ(fp1->on_, 0);
  EXPECT_EQ(fp2->do_, 0);
  EXPECT_EQ(fp2->on_, 0);

  chain().addFilters(std::move(f2));
  basicTest();

  EXPECT_EQ(fp1->do_, 2);
  EXPECT_EQ(fp1->on_, 0);
  EXPECT_EQ(fp2->do_, 0);
  EXPECT_EQ(fp2->on_, 1);
}

TEST_F(OwnedGenericFilterTest, WantsMultiAddHard) {
  const unsigned NUM_FILTERS = 5000;
  auto filters = getRandomFilters(NUM_FILTERS);
  // Now check the counts on each filter. Filters are pushed to the front
  // of the chain, so filters towards the front have low call/callback counts
  for (unsigned i = 0; i < NUM_FILTERS; ++i) {
    auto f = filters[i];
    if (f->kWantsCalls_) {
      EXPECT_EQ(f->do_, i + 1);
    } else {
      EXPECT_EQ(f->do_, 0);
    }
    if (f->kWantsCallbacks_) {
      EXPECT_EQ(f->on_, i + 1);
    } else {
      EXPECT_EQ(f->on_, 0);
    }
  }
}

TEST_F(OwnedGenericFilterTest, ChangeCallback) {
  // The call-only filter in the chain doesn't want callbacks, so doing
  // chain()->setCallback() is an error! Instead, must use chain().setCallback()
  auto f = std::make_unique<TestFilterNoCallback<true>>();
  MockTesterCallback callback2;

  TestFilter<true>* fp = f.get();
  chain().addFilters(std::move(f));
  basicTest();

  EXPECT_EQ(fp->do_, 1);
  EXPECT_EQ(fp->on_, 0);

  chain().setCallback(&callback2);
  EXPECT_EQ(actor_->cb_, &callback2);
  EXPECT_CALL(callback2, onA());
  actor_->cb_->onA();

  EXPECT_EQ(fp->on_, 0);
}

TEST_F(UnownedGenericFilterTest, All) {
  const unsigned NUM_FILTERS = 5000;
  auto filters = getRandomFilters(NUM_FILTERS);
  // Now check the counts on each filter
  unsigned i = 0;
  for (auto f : filters) {
    if (f->kWantsCalls_) {
      EXPECT_EQ(f->do_, i + 1);
    } else {
      EXPECT_EQ(f->do_, 0);
    }
    if (f->kWantsCallbacks_) {
      EXPECT_EQ(f->on_, i + 1);
    } else {
      EXPECT_EQ(f->on_, 0);
    }
    delete f;
    ++i;
  }
  delete actor_;
}

TEST_F(OwnedGenericFilterTest, SetNullCb) {
  // Some objects have a special behavior when the callback is set to
  // nullptr. So in this case, we need to make sure it propagates
  auto filters = getRandomFilters(100);
  chain().setCallback(nullptr);
  CHECK(nullptr == actor_->cb_);

  testFilters(filters, nullptr);

  MockTesterCallback head;
  chain().setCallback(&head);

  testFilters(filters, &head);

  TesterInterface::Callback* cb = &head;
  for (auto f : filters) {
    if (f->kWantsCallbacks_) {
      cb = f;
    }
  }
  // The actor's callback should be the last filter in the chain that
  // wants callbacks
  ASSERT_EQ(actor_->cb_, cb);
}

// This class owns itself
class TestFilterOddDeleteDo : public TestFilter<false> {
 public:
  explicit TestFilterOddDeleteDo(int* deletions)
      : TestFilter<false>(true, true), deletions_(CHECK_NOTNULL(deletions)) {
  }
  ~TestFilterOddDeleteDo() override {
    ++*deletions_;
  }

  void doA() override {
    auto call = call_;
    if (id_ % 2) {
      delete this;
    } else if (times_++) {
      delete this;
    }
    call->doA();
  };
  unsigned times_{0};
  int* const deletions_;
};

TEST_F(UnownedGenericFilterTest, DeleteDo) {
  // Test where a filter in the middle of the chain deletes itself early
  int deletions = 0;

  for (unsigned i = 0; i < 4; ++i) {
    chain().addFilters(new TestFilterOddDeleteDo(&deletions));
  }

  for (unsigned i = 0; i < 2; ++i) {
    // First time around, the odd id's get deleted
    // Second time should just forward the calls normally
    EXPECT_CALL(*actor_, doA());
    chain()->doA();
    EXPECT_EQ(deletions, (i + 1) * 2);
  }
  basicTest();
  delete actor_;
}

template <bool Owned = false>
class TestFilterOddDeleteOn : public TestFilter<Owned> {
 public:
  explicit TestFilterOddDeleteOn(int* deletions)
      : deletions_(CHECK_NOTNULL(deletions)) {
  }
  ~TestFilterOddDeleteOn() override {
    ++*deletions_;
  }

  void onA() override {
    auto callback = this->callback_;
    if (this->id_ % 2) {
      delete this;
    } else if (times_++) {
      delete this;
    }
    callback->onA();
  };
  unsigned times_{0};
  int* const deletions_;
};

TEST_F(UnownedGenericFilterTest, DeleteOn) {
  // Test where a filter in the middle of the chain deletes itself early
  int deletions = 0;

  for (unsigned i = 0; i < 4; ++i) {
    chain().addFilters(new TestFilterOddDeleteOn<>(&deletions));
  }

  for (unsigned i = 0; i < 2; ++i) {
    // First time around, the odd id's get deleted
    // Second time should just forward the calls normally
    EXPECT_CALL(callback_, onA());
    actor_->cb_->onA();
    EXPECT_EQ(deletions, (i + 1) * 2);
  }
  basicTest();
  delete actor_;
}

TEST_F(OwnedGenericFilterTest, DeleteChain) {
  // Add some filters to the chain and reset the chain. Make sure all the
  // filters are deleted.
  const unsigned NUM_FILTERS = 1000;
  int deletions = 0;
  for (unsigned i = 0; i < NUM_FILTERS; ++i) {
    chain().addFilters(
        std::make_unique<TestFilterOddDeleteOn<true>>(&deletions));
  }
  chain_.reset();
  EXPECT_EQ(deletions, NUM_FILTERS);
}

TEST_F(OwnedGenericFilterTest, GetChainEnd) {
  for (unsigned i = 1; i < 100; ++i) {
    auto filters = getRandomFilters(i);
    EXPECT_EQ(actor_, &chain().getChainEnd());
  }
}

TEST_F(OwnedGenericFilterTest, SetDestination) {
  auto filters = getRandomFilters(20);
  EXPECT_CALL(*actor_, doA());
  chain()->doA();
  auto tester2 = getTester<true>();
  actor_ = tester2.get();
  auto oldTester = chain().setDestination(std::move(tester2));
  EXPECT_CALL(*actor_, doA());
  chain()->doA();
}

TEST_F(OwnedGenericFilterTest, Foreach) {
  auto filters = getRandomFilters(20);
  size_t count = 0;
  chain().foreach (
      [&count](GenericFilter<TesterInterface,
                             TesterInterface::Callback,
                             &TesterInterface::setCallback,
                             true,
                             std::default_delete<TesterInterface>>* filter) {
        if (dynamic_cast<TestFilter<true>*>(filter)) {
          count++;
        }
      });
  EXPECT_EQ(count, 20);
}
