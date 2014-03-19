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
#include "hphp/runtime/base/complex-types.h"
#include "hphp/runtime/base/strings.h"
#include "hphp/runtime/base/type-conversions.h"
#include "hphp/runtime/base/variable-serializer.h"

#include "hphp/runtime/ext/ext_collections.h"
#include "hphp/runtime/ext/ext_datetime.h"

#include "hphp/system/systemlib.h"

namespace HPHP {

const Object Object::s_nullObject = Object();

///////////////////////////////////////////////////////////////////////////////

void Object::compileTimeAssertions() {
  static_assert(sizeof(Object) == sizeof(ObjectBase), "Fix this.");
}

void ObjNR::compileTimeAssertions() {
  static_assert(offsetof(ObjNR, m_px) == kExpectedMPxOffset, "");
}

Object::~Object() {
  // force it out of line
}

Array Object::toArray() const {
  return m_px ? m_px->o_toArray() : Array();
}

String Object::toString() const {
  return m_px ? m_px->invokeToString() : String();
}

int64_t Object::toInt64ForCompare() const {
  check_collection_compare(m_px);
  return toInt64();
}

double Object::toDoubleForCompare() const {
  check_collection_compare(m_px);
  return toDouble();
}

bool Object::equal(const Object& v2) const {
  if (m_px == v2.get()) {
    return true;
  }
  if (!m_px || !v2.get()) {
    return false;
  }
  if (m_px->isCollection()) {
    return collectionEquals(m_px, v2.get());
  }
  if (UNLIKELY(m_px->instanceof(SystemLib::s_DateTimeInterfaceClass))) {
    return c_DateTime::GetTimestamp(*this) ==
        c_DateTime::GetTimestamp(v2);
  }
  if (v2.get()->getVMClass() != m_px->getVMClass()) {
    return false;
  }
  if (UNLIKELY(m_px->instanceof(SystemLib::s_ArrayObjectClass))) {
    // Compare the whole object, not just the array representation
    Array ar1(ArrayData::Create());
    Array ar2(ArrayData::Create());
    m_px->o_getArray(ar1, false);
    v2->o_getArray(ar2, false);
    return ar1->equal(ar2.get(), false);
  }
  return toArray().equal(v2.toArray());
}

bool Object::less(const Object& v2) const {
  check_collection_compare(m_px, v2.get());
  if (UNLIKELY(m_px->instanceof(SystemLib::s_DateTimeInterfaceClass))) {
    return c_DateTime::GetTimestamp(*this) <
        c_DateTime::GetTimestamp(v2);
  }
  return m_px != v2.m_px && toArray().less(v2.toArray());
}

bool Object::more(const Object& v2) const {
  check_collection_compare(m_px, v2.get());
  if (UNLIKELY(m_px->instanceof(SystemLib::s_DateTimeInterfaceClass))) {
    return c_DateTime::GetTimestamp(*this) >
        c_DateTime::GetTimestamp(v2);
  }
  return m_px != v2.m_px && toArray().more(v2.toArray());
}

static Variant warn_non_object() {
  raise_warning("Cannot access property on non-object");
  return uninit_null();
}

Variant Object::o_get(const String& propName, bool error /* = true */,
                      const String& context /* = null_string */) const {
  if (UNLIKELY(!m_px)) return warn_non_object();
  return m_px->o_get(propName, error, context);
}

Variant Object::o_set(const String& propName, const Variant& val,
                      const String& context /* = null_string */) {
  if (!m_px) {
    setToDefaultObject();
  }
  return m_px->o_set(propName, val, context);
}

Variant Object::o_setRef(const String& propName, const Variant& val,
                         const String& context /* = null_string */) {
  if (!m_px) {
    setToDefaultObject();
  }
  return m_px->o_setRef(propName, val, context);
}

Variant Object::o_set(const String& propName, RefResult val,
                      const String& context /* = null_string */) {
  return o_setRef(propName, variant(val), context);
}

const char* Object::classname_cstr() const {
  return m_px->o_getClassName().c_str();
}

///////////////////////////////////////////////////////////////////////////////
// output

void Object::serialize(VariableSerializer *serializer) const {
  if (m_px) {
    m_px->serialize(serializer);
  } else {
    serializer->writeNull();
  }
}

bool Object::unserialize(std::istream &in) {
  throw NotImplementedException(__func__);
}

void Object::setToDefaultObject() {
  raise_warning(Strings::CREATING_DEFAULT_OBJECT);
  operator=(SystemLib::AllocStdClassObject());
}

///////////////////////////////////////////////////////////////////////////////
}
