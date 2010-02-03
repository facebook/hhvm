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

#ifndef __HPHP_HPHP_VECTOR_H__
#define __HPHP_HPHP_VECTOR_H__

#include <cpp/base/types.h>
#include <cpp/base/type_string.h>
#include <cpp/base/util/hphp_map_cell.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

namespace HphpVectorFuncs {

// HphpVector<int64>
inline void allocate(int64 *data, int count) {}
inline void deallocate(int64 *data, int count) {}
inline void reset(int64 *data, int count) {}
inline void sweep(int64 *data, int count) {}
inline void copy(int64 *dest, int64 *src, int count) {
  memcpy(dest, src, count * sizeof(int64));
}

// HphpVector<String>
inline void allocate(String *data, int count) {}
inline void deallocate(String *data, int count) {
  for (int i = 0; i < count; i++) {
    data[i].~String();
  }
}
inline void reset(String *data, int count) {
  memset(data, 0, count * sizeof(String));
}
inline void sweep(String *data, int count) {}
inline void copy(String *dest, String *src, int count) {
  for (int i = 0; i < count; i++) {
    dest[i] = src[i];
  }
}

// HphpVector<T*>
template<typename T> inline void allocate(T **data, int count) {}
template<typename T> inline void deallocate(T **data, int count) {}
template<typename T> inline void reset(T **data, int count) {}
template<typename T> inline void sweep(T **data, int count) {}
template<typename T> inline void copy(T **dest, T **src, int count) {
  memcpy(dest, src, count * sizeof(T*));
}

}

///////////////////////////////////////////////////////////////////////////////

/**
 * A vector that's using LinearMemoryAllocator. This also makes the following
 * assumptions:
 *
 *  1. T is movable within the vector without hurting anything.
 *  2. new T has all bytes 0
 *  3. T doesn't have vtable
 */
template<typename T>
class HphpVector {
public:
  HphpVector(int count = 0)
    : m_data(NULL), m_size(0), m_count(count), m_bytes(0) {
    ASSERT(count >= 0);
    if (count) {
      reserve(count);
      HphpVectorFuncs::allocate((T*)m_data, m_count);
    } else {
      m_size = 4 * sizeof(T);
      m_data = (char*)calloc(m_size, 1);
    }
    m_bytes = m_count * sizeof(T);
  }

  HphpVector(const HphpVector<T> &src)
    : m_data(NULL), m_size(0), m_count(0), m_bytes(0) {
    append(src);
  }

  ~HphpVector() {
    if (m_data) {
      HphpVectorFuncs::deallocate((T*)m_data, m_count);
      free(m_data);
    }
  }

  void reserve(int count) {
    ASSERT(count >= 0);
    int newsize = count * sizeof(T);
    if (newsize > m_size) {
      int oldsize = m_size;
      for (m_size = 4; m_size < newsize; m_size <<= 1);
      if (m_data) {
        m_data = (char*)realloc(m_data, m_size);
        memset(m_data + oldsize, 0, m_size - oldsize);
      } else {
        m_data = (char*)calloc(m_size, 1);
      }
    }
  }
  void clear() {
    HphpVectorFuncs::deallocate((T*)m_data, m_count);
    m_count = 0;
    m_bytes = 0;
  }
  void resize(int count) {
    ASSERT(count >= 0);
    if (count > m_count) {
      reserve(count);
      HphpVectorFuncs::allocate((T*)(m_data + m_bytes), count - m_count);
      m_count = count;
      m_bytes = m_count * sizeof(T);
    } else if (count < m_count) {
      m_bytes = count * sizeof(T);
      HphpVectorFuncs::deallocate((T*)(m_data + m_bytes), m_count - count);
      m_count = count;
    }
  }

  unsigned int size() const {
    return m_count;
  }
  bool empty() const {
    return m_count == 0;
  }

