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
  // All KeyOrders of length > kMaxLen that have the same prefix
  // are group together.
  constexpr static size_t kMaxLen = 16;
  KeyOrder insert(const StringData*) const;
  KeyOrder remove(const StringData*) const;
  KeyOrder pop() const;
  std::string toString() const;
  bool operator==(const KeyOrder& other) const;
  static KeyOrder ForArray(const ArrayData*);
  static KeyOrder MakeInvalid();
  using KeyOrderData = std::vector<LowStringPtr>;

private:
  friend struct KeyOrderHash;
  friend struct KeyOrderDataHash;
  static KeyOrder Make(const KeyOrderData&);
  // Default KeyOrder represents a KeyOrder that has non-static string keys.
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
