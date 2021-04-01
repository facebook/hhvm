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
#include "hphp/runtime/base/mixed-array.h"
#include "hphp/runtime/base/set-array.h"

#include <folly/Format.h>

#include <cassert>
#include <cmath>
#include <string>
#include <sstream>

namespace HPHP { namespace jit {

///////////////////////////////////////////////////////////////////////////////

namespace {

IterSpecialization getIterSpecialization(const ArrayData* arr) {
  using S = IterSpecialization;
  auto result = IterSpecialization::generic();

  switch (arr->kind()) {
    case ArrayData::kVecKind: {
      result.specialized = true;
      result.base_type = S::Vec;
      result.key_types = S::Int;
      return result;
    }

    case ArrayData::kDictKind: {
      auto const keys = MixedArray::asMixed(arr)->keyTypes();
      if (keys.mayIncludeTombstone()) return result;
      result.specialized = true;
      result.base_type = S::Dict;
      result.key_types = [&]{
        if (keys.mustBeStaticStrs()) return S::StaticStr;
        if (keys.mustBeInts()) return S::Int;
        if (keys.mustBeStrs()) return S::Str;
        return S::ArrayKey;
      }();
      return result;
    }

    default: return result;
  }
}

}

///////////////////////////////////////////////////////////////////////////////

ArrayIterProfile::Result ArrayIterProfile::result() const {
  ArrayIterProfile::Result result;
  assertx(!result.top_specialization.specialized);
  result.value_type = m_value_type;

  result.num_arrays = m_generic_base_count;
  result.num_iterations = m_num_iterations;
  for (uint32_t i = 0; i < IterSpecialization::kNumBaseTypes; ++i) {
    auto const count = m_base_type_counts[i];
    result.num_arrays += count;
    if (count > result.top_count) {
      result.top_specialization.base_type = (IterSpecialization::BaseType)i;
      result.top_specialization.specialized = true;
      result.top_count = count;
    }
  }

  auto key_types_total = m_empty_count;
  for (uint32_t i = 0; i < IterSpecialization::kNumKeyTypes; ++i) {
    key_types_total += m_key_types_counts[i];
  }
  auto const counts  = m_key_types_counts;
  auto const statics = m_empty_count + counts[IterSpecialization::StaticStr];
  auto const ints    = m_empty_count + counts[IterSpecialization::Int];
  auto const strs    = statics + counts[IterSpecialization::Str];
  result.top_specialization.key_types = [&]{
    auto const type = result.top_specialization.base_type;
    if (type == IterSpecialization::Vec) return IterSpecialization::Int;
    if (statics == key_types_total)      return IterSpecialization::StaticStr;
    if (ints    == key_types_total)      return IterSpecialization::Int;
    if (strs    == key_types_total)      return IterSpecialization::Str;
    return IterSpecialization::ArrayKey;
  }();
  return result;
}

void ArrayIterProfile::update(const ArrayData* arr, bool is_kviter) {
  // Generally speaking, bespoke arrays that we encounter during profiling
  // should be LoggingArrays. Don't include them in our profile.
  if (!arr->isVanilla()) return;

  auto const size = arr->size();
  auto const specialization = getIterSpecialization(arr);
  if (specialization.specialized) {
    if (arr->empty()) {
      m_empty_count++;
    } else {
      m_key_types_counts[specialization.key_types]++;
    }
    m_num_iterations += size;
    m_base_type_counts[specialization.base_type]++;
    size_t num_profiled_values = 0;
    IterateKV(arr, [&](TypedValue k, TypedValue v) {
      m_value_type |= typeFromTV(&v, nullptr);
      return ++num_profiled_values == kNumProfiledValues;
    });
  } else {
    // We'll check for generic bases and exit the translation when initializing
    // a specialized iterator, so we shouldn't update the key_type counts, the
    // value_type, or the num_iterations for these bases.
    m_generic_base_count++;
  }
  auto const array_index = size == 0 ? 0 : 1 + (size_t)std::floor(std::log2(size));
  assertx(array_index < kNumApproximateCountBuckets);
  m_approximate_iteration_buckets[array_index]++;
}

void ArrayIterProfile::reduce(ArrayIterProfile& l, const ArrayIterProfile& r) {
  for (uint32_t i = 0; i < IterSpecialization::kNumBaseTypes; ++i) {
    l.m_base_type_counts[i] += r.m_base_type_counts[i];
  }
  for (uint32_t i = 0; i < IterSpecialization::kNumKeyTypes; ++i) {
    l.m_key_types_counts[i] += r.m_key_types_counts[i];
  }
  l.m_num_iterations += r.m_num_iterations;
  l.m_generic_base_count += r.m_generic_base_count;
  l.m_empty_count += r.m_empty_count;
  l.m_value_type |= r.m_value_type;
  for (uint32_t i = 0; i < kNumApproximateCountBuckets; ++i) {
    l.m_approximate_iteration_buckets[i] += r.m_approximate_iteration_buckets[i];
  }
}

folly::dynamic ArrayIterProfile::toDynamic() const {
  using folly::dynamic;

  dynamic base_type = dynamic::object();
  for (uint32_t i = 0; i < IterSpecialization::kNumBaseTypes; ++i) {
    auto const str = show((IterSpecialization::BaseType)i);
    base_type[str] = m_base_type_counts[i];
  }
  base_type["Generic"] = m_generic_base_count;

  dynamic key_types = dynamic::object();
  for (uint32_t i = 0; i < IterSpecialization::kNumKeyTypes; ++i) {
    auto const str = show((IterSpecialization::KeyTypes)i);
    key_types[str] = m_key_types_counts[i];
  }
  key_types["Empty"] = m_empty_count;

  dynamic approx_counts = dynamic::object();
  for (uint32_t i = 0; i < kNumApproximateCountBuckets; ++i) {
    auto const count_for_bucket = m_approximate_iteration_buckets[i];
    if (count_for_bucket == 0) {
      continue;
    }
    approx_counts[std::to_string(i)] = count_for_bucket;
  }

  return dynamic::object("keyTypes", key_types)
                        ("baseType", base_type)
                        ("valueType", m_value_type.toString())
                        // consider adding the base here so you know
                        // what the buckets refer to without having to
                        // assert it
                        ("numIterations", m_num_iterations)
                        ("approximateCounts", approx_counts)
                        ("profileType", "ArrayIterProfile");
}

std::string ArrayIterProfile::toString() const {
  std::ostringstream out;
  out << "NumIterations: " << m_num_iterations;
  out << "\nValueType: " << m_value_type.toString();
  out << "\n\nBaseType:";
  for (uint32_t i = 0; i < IterSpecialization::kNumBaseTypes; ++i) {
    auto const str = show((IterSpecialization::BaseType)i);
    out << "\n  " << str << ": " << m_base_type_counts[i];
  }
  out << "\n  Generic" << ": " << m_generic_base_count;
  out << "\n\nKeyTypes:";
  for (uint32_t i = 0; i < IterSpecialization::kNumKeyTypes; ++i) {
    auto const str = show((IterSpecialization::KeyTypes)i);
    out << "\n  " << str << ": " << m_key_types_counts[i];
  }
  out << "\n  Empty" << ": " << m_empty_count;
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

}}
