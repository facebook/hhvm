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

#include "hphp/runtime/ext_hhvm/ext_zend_compat.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

// TODO zPrepArgs needs to be updated so take care of
// boxing varargs
void zPrepArgs(ActRec* ar) {
  int32_t numArgs = ar->numArgs();
  TypedValue* args = (TypedValue*)ar - 1;
  for (int32_t i = 0; i < numArgs; ++i) {
    TypedValue* arg = args-i;
    if (arg->m_type != KindOfRef) {
      tvBox(arg);
    }
  }
}

///////////////////////////////////////////////////////////////////////////////
}

