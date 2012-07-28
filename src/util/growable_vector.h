#ifndef _GROWABLE_VECTOR_H_
#define _GROWABLE_VECTOR_H_

namespace HPHP {

/*
 * GrowableVector --
 *
 * We make a large number of these, and they typically only have a small
 * number of entries.
 * It's a shame to use a 24-byte std::vector for this.
 */
template<typename T>
struct GrowableVector {
  uint32_t m_size;
  T *m_data; // Actually variable length
  GrowableVector() : m_size(0) { m_data = (T*)malloc(sizeof(T)); }
  
  typedef GrowableVector<T> Self;
  Self& operator=(const Self& rhs) {
    m_size = rhs.m_size;
    m_data = (T*)malloc(sizeof(T) * Util::roundUpToPowerOfTwo(m_size + 1));
    memcpy(m_data, rhs.m_data, m_size * sizeof(T));
    return *this;
  }

  // Copy ctor
  GrowableVector(const GrowableVector& rhs) {
    *this = rhs;
  }

  ~GrowableVector() {
    free(m_data);
  }

  T& operator[](const size_t idx) {
    ASSERT(idx < m_size);
    return m_data[idx];
  }
  const T& operator[](const size_t idx) const {
    ASSERT(idx < m_size);
    return m_data[idx];
  }

  void push_back(const T& datum) {
    // m_data always has room for at least one element due to the m_data[1]
    // declaration, so the realloc() code first has to kick in when a second
    // element is about to be pushed.
    if (Util::isPowerOfTwo(m_size)) {
      m_data = (T*)realloc(m_data, 2 * m_size * sizeof(T));
    }
    m_data[m_size++] = datum;
  }

  typedef T* iterator;
  typedef T* const_iterator;
  iterator begin() { return &m_data[0]; }
  iterator end()   { return &m_data[m_size]; }
  const_iterator begin() const { return &m_data[0]; }
  const_iterator end()   const { return &m_data[m_size]; }
  size_t size() const { return m_size; }
  bool empty() const  { return size() == 0; }
};

}

#endif
