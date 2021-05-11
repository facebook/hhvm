/*
  +----------------------------------------------------------------------+
  | HipHop for PHP                                                       |
  +----------------------------------------------------------------------+
  | Copyright (c) 2010-present Facebook, Inc. (http://www.facebook.com)  |
  +----------------------------------------------------------------------+
  | This source file is subject to version 3.01 of the PHP license,      |
  | that is bundled with this package in the file LICENSE, and is        |
  | available through the world-wide-web at the following url:           |
  | http://www.php.net/license/3_01.txt                                  |
  | If you did not receive a copy of the PHP license and are unable to   |
  | obtain it through the world-wide-web, please send a note to          |
  | license@php.net so we can mail you a copy immediately.               |
  +----------------------------------------------------------------------+
*/

#pragma once

#include "hphp/runtime/base/types.h"

#include <folly/container/F14Map.h>

#include <vector>

namespace HPHP {

struct ArrayData;

namespace bespoke {

// KeyOrder represents insertion order of static string keys.
struct KeyOrder {
  using KeyOrderData = std::vector<LowStringPtr>;
  using const_iterator = KeyOrderData::const_iterator;

  KeyOrder insert(const StringData*) const;
  KeyOrder remove(const StringData*) const;
  KeyOrder pop() const;
  const_iterator begin() const;
  const_iterator end() const;
  size_t size() const;

  // A "valid" KeyOrder is one that can be used to make a struct layout, and
  // that can be merged with other valid layouts. An "empty" layout is valid,
  // but we typically won't want to make it a struct of its own.
  bool empty() const;
  bool valid() const;
  bool contains(const StringData*) const;

  bool operator==(const KeyOrder& other) const;
  std::string toString() const;

  static KeyOrder ForArray(const ArrayData*);
  static KeyOrder Make(const KeyOrderData&);
  static KeyOrder MakeInvalid();

private:
  friend struct KeyOrderHash;
  friend struct KeyOrderDataHash;

  // A default KeyOrder is "invalid", for arrayswith non-static-string keys.
  KeyOrder() = default;
  explicit KeyOrder(const KeyOrderData*);

  bool isTooLong() const;
  static KeyOrderData trimKeyOrder(const KeyOrderData&);

  const KeyOrderData* m_keys = nullptr;
};

struct KeyOrderHash {
  size_t operator()(const KeyOrder& ko) const {
    return std::hash<const KeyOrder::KeyOrderData*>{}(ko.m_keys);
  }
};

// Wrapper around std::atomic offering copy construction/assignment. Meant to
// be used as a value type for containers when we've properly synchronized all
// potential internal value copies on that container (e.g. resizes).
//
// TODO(kshaunak): Move this wrapper to a utilities file.
template <typename T>
struct CopyAtomic {
  CopyAtomic(): value() {}
  /* implicit */ CopyAtomic(T value): value(value) {}

  CopyAtomic(const CopyAtomic<T>& other)
    : value(other.value.load(std::memory_order_acquire))
  {}

  CopyAtomic& operator=(const CopyAtomic<T>& other) {
    value = other.value.load(std::memory_order_acquire);
    return *this;
  }

  operator T() const {
    return value;
  }

  std::atomic<T> value;
};

// TODO(kshaunak): We can switch this over to a folly::F14Map.
using KeyOrderMap =
  std::unordered_map<KeyOrder, CopyAtomic<size_t>, KeyOrderHash>;

// Return a KeyOrder in "canonical form" (for now: sorted) that includes all
// keys in the given map. If the map has too many keys, or keys of the wrong
// type, this function will return an invalid KeyOrder.
KeyOrder collectKeyOrder(const KeyOrderMap& map);

void mergeKeyOrderMap(KeyOrderMap& dst, const KeyOrderMap& src);

}}
