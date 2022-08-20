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

#include <exception>

#include <folly/CPortability.h>
#include <folly/MapUtil.h>
#include <folly/portability/GTest.h>

#include <thrift/lib/cpp2/Flags.h>
#include <thrift/lib/cpp2/PluggableFunction.h>

#include <folly/experimental/observer/SimpleObservable.h>

THRIFT_FLAG_DEFINE_bool(test_flag_bool, true);
THRIFT_FLAG_DEFINE_int64(test_flag_int, 42);
THRIFT_FLAG_DECLARE_bool(test_flag_bool_external);
THRIFT_FLAG_DECLARE_int64(test_flag_int_external);

class TestFlagsBackend : public apache::thrift::detail::FlagsBackend {
 public:
  folly::observer::Observer<folly::Optional<bool>> getFlagObserverBool(
      folly::StringPiece name) override {
    return getFlagObservableBool(name).getObserver();
  }

  folly::observer::Observer<folly::Optional<int64_t>> getFlagObserverInt64(
      folly::StringPiece name) override {
    return getFlagObservableInt64(name).getObserver();
  }

  folly::observer::SimpleObservable<folly::Optional<bool>>&
  getFlagObservableBool(folly::StringPiece name) {
    if (auto observablePtr = folly::get_ptr(boolObservables_, name.str())) {
      return **observablePtr;
    }
    return *(
        boolObservables_[name.str()] = std::make_unique<
            folly::observer::SimpleObservable<folly::Optional<bool>>>(
            folly::Optional<bool>{}));
  }

  folly::observer::SimpleObservable<folly::Optional<int64_t>>&
  getFlagObservableInt64(folly::StringPiece name) {
    if (auto observablePtr = folly::get_ptr(int64Observables_, name.str())) {
      return **observablePtr;
    }
    return *(
        int64Observables_[name.str()] = std::make_unique<
            folly::observer::SimpleObservable<folly::Optional<int64_t>>>(
            folly::Optional<int64_t>{}));
  }

 private:
  std::unordered_map<
      std::string,
      std::unique_ptr<folly::observer::SimpleObservable<folly::Optional<bool>>>>
      boolObservables_;
  std::unordered_map<
      std::string,
      std::unique_ptr<
          folly::observer::SimpleObservable<folly::Optional<int64_t>>>>
      int64Observables_;
};

namespace {
TestFlagsBackend* testBackendPtr;
bool useDummyBackend{false};
} // namespace

namespace apache::thrift::detail {

THRIFT_PLUGGABLE_FUNC_SET(
    std::unique_ptr<apache::thrift::detail::FlagsBackend>, createFlagsBackend) {
  if (useDummyBackend) {
    return {};
  }
  auto testBackend = std::make_unique<TestFlagsBackend>();
  testBackendPtr = testBackend.get();
  return testBackend;
}

} // namespace apache::thrift::detail

TEST(Flags, Get) {
  EXPECT_EQ(true, THRIFT_FLAG(test_flag_bool));
  EXPECT_EQ(42, THRIFT_FLAG(test_flag_int));
  EXPECT_EQ(true, THRIFT_FLAG(test_flag_bool_external));
  EXPECT_EQ(42, THRIFT_FLAG(test_flag_int_external));

  testBackendPtr->getFlagObservableBool("test_flag_bool").setValue(false);
  testBackendPtr->getFlagObservableInt64("test_flag_int").setValue(41);
  testBackendPtr->getFlagObservableBool("test_flag_bool_external")
      .setValue(false);
  testBackendPtr->getFlagObservableInt64("test_flag_int_external").setValue(41);

  folly::observer_detail::ObserverManager::waitForAllUpdates();

  EXPECT_EQ(false, THRIFT_FLAG(test_flag_bool));
  EXPECT_EQ(41, THRIFT_FLAG(test_flag_int));
  EXPECT_EQ(false, THRIFT_FLAG(test_flag_bool_external));
  EXPECT_EQ(41, THRIFT_FLAG(test_flag_int_external));
}

TEST(Flags, Observe) {
  auto test_flag_bool_observer = THRIFT_FLAG_OBSERVE(test_flag_bool);
  auto test_flag_int_observer = THRIFT_FLAG_OBSERVE(test_flag_int);
  auto test_flag_bool_external_observer =
      THRIFT_FLAG_OBSERVE(test_flag_bool_external);
  auto test_flag_int_extenal_observer =
      THRIFT_FLAG_OBSERVE(test_flag_int_external);
  EXPECT_EQ(true, **test_flag_bool_observer);
  EXPECT_EQ(42, **test_flag_int_observer);
  EXPECT_EQ(true, **test_flag_bool_external_observer);
  EXPECT_EQ(42, **test_flag_int_extenal_observer);

  testBackendPtr->getFlagObservableBool("test_flag_bool").setValue(false);
  testBackendPtr->getFlagObservableInt64("test_flag_int").setValue(41);
  testBackendPtr->getFlagObservableBool("test_flag_bool_external")
      .setValue(false);
  testBackendPtr->getFlagObservableInt64("test_flag_int_external").setValue(41);

  folly::observer_detail::ObserverManager::waitForAllUpdates();

  EXPECT_EQ(false, **test_flag_bool_observer);
  EXPECT_EQ(41, **test_flag_int_observer);
  EXPECT_EQ(false, **test_flag_bool_external_observer);
  EXPECT_EQ(41, **test_flag_int_extenal_observer);
}

TEST(Flags, NoBackend) {
  useDummyBackend = true;

  EXPECT_EQ(true, THRIFT_FLAG(test_flag_bool));
  EXPECT_EQ(42, THRIFT_FLAG(test_flag_int));
  EXPECT_EQ(true, THRIFT_FLAG(test_flag_bool_external));
  EXPECT_EQ(42, THRIFT_FLAG(test_flag_int_external));
}

