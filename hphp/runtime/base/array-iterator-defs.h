/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2013 Facebook, Inc. (http://www.facebook.com)     |
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
#ifndef incl_HPHP_ARRAY_ITERATOR_DEFS_H_
#define incl_HPHP_ARRAY_ITERATOR_DEFS_H_

#include "folly/Likely.h"

namespace HPHP {

//////////////////////////////////////////////////////////////////////

/*
 * Fast check for whether any strong iterators exist in the entire
 * program.  Most strong iterator operations from arrays should be
 * guarded on checking this first, and placed in an unlikely path.
 */
inline bool strong_iterators_exist() {
  return tl_miter_table.ents[0].array != nullptr;
}

template<class Fn> NEVER_INLINE
void for_each_strong_iterator_slow(Fn fn) {
  tl_miter_table.extras.for_each(fn);
}

/*
 * The expectation with uses of for_each_strong_iterator is that most
 * callers will already test strong_iterators_exist, so we don't try
 * to fast path the case of !strong_iterators_exist().
 *
 * This is considered a precondition for now to make sure code is
 * doing it right.
 */
template<class Fn>
void for_each_strong_iterator(Fn fn) {
  assert(strong_iterators_exist());
  fn(tl_miter_table.ents[0]);
  fn(tl_miter_table.ents[1]);
  fn(tl_miter_table.ents[2]);
  fn(tl_miter_table.ents[3]);
  fn(tl_miter_table.ents[4]);
  fn(tl_miter_table.ents[5]);
  fn(tl_miter_table.ents[6]);
  static_assert(tl_miter_table.ents.size() == 7, "");
  if (UNLIKELY(!tl_miter_table.extras.empty())) {
    for_each_strong_iterator_slow(fn);
  }
}

//////////////////////////////////////////////////////////////////////

}

#endif
