/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010 Facebook, Inc. (http://www.facebook.com)          |
   | Copyright (c) 1998-2010 Zend Technologies Ltd. (http://www.zend.com) |
   +----------------------------------------------------------------------+
   | This source file is subject to version 2.00 of the Zend license,     |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | http://www.zend.com/license/2_00.txt.                                |
   | If you did not receive a copy of the Zend license and are unable to  |
   | obtain it through the world-wide-web, please send a note to          |
   | license@zend.com so we can mail you a copy immediately.              |
   +----------------------------------------------------------------------+
*/

#ifndef __HPHP_ZEND_COLLATOR_H__
#define __HPHP_ZEND_COLLATOR_H__

#include <unicode/coll.h> // icu

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

class Variant;
#define COLLATOR_SORT_REGULAR   0
#define COLLATOR_SORT_NUMERIC   1
#define COLLATOR_SORT_STRING    2

bool collator_sort(Variant &array, int sort_flags, bool ascending,
                   UCollator *coll, UErrorCode *errcode);
bool collator_asort(Variant &array, int sort_flags, bool ascending,
                    UCollator *coll, UErrorCode *errcode);

///////////////////////////////////////////////////////////////////////////////
}

#endif // __HPHP_ZEND_COLLATOR_H__
