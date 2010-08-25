/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010 Facebook, Inc. (http://www.facebook.com)          |
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

#include <runtime/base/global_array_wrapper.h>

namespace HPHP {

///////////////////////////////////////////////////////////////////////////////

ssize_t GlobalArrayWrapper::getIndex(CVarRef k) const {
  if (k.isInteger()) {
    return ((Array*)m_globals)->get()->getIndex(k.toInt64()) +
      m_globals->size();
  }
  String s = k.toString();
  return m_globals->getIndex(s.data(), s->hash());
}

///////////////////////////////////////////////////////////////////////////////
}
