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

#include <cpp/base/type_object.h>
#include <cpp/base/object_offset.h>
#include <cpp/base/builtin_functions.h>
#include <cpp/base/variable_serializer.h>

namespace HPHP {

const Object Object::s_nullObject = Object();

///////////////////////////////////////////////////////////////////////////////

Variant Object::o_get(CStrRef propName, int64 hash /* = -1 */) const {
  if (!m_px) throw NullPointerException();
  return m_px->o_get(propName, hash);
}

ObjectOffset Object::o_lval(CStrRef propName, int64 hash /* = -1 */) {
  if (!m_px) throw NullPointerException();
  return ObjectOffset(m_px, propName, hash);
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

///////////////////////////////////////////////////////////////////////////////
}
