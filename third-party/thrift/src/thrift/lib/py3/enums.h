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

#include <functional>
#include <optional>
#include <shared_mutex>
#include <string_view>
#include <unordered_map>
#include <Python.h>
#include <glog/logging.h>
#include <folly/Indestructible.h>
#include <folly/MapUtil.h>
#include <folly/Range.h>
#include <folly/SharedMutex.h>
#include <folly/String.h>
#include <thrift/lib/cpp/Thrift.h>

namespace thrift {
namespace py3 {

template <typename T>
struct PyEnumTraits {
  using NamesMap = std::vector<std::pair<std::string_view, std::string_view>>;
  static const NamesMap& namesmap();
};

class EnumData {
 public:
  using NamePair = std::pair<std::string_view, std::string_view>;
  using FindNameFunc = std::string_view(int);
  using FindValueFunc = std::optional<int>(std::string_view);
  EnumData(
      FindNameFunc* const fn,
      FindValueFunc* const fv,
      folly::Range<const std::string_view*> names,
      const std::vector<NamePair>& nm);

  virtual ~EnumData() {
    for (auto i : namesToInstances_) {
      Py_XDECREF(i.second);
    }
  }

  virtual std::pair<PyObject*, std::optional<int>> tryGetByName(
      std::string_view name) const;

  virtual std::pair<PyObject*, std::string_view> tryGetByValue(int value) const;

  PyObject* tryAddToCache(int value, PyObject* obj);

  std::size_t size() const { return size_; }

  std::string_view getPyName(std::string_view name) const {
    return folly::get_default(origToPyNames_, name, name);
  }

  folly::Range<const std::string_view*> getNames() const { return names_; }

 protected:
  using NameMap = std::unordered_map<std::string_view, std::string_view>;
  PyObject* findInCache(std::string_view name) const;

  std::string_view findPyName(int value) const {
    auto name = findName_(value);
    return name.empty() ? name : getPyName(name);
  }

  FindNameFunc* const findName_;
  FindValueFunc* const findValue_;
  const std::size_t size_;
  std::atomic<std::size_t> uncached_;
  const folly::Range<const std::string_view*> names_;
  NameMap pyToOrigNames_;
  NameMap origToPyNames_;
  std::unordered_map<std::string_view, PyObject*> namesToInstances_;
  mutable folly::SharedMutex cacheMutex_;
};

class EnumFlagsData : public EnumData {
 public:
  EnumFlagsData(
      FindNameFunc fn,
      FindValueFunc fv,
      folly::Range<const std::string_view*> names,
      const std::vector<NamePair>& nm,
      uint32_t maxPossibleFlagValue)
      : EnumData{fn, fv, names, nm},
        maxPossibleFlagValue_{maxPossibleFlagValue},
        flagValuesUncached_(1 << names.size()) {}
  ~EnumFlagsData() override {
    for (auto i : flagValuesToInstances_) {
      Py_XDECREF(i.second);
    }
  }
  std::pair<PyObject*, std::string_view> tryGetByValue(
      int value) const override;

  PyObject* tryAddToFlagValuesCache(uint32_t value, PyObject* obj);

  std::string getNameForDerivedValue(uint32_t value) const;

  uint32_t getInvertValue(uint32_t value) const {
    return maxPossibleFlagValue_ ^ value;
  }

  uint32_t convertNegativeValue(uint32_t value) const {
    return value & maxPossibleFlagValue_;
  }

 private:
  bool isValidFlagValue(uint32_t value) const {
    return (maxPossibleFlagValue_ | value) == maxPossibleFlagValue_;
  }

  PyObject* findInFlagValuesCache(uint32_t value) const;

  const uint32_t maxPossibleFlagValue_;
  mutable folly::SharedMutex flagValueCacheMutex_;
  std::unordered_map<uint32_t, PyObject*> flagValuesToInstances_;
  std::atomic<uint32_t> flagValuesUncached_;
};

namespace {
template <typename T>
std::string_view enumFindNameWrapper(int value) {
  const char* name =
      apache::thrift::TEnumTraits<T>::findName(static_cast<T>(value));
  return name == nullptr ? std::string_view{} : name;
}

template <typename T>
std::optional<int> enumFindValueWrapper(const std::string_view name) {
  T v{};
  const auto c = apache::thrift::TEnumTraits<T>::findValue(name.data(), &v);
  return c ? std::optional{static_cast<int>(v)} : std::nullopt;
}
} // namespace

template <typename T>
EnumData* createEnumData() {
  using Traits = apache::thrift::TEnumTraits<T>;
  return new EnumData(
      enumFindNameWrapper<T>,
      enumFindValueWrapper<T>,
      Traits::names,
      PyEnumTraits<T>::namesmap());
}

template <typename T>
EnumFlagsData* createEnumFlagsData() {
  using Traits = apache::thrift::TEnumTraits<T>;
  uint32_t s = 0;
  for (auto value : apache::thrift::TEnumTraits<T>::values) {
    s |= static_cast<uint32_t>(value);
  }
  return new EnumFlagsData(
      enumFindNameWrapper<T>,
      enumFindValueWrapper<T>,
      Traits::names,
      PyEnumTraits<T>::namesmap(),
      s);
}

template <typename T>
EnumData* createEnumDataForUnionType() {
  using type = typename T::Type;
  using Traits = apache::thrift::TEnumTraits<type>;
  return new EnumData(
      enumFindNameWrapper<type>,
      enumFindValueWrapper<type>,
      Traits::names,
      PyEnumTraits<type>::namesmap());
}

} // namespace py3
} // namespace thrift
