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

#include <runtime/base/complex_types.h>
#include <runtime/base/object_offset.h>
#include <runtime/base/builtin_functions.h>
#include <runtime/base/variable_serializer.h>
#include <system/gen/php/classes/stdclass.h>

namespace HPHP {

const Object Object::s_nullObject = Object();

///////////////////////////////////////////////////////////////////////////////

Object &Object::operator=(CVarRef var) {
  return operator=(var.toObject());
}

Array Object::toArray() const {
  return m_data.pobj ? m_data.pobj->o_toArray() : Array();
}

Variant Object::toKey() const {
  if (m_data.pobj) {
    if (isResource()) {
      return m_data.pobj->o_toInt64();
    } else {
      return m_data.pobj->t___tostring();
    }
  } else {
    return null_variant;
  }
}

bool Object::equal(CObjRef v2) const {
  if (m_data.pobj == v2.m_data.pobj)
    return true;
  if (!m_data.pobj || !v2.m_data.pobj)
    return false;
  if (isResource() || v2.isResource())
    return false;
  return (v2.get()->o_isClass(m_data.pobj->o_getClassName()) &&
          toArray().equal(v2.toArray()));
}

bool Object::less(CObjRef v2) const { return toArray().less(v2.toArray());}
bool Object::more(CObjRef v2) const { return toArray().more(v2.toArray());}

Variant Object::o_get(CStrRef propName, int64 hash /* = -1 */,
    bool error /* = true */) const {
  if (!m_data.pobj) throw NullPointerException();
  return m_data.pobj->o_get(propName, hash, error);
}

ObjectOffset Object::o_lval(CStrRef propName, int64 hash /* = -1 */) {
  if (!m_data.pobj) {
    operator=(NEW(c_stdclass)());
  }
  return ObjectOffset(m_data.pobj, propName, hash);
}

///////////////////////////////////////////////////////////////////////////////
// output

void Object::serialize(VariableSerializer *serializer) const {
  if (m_data.pobj) {
    m_data.pobj->serialize(serializer);
  } else {
    serializer->writeNull();
  }
}

bool Object::unserialize(std::istream &in) {
  throw NotImplementedException(__func__);
}

///////////////////////////////////////////////////////////////////////////////
}
