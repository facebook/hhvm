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

#include <thrift/lib/py3/enums.h>

namespace thrift {
namespace py3 {

EnumData::EnumData(
    FindNameFunc fn,
    FindValueFunc fv,
    folly::Range<const folly::StringPiece*> names,
    const std::vector<NamePair>& nm)
    : findName_{fn},
      findValue_{fv},
      size_{names.size()},
      uncached_{names.size()},
      names_{names} {
  for (auto pair : nm) {
    pyToOrigNames_.emplace(pair.first, pair.second);
    origToPyNames_.emplace(pair.second, pair.first);
  }
}

std::pair<PyObject*, std::optional<int>> EnumData::tryGetByName(
    std::string_view name) const {
  // return if already in cache
  if (auto inst = findInCache(name)) {
    return {inst, {}};
  }
  // Not valid if 1. cache is complete or 2. the name has been renamed
  if (uncached_.load(std::memory_order_acquire) == 0 ||
      origToPyNames_.find(name) != origToPyNames_.end()) {
    return {nullptr, {}};
  }
  // then find value with its original name
  return {nullptr, findValue_(folly::get_default(pyToOrigNames_, name, name))};
}

std::pair<PyObject*, std::string_view> EnumData::tryGetByValue(
    int value) const {
  auto name = findPyName(value);
  if (name.empty()) {
    // name not found, invalid value
    return {nullptr, {}};
  }
  auto inst = findInCache(name);
  // if cache is complete so it has to be found
  DCHECK(uncached_.load(std::memory_order_acquire) > 0 || inst);
  return {inst, name};
}

PyObject* EnumData::tryAddToCache(int value, PyObject* obj) {
  auto name = findPyName(value);
  DCHECK(!name.empty());
  {
    std::unique_lock wm{cacheMutex_};
    auto inserted = namesToInstances_.emplace(name, obj);
    if (!inserted.second) {
      return inserted.first->second;
    }
  }
  uncached_.fetch_sub(1, std::memory_order_release);
  Py_XINCREF(obj);
  return obj;
}

PyObject* EnumData::findInCache(const std::string_view name) const {
  std::shared_lock lock{cacheMutex_, std::defer_lock};
  if (uncached_.load(std::memory_order_acquire) != 0) {
    lock.lock(); // only hold the lock when cache not fully populated
  }
  return folly::get_default(namesToInstances_, name, nullptr);
}

std::pair<PyObject*, std::string_view> EnumFlagsData::tryGetByValue(
    int value) const {
  // check if a legit flag value
  if (!isValidFlagValue(value)) {
    return {nullptr, {}};
  }
  // find it in the main name => instances cache
  auto name = findPyName(value);
  if (name.empty()) {
    // name not valid, so it's a derived value
    auto inst = findInFlagValuesCache(value);
    // if flagValues cache pully populated, inst should be valid
    DCHECK(flagValuesUncached_.load(std::memory_order_acquire) >= 0 || inst);
    return {inst, ""};
  }
  // otherwise it's not a derived value
  auto inst = findInCache(name);
  // if main cache fully populated and it's a valid value, inst should be
  // valid
  DCHECK(uncached_.load(std::memory_order_acquire) >= 0 || inst);
  return {inst, name};
}

PyObject* EnumFlagsData::tryAddToFlagValuesCache(
    uint32_t value, PyObject* obj) {
  {
    std::unique_lock wm{flagValueCacheMutex_};
    auto inserted = flagValuesToInstances_.emplace(value, obj);
    if (!inserted.second) {
      return inserted.first->second;
    }
  }
  flagValuesUncached_.fetch_sub(1, std::memory_order_release);
  Py_XINCREF(obj);
  return obj;
}

std::string EnumFlagsData::getNameForDerivedValue(uint32_t value) const {
  if (value == 0) {
    return "0";
  }
  std::vector<std::string_view> names;
  for (uint32_t mask = 1; mask <= value; mask <<= 1) {
    DCHECK(mask > 0);
    if ((value & mask) != 0) {
      names.push_back(findName_(mask));
    }
  }
  return folly::join("|", names.crbegin(), names.crend());
}

PyObject* EnumFlagsData::findInFlagValuesCache(uint32_t value) const {
  std::shared_lock lock{flagValueCacheMutex_, std::defer_lock};
  if (flagValuesUncached_.load(std::memory_order_acquire) != 0) {
    lock.lock();
  }
  return folly::get_default(flagValuesToInstances_, value, nullptr);
}

} // namespace py3
} // namespace thrift
