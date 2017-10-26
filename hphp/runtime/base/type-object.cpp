/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-present Facebook, Inc. (http://www.facebook.com)  |
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

#include "hphp/runtime/base/type-object.h"

#include "hphp/runtime/base/array-iterator.h"
#include "hphp/runtime/base/builtin-functions.h"
#include "hphp/runtime/base/collections.h"
#include "hphp/runtime/base/strings.h"
#include "hphp/runtime/base/variable-serializer.h"

#include "hphp/runtime/ext/datetime/ext_datetime.h"

#include "hphp/system/systemlib.h"

namespace HPHP {

extern const Object null_object = Object();

///////////////////////////////////////////////////////////////////////////////

void Object::compileTimeAssertions() {
  static_assert(sizeof(Object) == sizeof(req::ptr<ObjectData>), "Fix this.");
}

void ObjNR::compileTimeAssertions() {
  static_assert(offsetof(ObjNR, m_px) == kExpectedMPxOffset, "");
}

Array Object::toArray() const {
  return m_obj ? m_obj->toArray() : Array();
}

String Object::toString() const {
  return m_obj ? m_obj->invokeToString() : String();
}

int64_t Object::toInt64ForCompare() const {
  check_collection_compare(get());
  return toInt64();
}

double Object::toDoubleForCompare() const {
  check_collection_compare(get());
  return toDouble();
}

const char* Object::classname_cstr() const {
  return m_obj->getClassName().c_str();
}

///////////////////////////////////////////////////////////////////////////////
}
