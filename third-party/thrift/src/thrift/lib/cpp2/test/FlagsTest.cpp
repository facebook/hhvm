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

#include <gtest/gtest.h>
#include <folly/MapUtil.h>

#include <thrift/lib/cpp2/Flags.h>
#include <thrift/lib/cpp2/PluggableFunction.h>

#include <folly/observer/SimpleObservable.h>

THRIFT_FLAG_DEFINE_bool(test_flag_bool, true);
THRIFT_FLAG_DEFINE_int64(test_flag_int, 42);
THRIFT_FLAG_DEFINE_string(test_flag_string, "foo");
THRIFT_FLAG_DECLARE_bool(test_flag_bool_external);
THRIFT_FLAG_DECLARE_int64(test_flag_int_external);
THRIFT_FLAG_DECLARE_string(test_flag_string_external);

class TestFlagsBackend : public apache::thrift::detail::FlagsBackend {
 public:
  folly::observer::Observer<std::optional<bool>> getFlagObserverBool(
      std::string_view name) override {
    return getFlagObservableBool(name).getObserver();
  }

  folly::observer::Observer<std::optional<int64_t>> getFlagObserverInt64(
      std::string_view name) override {
    return getFlagObservableInt64(name).getObserver();
  }

  folly::observer::Observer<std::optional<std::string>> getFlagObserverString(
      std::string_view name) override {
    return getFlagObservableString(name).getObserver();
  }

  folly::observer::SimpleObservable<std::optional<bool>>& getFlagObservableBool(
      std::string_view name) {
    return getFlagObservable<bool>(boolObservables_, name);
  }

  folly::observer::SimpleObservable<std::optional<int64_t>>&
  getFlagObservableInt64(std::string_view name) {
    return getFlagObservable<int64_t>(int64Observables_, name);
  }

  folly::observer::SimpleObservable<std::optional<std::string>>&
  getFlagObservableString(std::string_view name) {
    return getFlagObservable<std::string>(stringObservables_, name);
  }

  void reset() {
    for (auto& [_, observable] : boolObservables_) {
      observable->setValue(std::nullopt);
    }
    for (auto& [_, observable] : int64Observables_) {
      observable->setValue(std::nullopt);
    }
    for (auto& [_, observable] : stringObservables_) {
      observable->setValue(std::nullopt);
    }
  }

 private:
  template <typename T>
  using ObservablesMap = std::unordered_map<
      std::string,
      std::unique_ptr<folly::observer::SimpleObservable<std::optional<T>>>>;
  ObservablesMap<bool> boolObservables_;
  ObservablesMap<int64_t> int64Observables_;
  ObservablesMap<std::string> stringObservables_;

  template <typename T>
  static folly::observer::SimpleObservable<std::optional<T>>& getFlagObservable(
      ObservablesMap<T>& observables, std::string_view name) {
    auto nameStr = std::string{name};
    if (auto observablePtr = folly::get_ptr(observables, nameStr)) {
      return **observablePtr;
    }
    return *(
        observables[nameStr] = std::make_unique<
            folly::observer::SimpleObservable<std::optional<T>>>(
            std::optional<T>{}));
  }
};

