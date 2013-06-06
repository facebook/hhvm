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
#include "hphp/runtime/base/tv_comparisons.h"

#include "hphp/runtime/base/complex_types.h"
#include "hphp/runtime/base/comparisons.h"

namespace HPHP {

//////////////////////////////////////////////////////////////////////

bool tvSame(const TypedValue* tv1, const TypedValue* tv2) {
  bool const null1 = IS_NULL_TYPE(tv1->m_type);
  bool const null2 = IS_NULL_TYPE(tv2->m_type);
  if (null1 && null2) return true;
  if (null1 || null2) return false;

  if (tv1->m_type == KindOfRef) tv1 = tv1->m_data.pref->tv();
  if (tv2->m_type == KindOfRef) tv2 = tv2->m_data.pref->tv();

  switch (tv1->m_type) {
  case KindOfInt64:
  case KindOfBoolean:
    if (tv2->m_type != tv1->m_type) return false;
    return tv1->m_data.num == tv2->m_data.num;
  case KindOfDouble:
    if (tv2->m_type != tv1->m_type) return false;
    return tv1->m_data.dbl == tv2->m_data.dbl;

  case KindOfStaticString:
  case KindOfString:
    if (!IS_STRING_TYPE(tv2->m_type)) return false;
    return tv1->m_data.pstr->same(tv2->m_data.pstr);

  case KindOfArray:
    if (tv2->m_type != KindOfArray) return false;
    return tv1->m_data.parr->equal(tv2->m_data.parr, true);

  case KindOfObject:
    return tv2->m_type == KindOfObject &&
      tv1->m_data.pobj == tv2->m_data.pobj;

  default:
    break;
  }
  not_reached();
}

//////////////////////////////////////////////////////////////////////

}

