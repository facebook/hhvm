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
#ifndef __ARRAY_INIT_H__
#define __ARRAY_INIT_H__

#include <runtime/base/array/array_data.h>
#include <runtime/base/complex_types.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////
// macros for creating vectors or maps

#define CREATE_VECTOR1(e) Array(ArrayInit(1, true).set(e).create())
#define CREATE_VECTOR2(e1, e2)                                          \
  Array(ArrayInit(2, true).set(e1).set(e2).create())
#define CREATE_VECTOR3(e1, e2, e3)                                      \
  Array(ArrayInit(3, true).set(e1).set(e2).set(e3).create())
#define CREATE_VECTOR4(e1, e2, e3, e4)                                  \
  Array(ArrayInit(4, true).set(e1).set(e2).set(e3).set(e4).create())
#define CREATE_VECTOR5(e1, e2, e3, e4, e5)                              \
  Array(ArrayInit(5, true).set(e1).set(e2).set(e3).set(e4).set(e5).create())
#define CREATE_VECTOR6(e1, e2, e3, e4, e5, e6)                          \
  Array(ArrayInit(6, true).set(e1).set(e2).set(e3).set(e4).set(e5).     \
                           set(e6).create())

#define CREATE_MAP1(n, e) Array(ArrayInit(1, false).set(n, e).create())
#define CREATE_MAP2(n1, e1, n2, e2)                                       \
  Array(ArrayInit(2, false).set(n1, e1).set(n2, e2).create())
#define CREATE_MAP3(n1, e1, n2, e2, n3, e3)                               \
  Array(ArrayInit(3, false).set(n1, e1).set(n2, e2).set(n3, e3).create())
#define CREATE_MAP4(n1, e1, n2, e2, n3, e3, n4, e4)                       \
  Array(ArrayInit(4, false).set(n1, e1).set(n2, e2).set(n3, e3).          \
                            set(n4, e4).create())
#define CREATE_MAP5(n1, e1, n2, e2, n3, e3, n4, e4, n5, e5)               \
  Array(ArrayInit(5, false).set(n1, e1).set(n2, e2).set(n3, e3).          \
                            set(n4, e4).set(n5, e5).create())
#define CREATE_MAP6(n1, e1, n2, e2, n3, e3, n4, e4, n5, e5, n6, e6)       \
  Array(ArrayInit(6, false).set(n1, e1).set(n2, e2).set(n3, e3).          \
                            set(n4, e4).set(n5, e5).set(n6, e6).create())

///////////////////////////////////////////////////////////////////////////////
// ArrayInit

/**
 * When an Array is created, ArrayInit completely skips the use of
 * ArrayElement, (the set methods mimic the constructors of ArrayElement).
 * The setRef method handles the case where the value needs to be a reference.
 *
 * For arrays that need to have C++ references/pointers to their elements for
 * an extended period of time, set keepRef to true, so that there will not
 * be reference-breaking escalation.
 */
class ArrayInit {
public:
  ArrayInit(ssize_t n, bool isVector = false, bool keepRef = false);
  ~ArrayInit() {
    // In case an exception interrupts the initialization.
    if (m_data) m_data->release();
  }

  ArrayInit &set(CVarRef v) {
    m_data->append(v, false);
    return *this;
  }

  ArrayInit &set(RefResult v) {
    setRef(variant(v));
    return *this;
  }

  ArrayInit &set(CVarWithRefBind v) {
    m_data->appendWithRef(variant(v), false);
    return *this;
  }

  ArrayInit &setRef(CVarRef v) {
    m_data->appendRef(v, false);
    return *this;
  }

  ArrayInit &set(int64 name, CVarRef v, bool keyConverted = false) {
    m_data->set(name, v, false);
    return *this;
  }

  ArrayInit &set(litstr name, CVarRef v, bool keyConverted = false) {
    if (keyConverted) {
      m_data->set(name, v, false);
    } else {
      m_data->set(String(name).toKey(), v, false);
    }
    return *this;
  }

  ArrayInit &set(CStrRef name, CVarRef v, bool keyConverted = false) {
    if (keyConverted) {
      m_data->set(name, v, false);
    } else if (!name.isNull()) {
      m_data->set(name.toKey(), v, false);
    }
    return *this;
  }

  ArrayInit &set(CVarRef name, CVarRef v, bool keyConverted = false) {
    if (keyConverted) {
      m_data->set(name, v, false);
    } else {
      VarNR k(name.toKey());
      if (!k.isNull()) {
        m_data->set(k, v, false);
      }
    }
    return *this;
  }

