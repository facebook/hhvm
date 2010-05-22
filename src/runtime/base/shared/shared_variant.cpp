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

#include <boost/foreach.hpp>
#include <runtime/base/shared/shared_variant.h>
#include <runtime/ext/ext_variable.h>
#include <runtime/base/shared/shared_map.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

int SharedVariant::countReachable() {
#ifdef DEBUG_APC_LEAK
  markReachable();
#endif
  int count = 1;
  if (m_type == KindOfArray) {
    int size = arrSize();
    count += size; // key count
    for (int i = 0; i < size; i++) {
      SharedVariant* p = getValue(i);
      count += p->countReachable();
      p = getKey(i);
      p->countReachable();
    }
  }
  return count;
}

///////////////////////////////////////////////////////////////////////////////
}