  const T &operator[](int index) const {
    ASSERT(index >= 0);
    ASSERT(index < m_count);
    return *(const T*)(m_data + index * sizeof(T));
  }
  T &operator[](int index) {
    ASSERT(index >= 0);
    ASSERT(index < m_count);
    return *(T*)(m_data + index * sizeof(T));
  }
  const T &back() const {
    ASSERT(m_count);
    return *(const T*)(m_data + m_bytes - sizeof(T));
  }
  T &back() {
    ASSERT(m_count);
    return *(T*)(m_data + m_bytes - sizeof(T));
  }

  void push_back(const T &value) {
    reserve(++m_count);
    char *item = m_data + m_bytes;
    HphpVectorFuncs::reset((T*)item, 1);
    HphpVectorFuncs::allocate((T*)item, 1);
    *(T*)item = value;
    m_bytes += sizeof(T);
  }
  void insert(int index, const T &value) {
    ASSERT(index >= 0);
    ASSERT(index <= m_count);
    reserve(++m_count);
    int offset = index * sizeof(T);
    char *item = m_data + offset;
    if (m_bytes > offset) {
      memmove(item + sizeof(T), item, m_bytes - offset);
    }
    HphpVectorFuncs::reset((T*)item, 1);
    HphpVectorFuncs::allocate((T*)item, 1);
    *(T*)item = value;
    m_bytes += sizeof(T);
  }
  void remove(int index) {
    ASSERT(index >= 0);
    ASSERT(index < m_count);
    int offset = index * sizeof(T);
    char *item = m_data + offset;
    HphpVectorFuncs::deallocate((T*)item, 1);
    --m_count;
    m_bytes -= sizeof(T);
    if (m_bytes > offset) {
      memmove(item, item + sizeof(T), m_bytes - offset);
    }
  }
  void append(const HphpVector<T> &src, int start = 0, int count = -1) {
    ASSERT(&src != this);
    int newcount = src.size() - start;
    if (newcount <= 0) return;
    if (count >= 0 && newcount > count) {
      newcount = count;
      if (newcount == 0) return;
    }

    reserve(m_count += newcount);
    char *item = m_data + m_bytes;
    HphpVectorFuncs::reset((T*)item, newcount);
    HphpVectorFuncs::copy((T*)item, (T*)(src.m_data + start * sizeof(T)),
                          newcount);
    m_bytes = m_count * sizeof(T);
  }

  HphpVector &operator=(const HphpVector<T> &src) {
    clear();
    append(src);
    return *this;
  }

  void swap(HphpVector<T> &src) {
    HphpVector<T> tmp(0,0);
    memcpy(&tmp, &src, sizeof(HphpVector<T>));
    memcpy(&src, this, sizeof(HphpVector<T>));
    memcpy(this, &tmp, sizeof(HphpVector<T>));
    tmp.m_data = NULL;
  }

  /**
   * Memory allocator methods.
   */
  bool calculate(int &size) const {
    ASSERT(m_bytes == (int)(m_count * sizeof(T)));
    size += sizeof(int);
    size += m_bytes;
    return true;
  }
  void backup(LinearAllocator &allocator) const {
    allocator.backup(m_bytes);
    allocator.backup(m_data, m_bytes);
  }
  void restore(const char *&data) {
    int s = *(int*)data;
    data += sizeof(int);
    m_data = (char*)malloc(m_size);
    memcpy(m_data, data, s);
    data += s;
  }
  void sweep() {
    HphpVectorFuncs::sweep((T*)m_data, m_count);
    free(m_data);
    m_data = NULL;
  }

private:
  HphpVector(int, int) : m_data(NULL) {} // purely for swap

  char *m_data;  // malloc-ed memory
  int m_size;    // malloc-ed size
  int m_count;   // item count
  int m_bytes;   // always equal to m_count * sizeof(T)
};

///////////////////////////////////////////////////////////////////////////////
}

#endif // __HPHP_HPHP_VECTOR_H__
