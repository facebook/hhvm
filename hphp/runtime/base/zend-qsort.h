/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2013 Facebook, Inc. (http://www.facebook.com)     |
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

#ifndef incl_HPHP_ZEND_QSORT_H_
#define incl_HPHP_ZEND_QSORT_H_

#include <stdlib.h>
#include <limits.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

void _zend_qsort_swap(void *a, void *b, size_t siz);

typedef int (*compare_func_t)(const void *, const void *, const void *opaque);
void zend_qsort(void *base, size_t nmemb, size_t siz,
                compare_func_t compare, void *opaque);

///////////////////////////////////////////////////////////////////////////////
}

#endif // incl_HPHP_ZEND_QSORT_H_
