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

#include "hphp/runtime/base/array-data.h"
#include "hphp/runtime/vm/iter.h"
#include "hphp/runtime/vm/jit/prof-data-serialize.h"
#include "hphp/runtime/vm/jit/type.h"

namespace HPHP { namespace jit {

///////////////////////////////////////////////////////////////////////////////

/*
 * Target profile for the distribution of array types seen by a given iterator.
 *
 * We aim to identify the best IterSpecialization for that iterator, along with
 * some stats about how long the arrays we iterate over are on average and how
 * many arrays we'd fail to specialize given that specialization.
 */
struct ArrayIterProfile {
  /*
   * The specialization information we've collected.
   */
  struct Result {
    size_t num_arrays = 0;
    size_t num_iterations = 0;
    size_t top_count = 0;
    IterSpecialization top_specialization =
      IterSpecialization::generic();
    Type value_type = TBottom;
  };

  /*
   * Returns the result above. Decisions are left to the caller.
   */
  Result result() const;

  /*
   * Update the profile when we iterate over a given array.
   *
   * `is_kviter` will be true for iterators over both keys and values.
   */
  void update(const ArrayData* arr, bool is_kviter);

  /*
   * Combine `l' and `r', summing across the kind counts.
   */
  static void reduce(ArrayIterProfile& l, const ArrayIterProfile& r);

  /*
   * Serialize this profile to JSON so we can dump it in debug mode.
   */
  folly::dynamic toDynamic() const;

  /*
   * Print this profile as a human-readable string.
   */
  std::string toString() const;


  /*
   * We need custom serialization because this profile contains a jit::Type.
   * A type may contain pointers to classes or to static constant values.
   */
  void serialize(ProfDataSerializer& ser) const;
  void deserialize(ProfDataDeserializer& ser);

private:
  // To ensure that `update` is constant time, we examine at most this many
  // values when updating m_value_type. Bases with a large number of values are
  // usually monotyped, so we don't get much benefit from examining more.
  static constexpr size_t kNumProfiledValues = 16;

  size_t m_num_iterations;
  uint32_t m_base_type_counts[IterSpecialization::kNumBaseTypes];
  uint32_t m_key_types_counts[IterSpecialization::kNumKeyTypes];

  // For bases that aren't a specialized base type, we increment m_generic
  // instead of any of the m_base_types elements.
  uint32_t m_generic_base_count;

  // We have a generic ArrayKey option in m_key_types, but we don't want to
  // increment it for empty bases, because empty bases fit all key types.
  uint32_t m_empty_count;

  // Track a TypeProfile for values. Zero-initialized types are TBottom.
  static_assert(Type::kBottom.empty(), "Assuming TBottom is 0");
  Type m_value_type;

  // In RDS, but can't contain pointers to request-allocated data.
  TYPE_SCAN_IGNORE_ALL;
};

///////////////////////////////////////////////////////////////////////////////

}}
