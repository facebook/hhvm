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

#ifndef incl_ZVAL_HELPERS_H_
#define incl_ZVAL_HELPERS_H_

#include "hphp/runtime/ext_zend_compat/php-src/Zend/zend_types.h"

// Assumes 'tv' is dead
inline void tvWriteZval(zval* pref, HPHP::TypedValue* tv) {
  tv->m_type = HPHP::KindOfRef;
  tv->m_data.pref = pref;
  // including zend.h is suicide in here, so just inline zval_addref_p
  pref->zAddRef();
  pref->zRefcount();
}

// Assumes 'tv' is live
inline void tvSetZval(zval* pref, HPHP::TypedValue* tv) {
  assert(tvIsPlausible(*tv));
  auto const oldType = tv->m_type;
  auto const oldDatum = tv->m_data.num;
  tvWriteZval(pref, tv);
  tvRefcountedDecRefHelper(oldType, oldDatum);
}

#endif // incl_ZVAL_HELPERS_H_
