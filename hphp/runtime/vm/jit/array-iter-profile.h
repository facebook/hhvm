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
#include "hphp/runtime/base/array-key-types.h"
#include "hphp/runtime/vm/iter.h"
#include "hphp/runtime/vm/jit/prof-data-serialize.h"
#include "hphp/runtime/vm/jit/type.h"

namespace HPHP::jit {

///////////////////////////////////////////////////////////////////////////////

/*
 * Target profile for the distribution of array types seen by a given iterator.
 *
 * We aim to identify the best key and value types for that iterator.
 */
struct ArrayIterProfile {
  /*
   * The specialization information we've collected.
   */
  struct Result {
    ArrayKeyTypes key_types = ArrayKeyTypes::Any();
    Type value_type = TInitCell;
  };

  /*
   * Returns the result above. Decisions are left to the caller.
   */
  Result result() const;

  /*
   * Update the profile when we iterate over a given array.
   */
  void update(const ArrayData* arr);

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

  ArrayKeyTypes m_key_types = ArrayKeyTypes::Empty();

  // Track a TypeProfile for values. Zero-initialized types are TBottom.
  static_assert(Type::kBottom.empty(), "Assuming TBottom is 0");
  Type m_value_type;

  // In RDS, but can't contain pointers to request-allocated data.
  TYPE_SCAN_IGNORE_ALL;
};

///////////////////////////////////////////////////////////////////////////////

}
