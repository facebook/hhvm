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

// Returns the conformance test tag for definintion spec page, for the given
// Thrift type.
template <typename Tag>
constexpr const char* getSpecDefinitionTag() {
  if constexpr (type::is_a_v<Tag, type::string_t>) {
    return "spec/definition/data#utf-8-string";
  } else if constexpr (type::is_a_v<Tag, type::enum_c>) {
    return "spec/definition/data#enum";
  } else if constexpr (type::is_a_v<Tag, type::floating_point_c>) {
    return "spec/definition/data#floating-point";
  } else if constexpr (type::is_a_v<Tag, type::primitive_c>) {
    return "spec/definition/data#primitive-types";
  } else if constexpr (type::is_a_v<Tag, type::container_c>) {
    return "spec/definition/data#container-types";
  } else if constexpr (type::is_a_v<Tag, type::structured_c>) {
    return "spec/definition/data#structured-types";
  } else {
    return "spec/definition/data";
  }
}

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
using NamedValues = std::vector<NamedValue<type::standard_type<Tag>>>;

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
  using standard_type = type::standard_type<Tag>;
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
template <typename ValTag>
struct ValueGenerator<type::list<ValTag>>
    : BaseValueGenerator<type::list<ValTag>> {
  using Base = BaseValueGenerator<type::list<ValTag>>;
  using Values = typename Base::Values;
  using standard_type = typename Base::standard_type;

  FOLLY_EXPORT static const Values& getKeyValues() {
    static auto* kValues =
        new Values(getValues(ValueGenerator<ValTag>::getKeyValues()));
    return *kValues;
  }

  FOLLY_EXPORT static const Values& getInterestingValues() {
    static auto* kValues =
        new Values(getValues(ValueGenerator<ValTag>::getInterestingValues()));
    return *kValues;
  }

 private:
  static Values getValues(
      const typename ValueGenerator<ValTag>::Values& values);
};

// The generator for a set of values.
template <typename ValTag>
struct ValueGenerator<type::set<ValTag>>
    : BaseValueGenerator<type::set<ValTag>> {
  using Base = BaseValueGenerator<type::set<ValTag>>;
  using Values = typename Base::Values;
  using standard_type = typename Base::standard_type;

  FOLLY_EXPORT static const Values& getKeyValues() {
    static auto* kValues =
        new Values(getValues(ValueGenerator<ValTag>::getKeyValues()));
    return *kValues;
  }
  static const Values& getInterestingValues() {
    // Sets can only contain 'key' values.
    return getKeyValues();
  }

 private:
  static Values getValues(
      const typename ValueGenerator<ValTag>::Values& values);
};

// The generator for a map of values.
template <typename KeyTag, typename ValTag>
struct ValueGenerator<type::map<KeyTag, ValTag>>
    : BaseValueGenerator<type::map<KeyTag, ValTag>> {
  using Base = BaseValueGenerator<type::map<KeyTag, ValTag>>;
  using Values = typename Base::Values;
  using KeyValues = typename ValueGenerator<KeyTag>::Values;
  using MappedValues = typename ValueGenerator<ValTag>::Values;
  using standard_type = typename Base::standard_type;

  FOLLY_EXPORT static const Values& getKeyValues() {
    // To be used as a 'key' all keys and values must also be usable
    // as a 'key'
    static auto* kValues = new Values(getValues(
        ValueGenerator<KeyTag>::getKeyValues(),
        ValueGenerator<ValTag>::getKeyValues()));
    return *kValues;
  }

  FOLLY_EXPORT static const Values& getInterestingValues() {
    // Map keys must be 'key' values, but the values can be any values.
    static auto* kValues = new Values(getValues(
        ValueGenerator<KeyTag>::getKeyValues(),
        ValueGenerator<ValTag>::getInterestingValues()));
    return *kValues;
  }

 private:
  static Values getValues(const KeyValues& keys, const MappedValues& values);
};

// Implementation

template <typename ValTag>
auto ValueGenerator<type::list<ValTag>>::getValues(
    const typename ValueGenerator<ValTag>::Values& values) -> Values {
  Values result;
  result.emplace_back(standard_type{}, "empty");

  { // All values.
    standard_type all;
    addValues(values, std::back_inserter(all));
    result.emplace_back(std::move(all), "all");
  }
  { // All values x 2.
    standard_type duplicate;
    addValues(values, std::back_inserter(duplicate));
    addValues(values, std::back_inserter(duplicate));
    result.emplace_back(std::move(duplicate), "duplicate");
  }

  if (values.size() > 1) {
    // Reverse of all interesting values.
    standard_type reverse;
    addValues(values, std::back_inserter(reverse));
    std::reverse(reverse.begin(), reverse.end());
    result.emplace_back(std::move(reverse), "reverse");
  }

  if (values.size() > 2) { // Otherwise would duplicate interesting or reverse.
    standard_type frontSwap;
    addValues(values, std::back_inserter(frontSwap));
    std::swap(frontSwap[0], frontSwap[2]);
    result.emplace_back(std::move(frontSwap), "front swap");
  }
  return result;
}

template <typename ValTag>
auto ValueGenerator<type::set<ValTag>>::getValues(
    const typename ValueGenerator<ValTag>::Values& values) -> Values {
  Values result;
  result.emplace_back(standard_type(), "empty");

  standard_type all;
  addValues(values, std::inserter(all, all.begin()));
  result.emplace_back(std::move(all), "all");

  for (const auto& value : values) {
    standard_type single;
    single.emplace(value.value);
    result.emplace_back(std::move(single), fmt::format("set({})", value.name));
  }
  return result;
}

template <typename KeyTag, typename ValTag>
auto ValueGenerator<type::map<KeyTag, ValTag>>::getValues(
    const KeyValues& keys, const MappedValues& values) -> Values {
  Values result;
  result.emplace_back(standard_type(), "empty");
  for (const auto& value : values) {
    standard_type allKeys;
    for (const auto& key : keys) {
      allKeys.emplace(key.value, value.value);

      standard_type single;
      single.emplace(key.value, value.value);
      result.emplace_back(std::move(single), key.name + " -> " + value.name);
    }
    result.emplace_back(std::move(allKeys), "all keys -> " + value.name);
  }
  return result;
}

} // namespace apache::thrift::conformance::data
