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
#include "hphp/runtime/base/array_init.h"
#include "hphp/runtime/base/policy_array.h"
#include "hphp/runtime/base/hphp_array.h"
#include "hphp/runtime/base/runtime_option.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////
// ArrayInit

HOT_FUNC
ArrayInit::ArrayInit(ssize_t n) {
  if (!n) {
    m_data = HphpArray::GetStaticEmptyArray();
  } else if (false) {
    // Force compilation of PolicyArray
    m_data = NEW(PolicyArray)(n);
  } else {
    m_data = ArrayData::Make(n);
  }
}

HOT_FUNC
ArrayInit::ArrayInit(ssize_t n, MapInit)
  : m_data(ArrayData::Make(n)) {
}

///////////////////////////////////////////////////////////////////////////////
}
