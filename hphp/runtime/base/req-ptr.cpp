/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2015 Facebook, Inc. (http://www.facebook.com)     |
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

#include "hphp/runtime/base/req-ptr.h"
#include "hphp/runtime/base/object-data.h"
#include "hphp/runtime/base/resource-data.h"
#include "hphp/runtime/base/exceptions.h"
#include "hphp/runtime/base/type-object.h"
#include "hphp/runtime/base/type-resource.h"
#include "hphp/runtime/base/type-variant.h"

namespace HPHP {

void throw_invalid_object_type(ResourceData* p) {
  if (!p) throw_null_pointer_exception();
  throw_invalid_object_type(p->o_getClassName().c_str());
}

void throw_invalid_object_type(ObjectData* p) {
  if (!p) throw_null_pointer_exception();
  throw_invalid_object_type(p->getClassName().c_str());
}

void throw_invalid_object_type(const Resource& p) {
  throw_invalid_object_type(deref<ResourceData>(p));
}

void throw_invalid_object_type(const Object& p) {
  throw_invalid_object_type(p.get());
}

void throw_invalid_object_type(const Variant& p) {
  auto tv = p.asCell();
  switch (tv->m_type) {
    case KindOfNull:
    case KindOfUninit:
      throw_null_pointer_exception();
    case KindOfObject:
      throw_invalid_object_type(tv->m_data.pobj->getClassName().c_str());
    case KindOfResource:
      throw_invalid_object_type(tv->m_data.pres->o_getClassName().c_str());
    default:
      throw_invalid_object_type(tname(tv->m_type).c_str());
  }
}

}
