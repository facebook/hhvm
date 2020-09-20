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

#include "hphp/runtime/base/bespoke/entry-types.h"

#include "hphp/runtime/base/array-data-defs.h"
#include "hphp/runtime/base/array-iterator.h"

namespace HPHP { namespace bespoke {

TRACE_SET_MOD(bespoke);

//////////////////////////////////////////////////////////////////////////////

namespace {

//////////////////////////////////////////////////////////////////////////////

KeyTypes keyTypesForKey(TypedValue k, KeyTypes b) {
  auto const a = [&] {
    if (isStringType(k.type())) {
      return k.val().pstr->isStatic() ? KeyTypes::StaticStrings
                                      : KeyTypes::Strings;
    } else if (isIntType(k.type())) {
      return KeyTypes::Ints;
    } else {
      return KeyTypes::Any;
    }
  }();

  if (a == b) return a;

  if (a == KeyTypes::Empty) return b;
  if (b == KeyTypes::Empty) return a;

  if ((a == KeyTypes::StaticStrings && b == KeyTypes::Strings) ||
      (b == KeyTypes::StaticStrings && a == KeyTypes::Strings)) {
    return KeyTypes::Strings;
  }

  return KeyTypes::Any;
}

std::pair<ValueTypes, DataType> valueTypesForValue(TypedValue v,
                                                   ValueTypes vms,
                                                   DataType type) {
  switch (vms) {
    case ValueTypes::Empty:
      return {ValueTypes::Monotype, v.type()};

    case ValueTypes::Monotype:
      if (equivDataTypes(type, v.type())) {
        return {ValueTypes::Monotype, type};
      } else if (isNullType(v.type())) {
        return {ValueTypes::MonotypeNullable, type};
      } else if (isNullType(type)) {
        return {ValueTypes::MonotypeNullable, v.type()};
      } else {
        return {ValueTypes::Any, kInvalidDataType};
      }

    case ValueTypes::MonotypeNullable:
      if (isNullType(v.type()) || equivDataTypes(type, v.type())) {
        return {ValueTypes::MonotypeNullable, type};
      } else {
        return {ValueTypes::Any, kInvalidDataType};
      }

    case ValueTypes::Any:
      return {ValueTypes::Any, kInvalidDataType};
  }
  not_reached();
}

//////////////////////////////////////////////////////////////////////////////

}

//////////////////////////////////////////////////////////////////////////////

bool EntryTypes::checkInvariants() const {
  assertx(IMPLIES(valueTypes == ValueTypes::Any
                  || valueTypes == ValueTypes::Empty,
          valueDatatype == kInvalidDataType));
  assertx(IMPLIES(valueTypes == ValueTypes::MonotypeNullable,
          valueDatatype != KindOfNull));
  return true;
}

EntryTypes EntryTypes::ForArray(ArrayData* ad) {
  auto state = EntryTypes(KeyTypes::Empty, ValueTypes::Empty,
                          kInvalidDataType);

  IterateKV(
    ad,
    [&](TypedValue k, TypedValue v) {
      state = state.withKV(k, v);
      return true;
    }
  );

  return state;
}

EntryTypes EntryTypes::withV(TypedValue v) const {
  auto const valuePair = valueTypesForValue(v, valueTypes, valueDatatype);

  return EntryTypes(keyTypes, valuePair.first, valuePair.second);
}

EntryTypes EntryTypes::withKV(TypedValue k, TypedValue v) const {
  auto const newKeyTypes = keyTypesForKey(k, keyTypes);
  auto const valuePair = valueTypesForValue(v, valueTypes, valueDatatype);

  return EntryTypes(newKeyTypes, valuePair.first, valuePair.second);
}

EntryTypes EntryTypes::pessimizeValueTypes() const {
  return EntryTypes(keyTypes, ValueTypes::Any,
                    kInvalidDataType);
}

std::string EntryTypes::toString() const {
  auto const keySt = [&] {
    switch (keyTypes) {
      case KeyTypes::Empty: return "Empty";
      case KeyTypes::Ints: return "Ints";
      case KeyTypes::StaticStrings: return "StaticStrings";
      case KeyTypes::Strings: return "Strings";
      case KeyTypes::Any: return "Any";
    }
    not_reached();
  }();
  auto const valueSt = [&] {
    switch (valueTypes) {
      case ValueTypes::Empty: return folly::sformat("Empty");
      case ValueTypes::Monotype:
        return folly::sformat("Monotype ({})", tname(valueDatatype));
      case ValueTypes::MonotypeNullable:
        return folly::sformat("MonotypeNullable ({})", tname(valueDatatype));
      case ValueTypes::Any: return folly::sformat("Any");
    }
    not_reached();
  }();

  return folly::sformat("<{}, {}>", keySt, valueSt);
}

}}
