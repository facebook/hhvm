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

#include <thrift/lib/cpp2/server/ServerAttribute.h>

#include <folly/observer/Observer.h>
#include <folly/portability/GTest.h>

using namespace apache::thrift;

TEST(ServerAttributeDynamic, BaselineFirst) {
  ServerAttributeDynamic<int> a{0};
  EXPECT_EQ(a.get(), 0);
  folly::observer::SimpleObservable<std::optional<int>> baselineObservable{1};
  folly::observer::SimpleObservable<std::optional<int>> overrideObservable{2};
  a.set(baselineObservable.getObserver(), AttributeSource::BASELINE);
  folly::observer_detail::ObserverManager::waitForAllUpdates();
  EXPECT_EQ(a.get(), 1);
  a.set(overrideObservable.getObserver(), AttributeSource::OVERRIDE);
  EXPECT_EQ(a.get(), 2);
  overrideObservable.setValue(std::nullopt);
  folly::observer_detail::ObserverManager::waitForAllUpdates();
  EXPECT_EQ(a.get(), 1);

  baselineObservable.setValue(std::nullopt);
  folly::observer_detail::ObserverManager::waitForAllUpdates();
  EXPECT_EQ(a.get(), 0);
}

TEST(ServerAttributeDynamic, OverrideFirst) {
  ServerAttributeDynamic<int> a{0};
  EXPECT_EQ(a.get(), 0);
  folly::observer::SimpleObservable<std::optional<int>> baselineObservable{1};
  folly::observer::SimpleObservable<std::optional<int>> overrideObservable{2};
  a.set(overrideObservable.getObserver(), AttributeSource::OVERRIDE);
  folly::observer_detail::ObserverManager::waitForAllUpdates();
  EXPECT_EQ(a.get(), 2);
  a.set(baselineObservable.getObserver(), AttributeSource::BASELINE);
  folly::observer_detail::ObserverManager::waitForAllUpdates();
  // still return overrided value
  EXPECT_EQ(a.get(), 2);
  baselineObservable.setValue(std::nullopt);
  folly::observer_detail::ObserverManager::waitForAllUpdates();
  // still return overrided value
  EXPECT_EQ(a.get(), 2);
  overrideObservable.setValue(std::nullopt);
  folly::observer_detail::ObserverManager::waitForAllUpdates();
  EXPECT_EQ(a.get(), 0);
}

TEST(ServerAttributeDynamic, StringBaselineFirst) {
  ServerAttributeDynamic<std::string> a{"a"};
  EXPECT_EQ(a.get(), "a");
  folly::observer::SimpleObservable<std::optional<std::string>>
      baselineObservable{"b"};
  folly::observer::SimpleObservable<std::optional<std::string>>
      overrideObservable{"c"};
  a.set(baselineObservable.getObserver(), AttributeSource::BASELINE);
  folly::observer_detail::ObserverManager::waitForAllUpdates();
  EXPECT_EQ(a.get(), "b");
  a.set(overrideObservable.getObserver(), AttributeSource::OVERRIDE);
  folly::observer_detail::ObserverManager::waitForAllUpdates();
  EXPECT_EQ(a.get(), "c");

  overrideObservable.setValue(std::nullopt);
  folly::observer_detail::ObserverManager::waitForAllUpdates();
  EXPECT_EQ(a.get(), "b");
  baselineObservable.setValue(std::nullopt);
  folly::observer_detail::ObserverManager::waitForAllUpdates();
  EXPECT_EQ(a.get(), "a");
}

TEST(ServerAttributeDynamic, StringOverrideFirst) {
  ServerAttributeDynamic<std::string> a{"a"};
  EXPECT_EQ(a.get(), "a");
  folly::observer::SimpleObservable<std::optional<std::string>>
      baselineObservable{"b"};
  folly::observer::SimpleObservable<std::optional<std::string>>
      overrideObservable{"c"};

  a.set(overrideObservable.getObserver(), AttributeSource::OVERRIDE);
  folly::observer_detail::ObserverManager::waitForAllUpdates();
  EXPECT_EQ(a.get(), "c");
  a.set(baselineObservable.getObserver(), AttributeSource::BASELINE);
  folly::observer_detail::ObserverManager::waitForAllUpdates();
  // still return overrided value
  EXPECT_EQ(a.get(), "c");

  baselineObservable.setValue(std::nullopt);
  folly::observer_detail::ObserverManager::waitForAllUpdates();
  // still return overrided value
  EXPECT_EQ(a.get(), "c");
  overrideObservable.setValue(std::nullopt);
  folly::observer_detail::ObserverManager::waitForAllUpdates();
  EXPECT_EQ(a.get(), "a");
}

