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
#ifndef incl_HHBBC_EVAL_CELL_H_
#define incl_HHBBC_EVAL_CELL_H_

#include <stdexcept>

#include "hphp/runtime/base/complex-types.h"

#include "hphp/hhbbc/type-system.h"

namespace HPHP { namespace HHBBC {

//////////////////////////////////////////////////////////////////////

/*
 * When constant-evaluating certain operations, it's possible they
 * will return non-static objects, or throw exceptions (e.g. cellAdd()
 * with an array and an int).
 *
 * This routine converts these things back to types.  In the case of
 * an exception it returns TInitCell.
 */
template<class Pred>
Type eval_cell(Pred p) {
  try {
    Cell c = p();
    if (IS_REFCOUNTED_TYPE(c.m_type)) {
      switch (c.m_type) {
      case KindOfString:
        {
          auto const sstr = makeStaticString(c.m_data.pstr);
          tvDecRef(&c);
          c = make_tv<KindOfStaticString>(sstr);
        }
        break;
      case KindOfArray:
        {
          auto const sarr = ArrayData::GetScalarArray(c.m_data.parr);
          tvDecRef(&c);
          c = make_tv<KindOfArray>(sarr);
        }
        break;
      default:
        always_assert(0 && "Impossible constant evaluation occurred");
      }
    }
    return from_cell(c);
  } catch (const std::exception&) {
    /*
     * Not currently trying to set the nothrow() flag based on whether
     * or not this catch block is hit.  To do it correctly, it will
     * require checking whether the non-exceptioning cases above
     * possibly entered the error handler by running a raise_notice or
     * similar.
     */
    return TInitCell;
  }
}

//////////////////////////////////////////////////////////////////////

}}

#endif
