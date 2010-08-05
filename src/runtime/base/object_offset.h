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

#ifndef __HPHP_OBJECT_OFFSET_H__
#define __HPHP_OBJECT_OFFSET_H__

#include <runtime/base/types.h>
#include <runtime/base/complex_types.h>
#include <runtime/base/object_data.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

/**
 * An object's "offset" is actually its property and it's always string based.
 * This class should delegate all real work to ObjectData.
 */
class ObjectOffset {
 public:
  /**
   * Constructing an object offset object. "hash" is optional and can be -1
   * when property was not a literal string during code generation time.
   */
  ObjectOffset(ObjectData *data, CStrRef property, int64 hash,
               CStrRef context = null_string)
    : m_data(data), m_property(property), m_hash(hash), m_context(context) {
  }

  /**
   * Get r-value of this offset object.
   */
  operator Variant() const {
    if (m_data) {
      return m_data->o_get(m_property, m_hash, true, m_context);
    }
    return null;
  }

  /**
   * Get l-value of this offset object.
   */
  Variant &lval() const {
    if (!m_data) {
      throw NullPointerException();
    }
    return m_data->o_lval(m_property, m_hash, m_context);
  }

  /**
   * Unset an object's property.
   */
  void unset() const {
    if (!m_data) {
      throw NullPointerException();
    }
    m_data->t___unset(m_property);
  }

  /**
   * Assignement operator. Almost the whole purpose for having this offset
   * class.
   */
  CVarRef operator=(CVarRef v) {
    if (m_data) {
      m_data->o_set(m_property, m_hash, v, false, m_context);
    }
    return v;
  }

  /**
   * Operator overloading. rvalAt() and lvalAt() could be type-specialized,
   * but these are rare calls, I think.
   */
  Variant &operator>>=(CVarRef v) {
    Variant &lv = lval();
    if (m_data) {
      m_data->o_set(m_property, m_hash, lv.toInt64() >> v.toInt64(), false,
                    m_context);
    }
    return lv;
  }
  Variant &operator<<=(CVarRef v) {
    Variant &lv = lval();
    if (m_data) {
      m_data->o_set(m_property, m_hash, lv.toInt64() << v.toInt64(), false,
                    m_context);
    }
    return lv;
  }
  Variant &operator^=(CVarRef v) {
    Variant &lv = lval();
    if (m_data) {
      m_data->o_set(m_property, m_hash, lv ^ v, false, m_context);
    }
    return lv;
  }
  Variant &operator|=(CVarRef v) {
    Variant &lv = lval();
    if (m_data) {
      m_data->o_set(m_property, m_hash, lv | v, false, m_context);
    }
    return lv;
  }
  Variant &operator&=(CVarRef v) {
    Variant &lv = lval();
    if (m_data) {
      m_data->o_set(m_property, m_hash, lv & v, false, m_context);
    }
    return lv;
  }
  Variant &operator+=(CVarRef v) {
    Variant &lv = lval();
    if (m_data) {
      m_data->o_set(m_property, m_hash, lv + v, false, m_context);
    }
    return lv;
  }
  Variant &operator-=(CVarRef v) {
    Variant &lv = lval();
    if (m_data) {
      m_data->o_set(m_property, m_hash, lv - v, false, m_context);
    }
    return lv;
  }
  Variant &operator*=(CVarRef v) {
    Variant &lv = lval();
    if (m_data) {
      m_data->o_set(m_property, m_hash, lv * v, false, m_context);
    }
    return lv;
  }
  Variant &operator/=(CVarRef v) {
    Variant &lv = lval();
    if (m_data) {
      m_data->o_set(m_property, m_hash, lv / v, false, m_context);
    }
    return lv;
  }
  Variant &operator%=(CVarRef v) {
    Variant &lv = lval();
    if (m_data) {
      m_data->o_set(m_property, m_hash, lv % v, false, m_context);
    }
    return lv;
  }

  Variant rvalAt(const Primitive &key, int64 hash = -1) {
    return operator Variant().rvalAt(key, hash);
  }
  Variant &lvalAt(const Primitive &key, int64 hash = -1) {
    return lval().lvalAt(key, hash);
  }

 private:
  ObjectData *m_data;
  String m_property;
  int64 m_hash;
  CStrRef m_context;
};

///////////////////////////////////////////////////////////////////////////////
}

#endif // __HPHP_OBJECT_OFFSET_H__