TEST(Flags, MockGet) {
  EXPECT_EQ(true, THRIFT_FLAG(test_flag_bool));
  EXPECT_EQ(42, THRIFT_FLAG(test_flag_int));
  EXPECT_EQ(true, THRIFT_FLAG(test_flag_bool_external));
  EXPECT_EQ(42, THRIFT_FLAG(test_flag_int_external));

  THRIFT_FLAG_SET_MOCK(test_flag_bool, false);
  THRIFT_FLAG_SET_MOCK(test_flag_int, 41);
  THRIFT_FLAG_SET_MOCK(test_flag_bool_external, false);
  THRIFT_FLAG_SET_MOCK(test_flag_int_external, 41);

  folly::observer_detail::ObserverManager::waitForAllUpdates();

  EXPECT_EQ(false, THRIFT_FLAG(test_flag_bool));
  EXPECT_EQ(41, THRIFT_FLAG(test_flag_int));
  EXPECT_EQ(false, THRIFT_FLAG(test_flag_bool_external));
  EXPECT_EQ(41, THRIFT_FLAG(test_flag_int_external));

  THRIFT_FLAG_SET_MOCK(test_flag_bool, true);
  THRIFT_FLAG_SET_MOCK(test_flag_int, 9);
  THRIFT_FLAG_SET_MOCK(test_flag_bool_external, true);
  THRIFT_FLAG_SET_MOCK(test_flag_int_external, 61);

  folly::observer_detail::ObserverManager::waitForAllUpdates();

  EXPECT_EQ(true, THRIFT_FLAG(test_flag_bool));
  EXPECT_EQ(9, THRIFT_FLAG(test_flag_int));
  EXPECT_EQ(true, THRIFT_FLAG(test_flag_bool_external));
  EXPECT_EQ(61, THRIFT_FLAG(test_flag_int_external));
}

TEST(Flags, MockObserve) {
  EXPECT_EQ(true, THRIFT_FLAG(test_flag_bool));
  EXPECT_EQ(42, THRIFT_FLAG(test_flag_int));
  EXPECT_EQ(true, THRIFT_FLAG(test_flag_bool_external));
  EXPECT_EQ(42, THRIFT_FLAG(test_flag_int_external));

  auto test_flag_bool_observer = THRIFT_FLAG_OBSERVE(test_flag_bool);
  auto test_flag_int_observer = THRIFT_FLAG_OBSERVE(test_flag_int);
  auto test_flag_bool_external_observer =
      THRIFT_FLAG_OBSERVE(test_flag_bool_external);
  auto test_flag_int_extenal_observer =
      THRIFT_FLAG_OBSERVE(test_flag_int_external);

  THRIFT_FLAG_SET_MOCK(test_flag_bool, false);
  THRIFT_FLAG_SET_MOCK(test_flag_int, 41);
  THRIFT_FLAG_SET_MOCK(test_flag_bool_external, false);
  THRIFT_FLAG_SET_MOCK(test_flag_int_external, 41);

  folly::observer_detail::ObserverManager::waitForAllUpdates();

  EXPECT_EQ(false, **test_flag_bool_observer);
  EXPECT_EQ(41, **test_flag_int_observer);
  EXPECT_EQ(false, **test_flag_bool_external_observer);
  EXPECT_EQ(41, **test_flag_int_extenal_observer);

  THRIFT_FLAG_SET_MOCK(test_flag_bool, true);
  THRIFT_FLAG_SET_MOCK(test_flag_int, 9);
  THRIFT_FLAG_SET_MOCK(test_flag_bool_external, true);
  THRIFT_FLAG_SET_MOCK(test_flag_int_external, 61);

  folly::observer_detail::ObserverManager::waitForAllUpdates();

  EXPECT_EQ(true, **test_flag_bool_observer);
  EXPECT_EQ(9, **test_flag_int_observer);
  EXPECT_EQ(true, **test_flag_bool_external_observer);
  EXPECT_EQ(61, **test_flag_int_extenal_observer);
}

TEST(Flags, MockValuePreferred) {
  EXPECT_EQ(true, THRIFT_FLAG(test_flag_bool));
  EXPECT_EQ(42, THRIFT_FLAG(test_flag_int));
  EXPECT_EQ(true, THRIFT_FLAG(test_flag_bool_external));
  EXPECT_EQ(42, THRIFT_FLAG(test_flag_int_external));

  THRIFT_FLAG_SET_MOCK(test_flag_bool, true);
  THRIFT_FLAG_SET_MOCK(test_flag_int, 73);
  THRIFT_FLAG_SET_MOCK(test_flag_bool_external, true);
  THRIFT_FLAG_SET_MOCK(test_flag_int_external, 49);

  folly::observer_detail::ObserverManager::waitForAllUpdates();

  testBackendPtr->getFlagObservableBool("test_flag_bool").setValue(false);
  testBackendPtr->getFlagObservableInt64("test_flag_int").setValue(41);
  testBackendPtr->getFlagObservableBool("test_flag_bool_external")
      .setValue(false);
  testBackendPtr->getFlagObservableInt64("test_flag_int_external").setValue(41);

  folly::observer_detail::ObserverManager::waitForAllUpdates();

  EXPECT_EQ(true, THRIFT_FLAG(test_flag_bool));
  EXPECT_EQ(73, THRIFT_FLAG(test_flag_int));
  EXPECT_EQ(true, THRIFT_FLAG(test_flag_bool_external));
  EXPECT_EQ(49, THRIFT_FLAG(test_flag_int_external));
}
