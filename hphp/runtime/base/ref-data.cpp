/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2014 Facebook, Inc. (http://www.facebook.com)     |
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

#include "hphp/runtime/base/ref-data.h"

#include "hphp/runtime/base/types.h"
#include "hphp/runtime/base/variable-serializer.h"

namespace HPHP {

void RefData::compileTimeAsserts() {
  static_assert(offsetof(RefData, m_count) == FAST_REFCOUNT_OFFSET, "");
  static_assert(sizeof(RefData::m_count) == TypedValueAux::auxSize, "");
  static_assert(sizeof(DataType) == 1, "required for m_cow/z packing");
}

}
