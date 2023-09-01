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

#pragma once

#include <ostream>
#include <string>
#include <tuple>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include <fmt/core.h>
#include <folly/CPortability.h>
#include <thrift/lib/cpp2/type/NativeType.h>
#include <thrift/lib/cpp2/type/Tag.h>
#include <thrift/lib/cpp2/type/ThriftType.h>

namespace apache::thrift::conformance::data {

// A value with an associated name.
template <typename T>
struct NamedValue {
  using value_type = T;

  explicit NamedValue(T value, std::string name)
      : value(std::move(value)), name(std::move(name)) {}

  T value;
  std::string name;

  friend bool operator==(const NamedValue& lhs, const NamedValue& rhs) {
    return lhs.value == rhs.value && lhs.name == rhs.name;
  }
};

// A list of named values for the given thrift type.
template <typename Tag>
using NamedValues = std::vector<NamedValue<type::native_type<Tag>>>;

// Adds all the values using the given inserter.
template <typename C, typename I>
void addValues(const C& values, I inserter) {
  for (const auto& entry : values) {
    *inserter++ = entry.value;
  }
}

template <typename Tag>
struct BaseValueGenerator {
  static_assert(type::is_concrete_v<Tag>, "not a concrete type");

  using thrift_type = Tag;
  using native_type = type::native_type<Tag>;
  using Values = NamedValues<Tag>;
};

// The interface for a value generator for a given type and implementation
// for base types.
template <typename Tag>
struct ValueGenerator : BaseValueGenerator<Tag> {
  using Values = typename BaseValueGenerator<Tag>::Values;

  // Intersting values for the given type.
  FOLLY_EXPORT static const Values& getInterestingValues();

  // Interesting unique values that can be used in a set or map key.
  FOLLY_EXPORT static const Values& getKeyValues();
};

// The generator for a list of values.
template <typename VTag>
struct ValueGenerator<type::list<VTag>> : BaseValueGenerator<type::list<VTag>> {
  using Base = BaseValueGenerator<type::list<VTag>>;
  using Values = typename Base::Values;
  using native_type = typename Base::native_type;

  FOLLY_EXPORT static const Values& getKeyValues() {
    static auto* kValues =
        new Values(getValues(ValueGenerator<VTag>::getKeyValues()));
    return *kValues;
  }

  FOLLY_EXPORT static const Values& getInterestingValues() {
    static auto* kValues =
        new Values(getValues(ValueGenerator<VTag>::getInterestingValues()));
    return *kValues;
  }

 private:
  static Values getValues(const typename ValueGenerator<VTag>::Values& values);
};

// The generator for a service of values.
template <typename Adapter, typename VTag>
struct ValueGenerator<type::adapted<Adapter, VTag>>
    : BaseValueGenerator<type::adapted<Adapter, VTag>> {
  using Base = BaseValueGenerator<type::adapted<Adapter, VTag>>;
  using Values = typename Base::Values;
  using native_type = typename Base::native_type;

  FOLLY_EXPORT static const Values& getKeyValues() {
    static auto* kValues =
        new Values(getValues(ValueGenerator<VTag>::getKeyValues()));
    return *kValues;
  }

  FOLLY_EXPORT static const Values& getInterestingValues() {
    static auto* iValues =
        new Values(getValues(ValueGenerator<VTag>::getInterestingValues()));
    return *iValues;
  }

 private:
  static Values getValues(const typename ValueGenerator<VTag>::Values& values);
};

// The generator for a set of values.
template <typename VTag>
struct ValueGenerator<type::set<VTag>> : BaseValueGenerator<type::set<VTag>> {
  using Base = BaseValueGenerator<type::set<VTag>>;
  using Values = typename Base::Values;
  using native_type = typename Base::native_type;

  FOLLY_EXPORT static const Values& getKeyValues() {
    static auto* kValues =
        new Values(getValues(ValueGenerator<VTag>::getKeyValues()));
    return *kValues;
  }
  static const Values& getInterestingValues() {
    // Sets can only contain 'key' values.
    return getKeyValues();
  }

