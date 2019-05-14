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

#ifndef HPHP_ARRAY_PROVENANCE_H
#define HPHP_ARRAY_PROVENANCE_H

#include "hphp/runtime/base/runtime-option.h"
#include "hphp/runtime/base/rds-local.h"
#include "hphp/runtime/base/typed-value.h"
#include "hphp/runtime/base/types.h"
#include "hphp/util/low-ptr.h"

namespace HPHP {

struct StringData;

namespace arrprov {

/*
 * A provenance annotation
 *
 * We need to store the filename and line since when assembling units, we
 * don't necessarily have the final Unit allocated yet. It may be faster to
 * make this a tagged union or store a different Tag type for static arrays
 */
struct Tag {
  Tag() = default;
  Tag(const StringData* filename, int line)
    : m_filename(filename)
    , m_line(line) {}

  const StringData* filename() const { return m_filename; }
  int line() const { return m_line; }

  bool operator==(const Tag& other) const {
    return m_filename == other.m_filename &&
           m_line == other.m_line;
  }
  bool operator!=(const Tag& other) const { return !(*this == other); }

private:
  const StringData* m_filename{nullptr};
  int m_line{0};
};

/*
 * This is a separate struct so it can live in RDS and not be GC scanned--the
 * actual RDS-local handle is kept in the implementation
 */
struct ArrayProvenanceTable {
  /* The table itself -- allocated in general heap */
  folly::F14FastMap<const ArrayData*, Tag> tags;

  /*
   * We never dereference ArrayData*s from this table--so it's safe for the GC
   * to ignore them in this table
   */
  TYPE_SCAN_IGNORE_FIELD(tags);
};

/*
 * Create a tag based on the current VMPC and unit.
 * Requires VM regs to be synced or for a sync point to be available
 */
Tag tagFromProgramCounter();

/*
 * `HPHP::arrprov::unchecked` operates on provenance tags without checking to
 * see if the instrumentation is currently enabled or not. Since these
 * functions call each other, we want to check only once. Variants that check
 * the option are in the `checked` namespace, which is inline and therefore
 * those can be accessed merely as HPHP::arrprov::whatever
 */

namespace unchecked {

bool arrayWantsTag(const ArrayData* ad);

/*
 * Get the provenance tag for a given array--regardless of if it is
 * static or not.
 */
folly::Optional<Tag> getTag(const ArrayData* ad);

/*
 * Create a tag based on the current VMPC and unit.
 * Requires VM regs to be synced or for a sync point to be available
 */
void setTag(ArrayData* ad, const Tag& tag);

inline void copyTag(const ArrayData* src, ArrayData* dest) {
  if (auto const tag = getTag(src)) {
    setTag(dest, *tag);
  } else {
    setTag(dest, tagFromProgramCounter());
  }
}

/*
 * Clear a tag for a released array--only call this if the array
 * is henceforth unreachable
 */
void clearTag(const ArrayData* ad);

} // namespace unchecked

/*
 * See the note above (on namespace unchecked)
 * for why this namespace exists and is inline
 */
inline namespace checked {

inline folly::Optional<Tag> getTag(const ArrayData* ad) {
  if (!RuntimeOption::EvalLogArrayProvenance) return {};
  return unchecked::getTag(ad);
}
inline void setTag(ArrayData* ad, const Tag& tag) {
  if (!RuntimeOption::EvalLogArrayProvenance) return;
  unchecked::setTag(ad, tag);
}
inline void copyTag(const ArrayData* src, ArrayData* dest) {
  if (!RuntimeOption::EvalLogArrayProvenance) return;
  unchecked::copyTag(src, dest);
}
inline void clearTag(ArrayData* ad) {
  if (!RuntimeOption::EvalLogArrayProvenance) return;
  unchecked::clearTag(ad);
}
inline void copyTagStatic(const ArrayData* src, ArrayData* dest) {
  if (!RuntimeOption::EvalLogArrayProvenance) return;
  if (auto const tag = unchecked::getTag(src)) {
    unchecked::setTag(dest, *tag);
  }
}

} // inline namespace checked

}} // namespace HPHP::arrprov

#endif // HPHP_ARRAY_PROVENANCE_H
