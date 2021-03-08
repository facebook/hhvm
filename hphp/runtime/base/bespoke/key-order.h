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

#include <vector>

namespace HPHP {

struct ArrayData;
namespace bespoke {

// KeyOrder represents insertion order of static string keys.
struct KeyOrder {
  using KeyOrderData = std::vector<LowStringPtr>;
  using const_iterator = KeyOrderData::const_iterator;

  // All KeyOrders of length > kMaxLen that have the same prefix
  // are grouped together.
  constexpr static size_t kMaxLen = 16;

  KeyOrder insert(const StringData*) const;
  KeyOrder remove(const StringData*) const;
  KeyOrder pop() const;
  const_iterator begin() const;
  const_iterator end() const;

  // A "valid" KeyOrder is one that can be used to make a struct layout, and
  // that can be merged with other valid layouts. An "empty" layout is valid,
  // but we typically won't want to make it a struct of its own.
  bool empty() const;
  bool valid() const;

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

}}