 private:
  static Values getValues(const typename ValueGenerator<VTag>::Values& values);
};

// The generator for a map of values.
template <typename KTag, typename VTag>
struct ValueGenerator<type::map<KTag, VTag>>
    : BaseValueGenerator<type::map<KTag, VTag>> {
  using Base = BaseValueGenerator<type::map<KTag, VTag>>;
  using Values = typename Base::Values;
  using KeyValues = typename ValueGenerator<KTag>::Values;
  using MappedValues = typename ValueGenerator<VTag>::Values;
  using native_type = typename Base::native_type;

  FOLLY_EXPORT static const Values& getKeyValues() {
    // To be used as a 'key' all keys and values must also be usable
    // as a 'key'
    static auto* kValues = new Values(getValues(
        ValueGenerator<KTag>::getKeyValues(),
        ValueGenerator<VTag>::getKeyValues()));
    return *kValues;
  }

  FOLLY_EXPORT static const Values& getInterestingValues() {
    // Map keys must be 'key' values, but the values can be any values.
    static auto* kValues = new Values(getValues(
        ValueGenerator<KTag>::getKeyValues(),
        ValueGenerator<VTag>::getInterestingValues()));
    return *kValues;
  }

 private:
  static Values getValues(const KeyValues& keys, const MappedValues& values);
};

// Implementation

template <typename VTag>
auto ValueGenerator<type::list<VTag>>::getValues(
    const typename ValueGenerator<VTag>::Values& values) -> Values {
  Values result;
  result.emplace_back(native_type{}, "empty");

  { // All values.
    native_type all;
    addValues(values, std::back_inserter(all));
    result.emplace_back(std::move(all), "all");
  }
  { // All values x 2.
    native_type duplicate;
    addValues(values, std::back_inserter(duplicate));
    addValues(values, std::back_inserter(duplicate));
    result.emplace_back(std::move(duplicate), "duplicate");
  }

  if (values.size() > 1) {
    // Reverse of all interesting values.
    native_type reverse;
    addValues(values, std::back_inserter(reverse));
    std::reverse(reverse.begin(), reverse.end());
    result.emplace_back(std::move(reverse), "reverse");
  }

  if (values.size() > 2) { // Otherwise would duplicate interesting or reverse.
    native_type frontSwap;
    addValues(values, std::back_inserter(frontSwap));
    std::swap(frontSwap[0], frontSwap[2]);
    result.emplace_back(std::move(frontSwap), "frontSwap");
  }
  return result;
}

template <typename Adapter, typename VTag>
auto ValueGenerator<type::adapted<Adapter, VTag>>::getValues(
    const typename ValueGenerator<VTag>::Values& values) -> Values {
  Values result;
  for (const auto& value : values) {
    result.emplace_back(Adapter::fromThrift(value.value), value.name);
  }
  return result;
}

template <typename VTag>
auto ValueGenerator<type::set<VTag>>::getValues(
    const typename ValueGenerator<VTag>::Values& values) -> Values {
  Values result;
  result.emplace_back(native_type(), "empty");

  native_type all;
  addValues(values, std::inserter(all, all.begin()));
  result.emplace_back(std::move(all), "all");

  for (const auto& value : values) {
    native_type single;
    single.emplace(value.value);
    result.emplace_back(std::move(single), fmt::format("set({})", value.name));
  }
  return result;
}

template <typename KTag, typename VTag>
auto ValueGenerator<type::map<KTag, VTag>>::getValues(
    const KeyValues& keys, const MappedValues& values) -> Values {
  Values result;
  result.emplace_back(native_type(), "empty");
  for (const auto& value : values) {
    native_type allKeys;
    for (const auto& key : keys) {
      allKeys.emplace(key.value, value.value);

      native_type single;
      single.emplace(key.value, value.value);
      result.emplace_back(std::move(single), key.name + " -> " + value.name);
    }
    result.emplace_back(std::move(allKeys), "all keys -> " + value.name);
  }
  return result;
}

} // namespace apache::thrift::conformance::data
