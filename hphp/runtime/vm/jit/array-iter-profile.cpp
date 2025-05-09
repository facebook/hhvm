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

#include "hphp/runtime/vm/jit/array-iter-profile.h"

#include "hphp/runtime/base/array-data.h"
#include "hphp/runtime/base/array-iterator.h"
#include "hphp/runtime/base/vanilla-dict.h"

#include <string>
#include <sstream>

namespace HPHP::jit {

///////////////////////////////////////////////////////////////////////////////

namespace {

ArrayKeyTypes getArrayKeyTypes(const ArrayData* arr) {
  switch (arr->kind()) {
    case ArrayData::kVecKind:
    case ArrayData::kBespokeVecKind:
      return ArrayKeyTypes::Ints();

    case ArrayData::kDictKind:
      return VanillaDict::as(arr)->keyTypes();

    case ArrayData::kKeysetKind:
    case ArrayData::kBespokeKeysetKind:
      return ArrayKeyTypes::Ints() | ArrayKeyTypes::Strs();

    case ArrayData::kBespokeDictKind:
      return ArrayKeyTypes::Any();

    case ArrayData::kNumKinds:
      always_assert(false);
  }
  always_assert(false);
}

}

///////////////////////////////////////////////////////////////////////////////

ArrayIterProfile::Result ArrayIterProfile::result() const {
  return { m_key_types, m_value_type };
}

void ArrayIterProfile::update(const ArrayData* arr) {
  // Generally speaking, bespoke arrays that we encounter during profiling
  // should be LoggingArrays. Don't include them in our profile.
  if (!arr->isVanilla()) return;

  m_key_types.copyFrom(getArrayKeyTypes(arr), false /* compact */);

  size_t num_profiled_values = 0;
  IterateV(arr, [&](TypedValue v) {
    m_value_type |= typeFromTV(&v, nullptr);
    return ++num_profiled_values == kNumProfiledValues;
  });
}

void ArrayIterProfile::reduce(ArrayIterProfile& l, const ArrayIterProfile& r) {
  l.m_key_types.copyFrom(r.m_key_types, false /* compact */);
  l.m_value_type |= r.m_value_type;
}

folly::dynamic ArrayIterProfile::toDynamic() const {
  using folly::dynamic;

  return dynamic::object("keyTypes", m_key_types.show())
                        ("valueType", m_value_type.toString())
                        ("profileType", "ArrayIterProfile");
}

std::string ArrayIterProfile::toString() const {
  std::ostringstream out;
  out << "ValueType: " << m_value_type.toString();
  out << "\nKeyTypes: " << m_key_types.show();
  return out.str();
}

void ArrayIterProfile::serialize(ProfDataSerializer& ser) const {
  auto const type_offset = offsetof(ArrayIterProfile, m_value_type);
  write_raw(ser, this, type_offset);
  m_value_type.serialize(ser);
}

void ArrayIterProfile::deserialize(ProfDataDeserializer& ser) {
  auto const type_offset = offsetof(ArrayIterProfile, m_value_type);
  read_raw(ser, this, type_offset);
  m_value_type = Type::deserialize(ser);
}

///////////////////////////////////////////////////////////////////////////////

}