  template<typename T>
  ArrayInit &set(const T &name, CVarRef v, bool keyConverted = false) {
    if (keyConverted) {
      m_data->set(name, v, false);
    } else {
      VarNR k(Variant(name).toKey());
      if (!k.isNull()) {
        m_data->set(k, v, false);
      }
    }
    return *this;
  }

  ArrayInit &set(litstr name, RefResult v, bool keyConverted = false) {
    if (keyConverted) {
      m_data->setRef(name, variant(v), false);
    } else {
      m_data->setRef(String(name).toKey(), variant(v), false);
    }
    return *this;
  }

  ArrayInit &set(CStrRef name, RefResult v, bool keyConverted = false) {
    if (keyConverted) {
      m_data->setRef(name, variant(v), false);
    } else if (!name.isNull()) {
      m_data->setRef(name.toKey(), variant(v), false);
    }
    return *this;
  }

  ArrayInit &set(CVarRef name, RefResult v, bool keyConverted = false) {
    if (keyConverted) {
      m_data->setRef(name, variant(v), false);
    } else {
      VarNR k(name.toKey());
      if (!k.isNull()) {
        m_data->setRef(k, variant(v), false);
      }
    }
    return *this;
  }

  template<typename T>
  ArrayInit &set(const T &name, RefResult v, bool keyConverted = false) {
    if (keyConverted) {
      m_data->setRef(name, variant(v), false);
    } else {
      VarNR k(Variant(name).toKey());
      if (!k.isNull()) {
        m_data->setRef(k, variant(v), false);
      }
    }
    return *this;
  }

  ArrayInit &add(int64 name, CVarRef v, bool keyConverted = false) {
    m_data->add(name, v, false);
    return *this;
  }

  ArrayInit &add(litstr name, CVarRef v, bool keyConverted = false) {
    return add(String(name), v, keyConverted);
  }

  ArrayInit &add(CStrRef name, CVarRef v, bool keyConverted = false) {
    if (keyConverted) {
      m_data->add(name, v, false);
    } else if (!name.isNull()) {
      m_data->add(name.toKey(), v, false);
    }
    return *this;
  }

  ArrayInit &add(CVarRef name, CVarRef v, bool keyConverted = false) {
    if (keyConverted) {
      m_data->add(name, v, false);
    } else {
      VarNR k(name.toKey());
      if (!k.isNull()) {
        m_data->add(k, v, false);
      }
    }
    return *this;
  }

  template<typename T>
  ArrayInit &add(const T &name, CVarRef v, bool keyConverted = false) {
    if (keyConverted) {
      m_data->add(name, v, false);
    } else {
      VarNR k(Variant(name).toKey());
      if (!k.isNull()) {
        m_data->add(k, v, false);
      }
    }
    return *this;
  }

  ArrayInit &setRef(int64 name, CVarRef v, bool keyConverted = false) {
    m_data->setRef(name, v, false);
    return *this;
  }

  ArrayInit &setRef(litstr name, CVarRef v, bool keyConverted = false) {
    if (keyConverted) {
      m_data->setRef(name, v, false);
    } else {
      m_data->setRef(String(name).toKey(), v, false);
    }
    return *this;
  }

  ArrayInit &setRef(CStrRef name, CVarRef v, bool keyConverted = false) {
    if (keyConverted) {
      m_data->setRef(name, v, false);
    } else {
      m_data->setRef(name.toKey(), v, false);
    }
    return *this;
  }

  ArrayInit &setRef(CVarRef name, CVarRef v, bool keyConverted = false) {
    if (keyConverted) {
      m_data->setRef(name, v, false);
    } else {
      Variant key(name.toKey());
      if (!key.isNull()) {
        m_data->setRef(key, v, false);
      }
    }
    return *this;
  }

  template<typename T>
  ArrayInit &setRef(const T &name, CVarRef v, bool keyConverted = false) {
    if (keyConverted) {
      m_data->setRef(name, v, false);
    } else {
      VarNR key(Variant(name).toKey());
      if (!key.isNull()) {
        m_data->setRef(key, v, false);
      }
    }
    return *this;
  }

  ArrayData *create() {
    ArrayData *ret = m_data;
    m_data = NULL;
    return ret;
  }
  operator ArrayData *() { return create(); }
  // this consructor should never be called directly, it is only called from
  // generated code.
  ArrayInit (ArrayData *data) {  m_data = data;}
private:
  ArrayData *m_data;
};

///////////////////////////////////////////////////////////////////////////////
}

#endif /* __ARRAY_INIT_H__ */