namespace {
TestFlagsBackend* testBackendPtr = nullptr;
bool useDummyBackend{false};

struct Flags : testing::Test {
  void TearDown() override {
    if (useDummyBackend) {
      return;
    }

    testBackendPtr->reset();

    THRIFT_FLAG_UNMOCK(test_flag_bool);
    THRIFT_FLAG_UNMOCK(test_flag_int);
    THRIFT_FLAG_UNMOCK(test_flag_string);
    THRIFT_FLAG_UNMOCK(test_flag_bool_external);
    THRIFT_FLAG_UNMOCK(test_flag_int_external);
    THRIFT_FLAG_UNMOCK(test_flag_string_external);

    folly::observer_detail::ObserverManager::waitForAllUpdates();
  }
};
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

TEST_F(Flags, Get) {
  EXPECT_EQ(true, THRIFT_FLAG(test_flag_bool));
  EXPECT_EQ(42, THRIFT_FLAG(test_flag_int));
  EXPECT_EQ("foo", THRIFT_FLAG(test_flag_string));
  EXPECT_EQ(true, THRIFT_FLAG(test_flag_bool_external));
  EXPECT_EQ(42, THRIFT_FLAG(test_flag_int_external));
  EXPECT_EQ("foo", THRIFT_FLAG(test_flag_string_external));

  testBackendPtr->getFlagObservableBool("test_flag_bool").setValue(false);
  testBackendPtr->getFlagObservableInt64("test_flag_int").setValue(41);
  testBackendPtr->getFlagObservableString("test_flag_string")
      .setValue(std::string{"bar"});
  testBackendPtr->getFlagObservableBool("test_flag_bool_external")
      .setValue(false);
  testBackendPtr->getFlagObservableInt64("test_flag_int_external").setValue(41);
  testBackendPtr->getFlagObservableString("test_flag_string_external")
      .setValue(std::string{"bar"});

  folly::observer_detail::ObserverManager::waitForAllUpdates();

  EXPECT_EQ(false, THRIFT_FLAG(test_flag_bool));
  EXPECT_EQ(41, THRIFT_FLAG(test_flag_int));
  EXPECT_EQ("bar", THRIFT_FLAG(test_flag_string));
  EXPECT_EQ(false, THRIFT_FLAG(test_flag_bool_external));
  EXPECT_EQ(41, THRIFT_FLAG(test_flag_int_external));
  EXPECT_EQ("bar", THRIFT_FLAG(test_flag_string_external));
}

TEST_F(Flags, Observe) {
  auto test_flag_bool_observer = THRIFT_FLAG_OBSERVE(test_flag_bool);
  auto test_flag_int_observer = THRIFT_FLAG_OBSERVE(test_flag_int);
  auto test_flag_string_observer = THRIFT_FLAG_OBSERVE(test_flag_string);
  auto test_flag_bool_external_observer =
      THRIFT_FLAG_OBSERVE(test_flag_bool_external);
  auto test_flag_int_extenal_observer =
      THRIFT_FLAG_OBSERVE(test_flag_int_external);
  auto test_flag_string_external_observer =
      THRIFT_FLAG_OBSERVE(test_flag_string_external);
  EXPECT_EQ(true, **test_flag_bool_observer);
  EXPECT_EQ(42, **test_flag_int_observer);
  EXPECT_EQ("foo", **test_flag_string_observer);
  EXPECT_EQ(true, **test_flag_bool_external_observer);
  EXPECT_EQ(42, **test_flag_int_extenal_observer);
  EXPECT_EQ("foo", **test_flag_string_external_observer);

  testBackendPtr->getFlagObservableBool("test_flag_bool").setValue(false);
  testBackendPtr->getFlagObservableInt64("test_flag_int").setValue(41);
  testBackendPtr->getFlagObservableString("test_flag_string")
      .setValue(std::string{"bar"});
  testBackendPtr->getFlagObservableBool("test_flag_bool_external")
      .setValue(false);
  testBackendPtr->getFlagObservableInt64("test_flag_int_external").setValue(41);
  testBackendPtr->getFlagObservableString("test_flag_string_external")
      .setValue(std::string{"bar"});

  folly::observer_detail::ObserverManager::waitForAllUpdates();

  EXPECT_EQ(false, **test_flag_bool_observer);
  EXPECT_EQ(41, **test_flag_int_observer);
  EXPECT_EQ("bar", **test_flag_string_observer);
  EXPECT_EQ(false, **test_flag_bool_external_observer);
  EXPECT_EQ(41, **test_flag_int_extenal_observer);
  EXPECT_EQ("bar", **test_flag_string_external_observer);
}

TEST_F(Flags, NoBackend) {
  if (testBackendPtr) {
    GTEST_SKIP() << "Requires isolated test run";
  }

  useDummyBackend = true;

  EXPECT_EQ(true, THRIFT_FLAG(test_flag_bool));
  EXPECT_EQ(42, THRIFT_FLAG(test_flag_int));
  EXPECT_EQ("foo", THRIFT_FLAG(test_flag_string));
  EXPECT_EQ(true, THRIFT_FLAG(test_flag_bool_external));
  EXPECT_EQ(42, THRIFT_FLAG(test_flag_int_external));
  EXPECT_EQ("foo", THRIFT_FLAG(test_flag_string_external));
}

TEST_F(Flags, MockGet) {
  EXPECT_EQ(true, THRIFT_FLAG(test_flag_bool));
  EXPECT_EQ(42, THRIFT_FLAG(test_flag_int));
  EXPECT_EQ("foo", THRIFT_FLAG(test_flag_string));
  EXPECT_EQ(true, THRIFT_FLAG(test_flag_bool_external));
  EXPECT_EQ(42, THRIFT_FLAG(test_flag_int_external));
  EXPECT_EQ("foo", THRIFT_FLAG(test_flag_string_external));

  THRIFT_FLAG_SET_MOCK(test_flag_bool, false);
  THRIFT_FLAG_SET_MOCK(test_flag_int, 41);
  THRIFT_FLAG_SET_MOCK(test_flag_string, std::string{"bar"});
  THRIFT_FLAG_SET_MOCK(test_flag_bool_external, false);
  THRIFT_FLAG_SET_MOCK(test_flag_int_external, 41);
  THRIFT_FLAG_SET_MOCK(test_flag_string_external, std::string{"bar"});

  folly::observer_detail::ObserverManager::waitForAllUpdates();

  EXPECT_EQ(false, THRIFT_FLAG(test_flag_bool));
  EXPECT_EQ(41, THRIFT_FLAG(test_flag_int));
  EXPECT_EQ("bar", THRIFT_FLAG(test_flag_string));
  EXPECT_EQ(false, THRIFT_FLAG(test_flag_bool_external));
  EXPECT_EQ(41, THRIFT_FLAG(test_flag_int_external));
  EXPECT_EQ("bar", THRIFT_FLAG(test_flag_string_external));

  THRIFT_FLAG_SET_MOCK(test_flag_bool, true);
  THRIFT_FLAG_SET_MOCK(test_flag_int, 9);
  THRIFT_FLAG_SET_MOCK(test_flag_string, std::string{"baz"});
  THRIFT_FLAG_SET_MOCK(test_flag_bool_external, true);
  THRIFT_FLAG_SET_MOCK(test_flag_int_external, 61);
  THRIFT_FLAG_SET_MOCK(test_flag_string_external, std::string{"baz"});

  folly::observer_detail::ObserverManager::waitForAllUpdates();

  EXPECT_EQ(true, THRIFT_FLAG(test_flag_bool));
  EXPECT_EQ(9, THRIFT_FLAG(test_flag_int));
  EXPECT_EQ("baz", THRIFT_FLAG(test_flag_string));
  EXPECT_EQ(true, THRIFT_FLAG(test_flag_bool_external));
  EXPECT_EQ(61, THRIFT_FLAG(test_flag_int_external));
  EXPECT_EQ("baz", THRIFT_FLAG(test_flag_string_external));
}

TEST_F(Flags, MockObserve) {
  EXPECT_EQ(true, THRIFT_FLAG(test_flag_bool));
  EXPECT_EQ(42, THRIFT_FLAG(test_flag_int));
  EXPECT_EQ("foo", THRIFT_FLAG(test_flag_string));
  EXPECT_EQ(true, THRIFT_FLAG(test_flag_bool_external));
  EXPECT_EQ(42, THRIFT_FLAG(test_flag_int_external));
  EXPECT_EQ("foo", THRIFT_FLAG(test_flag_string_external));

  auto test_flag_bool_observer = THRIFT_FLAG_OBSERVE(test_flag_bool);
  auto test_flag_int_observer = THRIFT_FLAG_OBSERVE(test_flag_int);
  auto test_flag_string_observer = THRIFT_FLAG_OBSERVE(test_flag_string);
  auto test_flag_bool_external_observer =
      THRIFT_FLAG_OBSERVE(test_flag_bool_external);
  auto test_flag_int_extenal_observer =
      THRIFT_FLAG_OBSERVE(test_flag_int_external);
  auto test_flag_string_external_observer =
      THRIFT_FLAG_OBSERVE(test_flag_string_external);

  THRIFT_FLAG_SET_MOCK(test_flag_bool, false);
  THRIFT_FLAG_SET_MOCK(test_flag_int, 41);
  THRIFT_FLAG_SET_MOCK(test_flag_string, std::string{"bar"});
  THRIFT_FLAG_SET_MOCK(test_flag_bool_external, false);
  THRIFT_FLAG_SET_MOCK(test_flag_int_external, 41);
  THRIFT_FLAG_SET_MOCK(test_flag_string_external, std::string{"bar"});

  folly::observer_detail::ObserverManager::waitForAllUpdates();

  EXPECT_EQ(false, **test_flag_bool_observer);
  EXPECT_EQ(41, **test_flag_int_observer);
  EXPECT_EQ("bar", **test_flag_string_observer);
  EXPECT_EQ(false, **test_flag_bool_external_observer);
  EXPECT_EQ(41, **test_flag_int_extenal_observer);
  EXPECT_EQ("bar", **test_flag_string_external_observer);

  THRIFT_FLAG_SET_MOCK(test_flag_bool, true);
  THRIFT_FLAG_SET_MOCK(test_flag_int, 9);
  THRIFT_FLAG_SET_MOCK(test_flag_string, std::string{"baz"});
  THRIFT_FLAG_SET_MOCK(test_flag_bool_external, true);
  THRIFT_FLAG_SET_MOCK(test_flag_int_external, 61);
  THRIFT_FLAG_SET_MOCK(test_flag_string_external, std::string{"baz"});

  folly::observer_detail::ObserverManager::waitForAllUpdates();

  EXPECT_EQ(true, **test_flag_bool_observer);
  EXPECT_EQ(9, **test_flag_int_observer);
  EXPECT_EQ("baz", **test_flag_string_observer);
  EXPECT_EQ(true, **test_flag_bool_external_observer);
  EXPECT_EQ(61, **test_flag_int_extenal_observer);
  EXPECT_EQ("baz", **test_flag_string_external_observer);
}

TEST_F(Flags, MockValuePreferred) {
  EXPECT_EQ(true, THRIFT_FLAG(test_flag_bool));
  EXPECT_EQ(42, THRIFT_FLAG(test_flag_int));
  EXPECT_EQ("foo", THRIFT_FLAG(test_flag_string));
  EXPECT_EQ(true, THRIFT_FLAG(test_flag_bool_external));
  EXPECT_EQ(42, THRIFT_FLAG(test_flag_int_external));
  EXPECT_EQ("foo", THRIFT_FLAG(test_flag_string_external));

  THRIFT_FLAG_SET_MOCK(test_flag_bool, true);
  THRIFT_FLAG_SET_MOCK(test_flag_int, 73);
  THRIFT_FLAG_SET_MOCK(test_flag_string, std::string{"mock"});
  THRIFT_FLAG_SET_MOCK(test_flag_bool_external, true);
  THRIFT_FLAG_SET_MOCK(test_flag_int_external, 49);
  THRIFT_FLAG_SET_MOCK(test_flag_string_external, std::string{"mock"});

  folly::observer_detail::ObserverManager::waitForAllUpdates();

  testBackendPtr->getFlagObservableBool("test_flag_bool").setValue(false);
  testBackendPtr->getFlagObservableInt64("test_flag_int").setValue(41);
  testBackendPtr->getFlagObservableString("test_flag_string")
      .setValue(std::string{"bar"});
  testBackendPtr->getFlagObservableBool("test_flag_bool_external")
      .setValue(false);
  testBackendPtr->getFlagObservableInt64("test_flag_int_external").setValue(41);
  testBackendPtr->getFlagObservableString("test_flag_string_external")
      .setValue(std::string{"bar"});

  folly::observer_detail::ObserverManager::waitForAllUpdates();

  EXPECT_EQ(true, THRIFT_FLAG(test_flag_bool));
  EXPECT_EQ(73, THRIFT_FLAG(test_flag_int));
  EXPECT_EQ("mock", THRIFT_FLAG(test_flag_string));
  EXPECT_EQ(true, THRIFT_FLAG(test_flag_bool_external));
  EXPECT_EQ(49, THRIFT_FLAG(test_flag_int_external));
  EXPECT_EQ("mock", THRIFT_FLAG(test_flag_string_external));
}

void assertFlagValue(
    std::vector<apache::thrift::ThriftFlagInfo> allFlags,
    const std::string& name,
    const std::string& expectedValue) {
  bool flagFound = false;
  for (const auto& flag : allFlags) {
    if (flag.name == name) {
      EXPECT_EQ(expectedValue, flag.currentValue);
      flagFound = true;
    }
  }
  EXPECT_TRUE(flagFound);
}

TEST_F(Flags, getAllThriftFlags) {
  // Arrange
  THRIFT_FLAG_SET_MOCK(test_flag_bool, false);
  THRIFT_FLAG_SET_MOCK(test_flag_int, 88);
  THRIFT_FLAG_SET_MOCK(test_flag_string, std::string{"test string"});

  // Act
  auto allFlags = apache::thrift::getAllThriftFlags();

  // Assert
  assertFlagValue(allFlags, "test_flag_bool", "false");
  assertFlagValue(allFlags, "test_flag_int", "88");
  assertFlagValue(allFlags, "test_flag_string", "test string");
}
