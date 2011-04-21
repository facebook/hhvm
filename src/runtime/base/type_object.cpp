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

#include <runtime/base/complex_types.h>
#include <runtime/base/type_conversions.h>
#include <runtime/base/builtin_functions.h>
#include <runtime/base/variable_serializer.h>
#include <runtime/base/array/array_iterator.h>

#include <system/lib/systemlib.h>

namespace HPHP {

const Object Object::s_nullObject = Object();

///////////////////////////////////////////////////////////////////////////////

ArrayIter Object::begin(CStrRef context /* = null_string */,
                        bool setIterDirty /* = false */) const {
  if (!m_px) throw_null_pointer_exception();
  return m_px->begin(context);
}

MutableArrayIter Object::begin(Variant *key, Variant &val,
                               CStrRef context /* = null_string */,
                               bool setIterDirty /* = false */) const {
  if (!m_px) throw_null_pointer_exception();
  return m_px->begin(key, val, context);
}

Array Object::toArray() const {
  return m_px ? m_px->o_toArray() : Array();
}

Variant Object::toKey() const {
  return m_px ? (isResource() ? m_px->o_toInt64() : m_px->t___tostring())
    : String();
}

bool Object::equal(CObjRef v2) const {
  if (m_px == v2.get()) {
    return true;
  }
  if (!m_px || !v2.get()) {
    return false;
  }
  if (isResource() || v2.isResource()) {
    return false;
  }
  return (v2.get()->o_isClass(m_px->o_getClassName()) &&
          toArray().equal(v2.toArray()));
}

bool Object::less(CObjRef v2) const {
  return m_px != v2.m_px && toArray().less(v2.toArray());
}

bool Object::more(CObjRef v2) const {
  return m_px != v2.m_px && toArray().more(v2.toArray());
}

Variant Object::o_get(CStrRef propName, bool error /* = true */,
                      CStrRef context /* = null_string */) const {
  if (!m_px) throw_null_pointer_exception();
  return m_px->o_get(propName, error, context);
}

Variant Object::o_getPublic(CStrRef propName, bool error /* = true */) const {
  if (!m_px) throw_null_pointer_exception();
  return m_px->o_getPublic(propName, error);
}

Variant Object::o_set(CStrRef propName, CVarRef val,
                      CStrRef context /* = null_string */) {
  if (!m_px) {
    operator=(SystemLib::AllocStdClassObject());
  }
  return m_px->o_set(propName, val, false, context);
}

Variant Object::o_setRef(CStrRef propName, CVarRef val,
                         CStrRef context /* = null_string */) {
  if (!m_px) {
    operator=(SystemLib::AllocStdClassObject());
  }
  return m_px->o_setRef(propName, val, false, context);
}

Variant Object::o_set(CStrRef propName, RefResult val,
                      CStrRef context /* = null_string */) {
  return o_setRef(propName, variant(val), context);
}

Variant Object::o_setPublic(CStrRef propName, CVarRef val) {
  if (!m_px) {
    operator=(SystemLib::AllocStdClassObject());
  }
  return m_px->o_setPublic(propName, val, false);
}

Variant Object::o_setPublicRef(CStrRef propName, CVarRef val) {
  if (!m_px) {
    operator=(SystemLib::AllocStdClassObject());
  }
  return m_px->o_setPublicRef(propName, val, false);
}

Variant Object::o_setPublic(CStrRef propName, RefResult val) {
  return o_setPublicRef(propName, variant(val));
}

Variant &Object::o_lval(CStrRef propName, CVarRef tmpForGet,
                        CStrRef context /* = null_string */) {
  if (!m_px) {
    operator=(SystemLib::AllocStdClassObject());
  }
  return m_px->o_lval(propName, tmpForGet, context);
}

Variant &Object::o_unsetLval(CStrRef propName, CVarRef tmpForGet,
                             CStrRef context /* = null_string */) {
  if (!m_px) {
    return const_cast<Variant&>(tmpForGet);
  }
  return m_px->o_lval(propName, tmpForGet, context);
}

bool Object::o_isset(CStrRef propName,
                     CStrRef context /* = null_string */) const {
  if (!m_px) return false;
  return m_px->o_isset(propName, context);
}

bool Object::o_empty(CStrRef propName,
                     CStrRef context /* = null_string */) const {
  if (!m_px) return true;
  return m_px->o_empty(propName, context);
}

void Object::o_unset(CStrRef propName,
                     CStrRef context /* = null_string */) const {
  if (m_px) m_px->o_unset(propName, context);
}

Variant Object::o_argval(bool byRef, CStrRef propName,
    bool error /* = true */, CStrRef context /* = null_string */) {
  if (!byRef) {
    return o_get(propName, error, context);
  } else {
    return strongBind(o_lval(propName, context));
  }
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

Object Object::fiberMarshal(FiberReferenceMap &refMap) const {
  if (m_px) {
    return m_px->fiberMarshal(refMap);
  }
  return Object();
}

Object Object::fiberUnmarshal(FiberReferenceMap &refMap) const {
  if (m_px) {
    return m_px->fiberUnmarshal(refMap);
  }
  return Object();
}

///////////////////////////////////////////////////////////////////////////////
}
