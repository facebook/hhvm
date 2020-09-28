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

#ifndef HPHP_ENTRY_TYPES_H
#define HPHP_ENTRY_TYPES_H

#include "hphp/runtime/base/array-data.h"
#include "hphp/runtime/base/datatype.h"
#include "hphp/runtime/base/typed-value.h"

namespace HPHP { namespace bespoke {

enum class KeyTypes : uint8_t {
  Empty,
  Ints,
  StaticStrings,
  Strings,
  Any
};

enum class ValueTypes : uint8_t {
  Empty,
  // Monotype indicates that the values are all of a single type. This single
  // type may be KindOfNull. An array containing only nulls will be Monotype,
  // not MonotypeNullable.
  Monotype,
  // MonotypeNullable indicates that all values are of a single type, or are
  // KindOfNull. We must have seen at least 1 non-null value for
  // MonotypeNullable to apply.
  MonotypeNullable,
  Any
};

struct EntryTypes {
  static EntryTypes ForArray(ArrayData* ad);

  EntryTypes(KeyTypes keyTypes, ValueTypes valueTypes,
             DataType valueDatatype)
    : keyTypes(keyTypes)
    , valueTypes(valueTypes)
    , valueDatatype(valueDatatype)
  { assertx(checkInvariants()); }

  explicit EntryTypes(uint16_t val)
   : keyTypes(static_cast<KeyTypes>((val >> 8) & 0xF))
   , valueTypes(static_cast<ValueTypes>((val >> 12) & 0xF))
   , valueDatatype(static_cast<DataType>(static_cast<int8_t>(val & 0xFF)))
  { assertx(checkInvariants()); }

  EntryTypes with(TypedValue k, TypedValue v) const;
  EntryTypes pessimizeValueTypes() const;
  std::string toString() const;

  uint16_t asInt16() const {
    return ((static_cast<uint8_t>(keyTypes) << 8) |
            (static_cast<uint8_t>(valueTypes) << 12) |
            static_cast<uint8_t>(valueDatatype));
  }

  bool operator==(const EntryTypes& other) const {
    return keyTypes == other.keyTypes &&
           valueTypes == other.valueTypes &&
           valueDatatype == other.valueDatatype;
  }

  bool operator!=(const EntryTypes& other) const {
    return !(*this == other);
  }

  bool checkInvariants() const;

  KeyTypes keyTypes;
  ValueTypes valueTypes;
  DataType valueDatatype;
};

}}

#endif
