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
#include <runtime/base/array/vector_array.h>
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
      if (enable_vector_array && RuntimeOption::UseVectorArray) {
        m_data = StaticEmptyVectorArray::Get();
      } else if (hhvm || // HHVM always uses HphpArray
                (enable_hphp_array && RuntimeOption::UseHphpArray)) {
        m_data = HphpArray::GetStaticEmptyArray();
      } else {
        m_data = StaticEmptyZendArray::Get();
      }
    }
  } else {
    if (keepRef) {
      m_data = NEW(ZendArray)(n);
    } else {
      if (hhvm || // HHVM always uses HphpArray
                 (enable_hphp_array && RuntimeOption::UseHphpArray)) {
        m_data = NEW(HphpArray)(n);
      } else {
        m_data = NEW(ZendArray)(n);
      }
    }
  }
}

HOT_FUNC
ArrayData *ArrayInit::CreateVector(ssize_t n) {
  if (enable_vector_array && RuntimeOption::UseVectorArray) {
    return NEW(VectorArray)(n);
  }
  if (hhvm || // HHVM always uses HphpArray
      (enable_hphp_array && RuntimeOption::UseHphpArray)) {
    return NEW(HphpArray)(n);
  }
  return NEW(ZendArray)(n);
}

HOT_FUNC
ArrayData *ArrayInit::CreateMap(ssize_t n) {
  if (hhvm || // HHVM always uses HphpArray
      (enable_hphp_array && RuntimeOption::UseHphpArray)) {
    return NEW(HphpArray)(n);
  }
  return NEW(ZendArray)(n);
}

ArrayData *ArrayInit::CreateParams(int count, ...) {
  if (enable_vector_array && RuntimeOption::UseVectorArray) {
    va_list ap;
    va_start(ap, count);
    ArrayInit ai(count, vectorInit);
    for (int i = 0; i < count; i++) {
      ai.setRef(*va_arg(ap, const Variant *));
    }
    va_end(ap);
    return ai.create();
  }
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
