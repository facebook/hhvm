/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010- Facebook, Inc. (http://www.facebook.com)         |
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
#include <runtime/base/array/array_init.h>
#include <runtime/base/array/zend_array.h>
#include <runtime/base/array/hphp_array.h>
#include <runtime/base/runtime_option.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////
// ArrayInit

HOT_FUNC
ArrayInit::ArrayInit(ssize_t n, bool keepRef /* = false */) {
  if (n == 0) {
    if (keepRef) {
      m_data = StaticEmptyZendArray::Get();
    } else {
      m_data = HphpArray::GetStaticEmptyArray();
    }
  } else {
    if (keepRef) {
      m_data = NEW(ZendArray)(n);
    } else {
      m_data = NEW(HphpArray)(n);
    }
  }
}

HOT_FUNC
ArrayData *ArrayInit::CreateVector(ssize_t n) {
  return NEW(HphpArray)(n);
}

HOT_FUNC
ArrayData *ArrayInit::CreateMap(ssize_t n) {
  return NEW(HphpArray)(n);
}

ArrayData *ArrayInit::CreateParams(int count, ...) {
  va_list ap;
  va_start(ap, count);
  ArrayInit ai(count);
  for (int i = 0; i < count; i++) {
    ai.setRef(*va_arg(ap, const Variant *));
  }
  va_end(ap);
  return ai.create();
}


///////////////////////////////////////////////////////////////////////////////
}