TEST(ServerAttributeDynamic, setDefault) {
  ServerAttributeDynamic<int> a{0};
  EXPECT_EQ(a.get(), 0);
  folly::observer::SimpleObservable<std::optional<int>> baselineObservable{1};
  folly::observer::SimpleObservable<std::optional<int>> overrideObservable{2};
  folly::observer::SimpleObservable<int> defaultVal{3};

  a.set(baselineObservable.getObserver(), AttributeSource::BASELINE);
  folly::observer_detail::ObserverManager::waitForAllUpdates();
  EXPECT_EQ(a.get(), 1);
  a.set(overrideObservable.getObserver(), AttributeSource::OVERRIDE);
  folly::observer_detail::ObserverManager::waitForAllUpdates();
  EXPECT_EQ(a.get(), 2);

  // update the default to 3 instead of 0
  a.setDefault(defaultVal.getObserver());

  // still gets override value
  EXPECT_EQ(a.get(), 2);

  overrideObservable.setValue(std::nullopt);
  folly::observer_detail::ObserverManager::waitForAllUpdates();
  EXPECT_EQ(a.get(), 1);
  baselineObservable.setValue(std::nullopt);
  folly::observer_detail::ObserverManager::waitForAllUpdates();
  // should return the new set default value
  EXPECT_EQ(a.get(), 3);
}

TEST(ServerAttributeDynamic, Observable) {
  folly::observer::SimpleObservable<std::string> defaultObservable{"default"};
  folly::observer::SimpleObservable<std::optional<std::string>>
      baselineObservable{"baseline"};
  folly::observer::SimpleObservable<std::optional<std::string>>
      overrideObservable{"override"};

  apache::thrift::detail::ServerAttributeObservable<std::string> attr{
      defaultObservable.getObserver()};
  auto observer = attr.getObserver();

  attr.set(baselineObservable.getObserver(), AttributeSource::BASELINE);
  folly::observer_detail::ObserverManager::waitForAllUpdates();
  EXPECT_EQ(**observer, "baseline");

  attr.set(overrideObservable.getObserver(), AttributeSource::OVERRIDE);
  EXPECT_EQ(**observer, "override");

  overrideObservable.setValue("override 2");
  folly::observer_detail::ObserverManager::waitForAllUpdates();
  EXPECT_EQ(**observer, "override 2");

  baselineObservable.setValue("baseline 2");
  overrideObservable.setValue(std::nullopt);
  folly::observer_detail::ObserverManager::waitForAllUpdates();

  EXPECT_EQ(**observer, "baseline 2");
  baselineObservable.setValue("baseline 3");
  folly::observer_detail::ObserverManager::waitForAllUpdates();
  EXPECT_EQ(**observer, "baseline 3");

  defaultObservable.setValue("default 2");
  baselineObservable.setValue(std::nullopt);
  folly::observer_detail::ObserverManager::waitForAllUpdates();
  EXPECT_EQ(**observer, "default 2");
}

TEST(ServerAttributeDynamic, Atomic) {
  ServerAttributeAtomic<int> attr{42};
  folly::observer::SimpleObservable<std::optional<int>> baselineObservable{24};
  folly::observer::SimpleObservable<std::optional<int>> overrideObservable{12};
  auto observer = attr.getAtomicObserver();
  EXPECT_EQ(*observer, 42);

  attr.set(baselineObservable.getObserver(), AttributeSource::BASELINE);
  folly::observer_detail::ObserverManager::waitForAllUpdates();
  EXPECT_EQ(*observer, 24);

  attr.set(overrideObservable.getObserver(), AttributeSource::OVERRIDE);
  folly::observer_detail::ObserverManager::waitForAllUpdates();
  EXPECT_EQ(*observer, 12);

  overrideObservable.setValue(std::nullopt);
  folly::observer_detail::ObserverManager::waitForAllUpdates();
  EXPECT_EQ(*observer, 24);
}

TEST(ServerAttributeDynamic, ThreadLocal) {
  ServerAttributeThreadLocal<int> attr{42};
  folly::observer::SimpleObservable<std::optional<int>> baselineObservable{24};
  folly::observer::SimpleObservable<std::optional<int>> overrideObservable{12};
  auto observer = attr.getTLObserver();
  EXPECT_EQ(**observer, 42);

  attr.set(baselineObservable.getObserver(), AttributeSource::BASELINE);
  folly::observer_detail::ObserverManager::waitForAllUpdates();
  EXPECT_EQ(**observer, 24);

  attr.set(overrideObservable.getObserver(), AttributeSource::OVERRIDE);
  EXPECT_EQ(**observer, 12);

  overrideObservable.setValue(std::nullopt);
  folly::observer_detail::ObserverManager::waitForAllUpdates();
  EXPECT_EQ(**observer, 24);
}

TEST(ServerAttributeStatic, Basic) {
  ServerAttributeStatic<std::string> attr{"default"};
  EXPECT_EQ(attr.get(), "default");

  attr.set("baseline", AttributeSource::BASELINE);
  EXPECT_EQ(attr.get(), "baseline");
  attr.set("override", AttributeSource::OVERRIDE);
  EXPECT_EQ(attr.get(), "override");

  attr.reset(AttributeSource::OVERRIDE);
  EXPECT_EQ(attr.get(), "baseline");
  attr.reset(AttributeSource::BASELINE);
  EXPECT_EQ(attr.get(), "default");

  attr.set("override", AttributeSource::OVERRIDE);
  EXPECT_EQ(attr.get(), "override");
  attr.set("baseline", AttributeSource::BASELINE);
  EXPECT_EQ(attr.get(), "override");
  attr.reset(AttributeSource::BASELINE);
  EXPECT_EQ(attr.get(), "override");

  attr.reset(AttributeSource::OVERRIDE);
  EXPECT_EQ(attr.get(), "default");
}
