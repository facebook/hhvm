#ifndef __ARRAY_INIT_H__
#define __ARRAY_INIT_H__

#include <cpp/base/array/array_element.h>
#include <vector>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////
// ArrayInit

/**
 * When a ZendArray is created, ArrayInit completely skips the use of
 * ArrayElement, (the four set methods mimic the four constructors of
 * ArrayElement).
 */
class ArrayInit {
public:
  ArrayInit(int n);

  template<typename T>
  ArrayInit &set(int p, const T &value) {
    if (m_data) {
      Variant v(value);
      m_data->append(v, false);
    } else {
      (*m_elements)[p] = NEW(ArrayElement)(value);
    }
    return *this;
  }

  ArrayInit &set(int p, CVarRef v) {
    if (m_data) {
      m_data->append(v, false);
    } else {
      (*m_elements)[p] = NEW(ArrayElement)(v);
    }
    return *this;
  }

  template<typename T>
  ArrayInit &set(int p, CVarRef name, const T &value, int64 prehash = -1) {
    if (m_data) {
      Variant v(value);
      m_data->set(name, v, false, prehash);
    } else {
      (*m_elements)[p] = NEW(ArrayElement)(value);
    }
    return *this;
  }

  ArrayInit &set(int p, CVarRef name, CVarRef v, int64 prehash = -1) {
    if (m_data) {
      m_data->set(name, v, false, prehash);
    } else {
      (*m_elements)[p] = NEW(ArrayElement)(v);
    }
    return *this;
  }

  ArrayData *create() {
    if (m_data) return m_data;
    ArrayData *ret = ArrayData::Create(m_elements, true);
    delete m_elements;
    return ret;
  }

private:
  ArrayElementVec *m_elements; // set when RuntimeOption::UseZendArray is false
  ArrayData *m_data;           // set when RuntimeOption::UseZendArray is true
};

///////////////////////////////////////////////////////////////////////////////
}

#endif /* __ARRAY_INIT_H__ */
