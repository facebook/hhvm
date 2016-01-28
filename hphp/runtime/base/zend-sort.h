/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2016 Facebook, Inc. (http://www.facebook.com)     |
   | Copyright (c) 2014-2015 Etsy, Inc. (http://www.etsy.com)             |
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

#ifndef incl_HPHP_ZEND_SORT_H_
#define incl_HPHP_ZEND_SORT_H_

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

struct Variant;

bool zend_sort(Variant& array, int sort_flags, bool ascending);
bool zend_asort(Variant& array, int sort_flags, bool ascending);
bool zend_ksort(Variant &array, int sort_flags, bool ascending);

///////////////////////////////////////////////////////////////////////////////
}

#endif // incl_HPHP_ZEND_SORT_H_
