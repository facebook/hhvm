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

#include "hphp/runtime/base/type-object.h"

#include "hphp/runtime/base/array-iterator.h"
#include "hphp/runtime/base/builtin-functions.h"
#include "hphp/runtime/base/collections.h"
#include "hphp/runtime/base/strings.h"
#include "hphp/runtime/base/type-conversions.h"
#include "hphp/runtime/base/variable-serializer.h"

#include "hphp/runtime/ext/datetime/ext_datetime.h"

#include "hphp/system/systemlib.h"

namespace HPHP {

const Object Object::s_nullObject = Object();

///////////////////////////////////////////////////////////////////////////////

void Object::compileTimeAssertions() {
  static_assert(sizeof(Object) == sizeof(req::ptr<ObjectData>), "Fix this.");
}

void ObjNR::compileTimeAssertions() {
  static_assert(offsetof(ObjNR, m_px) == kExpectedMPxOffset, "");
}

Object::~Object() {
  // force it out of line
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

bool Object::equal(const Object& v2) const {
  if (m_obj == v2.m_obj) {
    return true;
  }
  if (!m_obj || !v2) {
    return false;
  }
  if (m_obj->isCollection()) {
    return collections::equals(get(), v2.get());
  }
  if (UNLIKELY(m_obj->instanceof(SystemLib::s_DateTimeInterfaceClass))) {
    return DateTimeData::getTimestamp(*this) ==
        DateTimeData::getTimestamp(v2);
  }
  if (v2.get()->getVMClass() != m_obj->getVMClass()) {
    return false;
  }
  if (UNLIKELY(m_obj->instanceof(SystemLib::s_ArrayObjectClass))) {
    // Compare the whole object, not just the array representation
    auto ar1 = Array::Create();
    auto ar2 = Array::Create();
    m_obj->o_getArray(ar1);
    v2->o_getArray(ar2);
    return ar1->equal(ar2.get(), false);
  }
  return toArray().equal(v2.toArray());
}

bool Object::less(const Object& v2) const {
  check_collection_compare(get(), v2.get());
  if (UNLIKELY(m_obj->instanceof(SystemLib::s_DateTimeInterfaceClass))) {
    return DateTimeData::getTimestamp(*this) <
        DateTimeData::getTimestamp(v2);
  }
  return m_obj != v2.m_obj && toArray().less(v2.toArray());
}

bool Object::more(const Object& v2) const {
  check_collection_compare(get(), v2.get());
  if (UNLIKELY(m_obj->instanceof(SystemLib::s_DateTimeInterfaceClass))) {
    return DateTimeData::getTimestamp(*this) >
        DateTimeData::getTimestamp(v2);
  }
  return m_obj != v2.m_obj && toArray().more(v2.toArray());
}

static Variant warn_non_object() {
  raise_notice("Cannot access property on non-object");
  return uninit_null();
}

Variant Object::o_get(const String& propName, bool error /* = true */,
                      const String& context /* = null_string */) const {
  if (UNLIKELY(!m_obj)) return warn_non_object();
  return m_obj->o_get(propName, error, context);
}

Variant Object::o_set(const String& propName, const Variant& val,
                      const String& context /* = null_string */) {
  if (!m_obj) {
    setToDefaultObject();
  }
  return m_obj->o_set(propName, val, context);
}

const char* Object::classname_cstr() const {
  return m_obj->getClassName().c_str();
}

///////////////////////////////////////////////////////////////////////////////
// output

void Object::serialize(VariableSerializer *serializer) const {
  if (m_obj) {
    m_obj->serialize(serializer);
  } else {
    serializer->writeNull();
  }
}

void Object::setToDefaultObject() {
  raise_warning(Strings::CREATING_DEFAULT_OBJECT);
  operator=(SystemLib::AllocStdClassObject());
}

///////////////////////////////////////////////////////////////////////////////
}
