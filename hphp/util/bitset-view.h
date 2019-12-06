/*
  +----------------------------------------------------------------------+
  | HipHop for PHP                                                       |
  +----------------------------------------------------------------------+
  | Copyright (c) 2010-present Facebook, Inc. (http://www.facebook.com)  |
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

#ifndef incl_HPHP_BITSET_VIEW_H_
#define incl_HPHP_BITSET_VIEW_H_

namespace HPHP {

/*
 * Bitset view
 *
 * This is a utility class to view a region of memory as a vector of bits.
 *
 * It is the user of this class's responsibility to ensure that the memory used
 * to construct the view is large enough to contain all the bits that get
 * addressed through operator[]--this class does not perform any allocation or
 * bounds checking.
 */
template <bool is_const>
struct BitsetView {
  using char_t = typename std::conditional<is_const,
                                           const unsigned char,
                                           unsigned char>::type;

  /*
   * A reference wrapper for a particular bit in the bitset--note that this is
   * larger than a typical pointer since it's a pointer to the byte and a mask
   * to select the particular bit of interest.
   */
  struct bit_reference {
    bit_reference(char_t* ptr, char_t mask)
      : m_ptr(ptr)
      , m_mask(mask) {}

    bit_reference(const bit_reference& o)
      : m_ptr(o.m_ptr)
      , m_mask(o.m_mask) {}

    bool operator=(bool b) {
      *m_ptr = (*m_ptr & ~m_mask) | (b ? m_mask : 0);
      return *this;
    }

    operator bool() const {
      return *m_ptr & m_mask;
    }

    bool operator!() const {
      return !(static_cast<bool>(*this));
    }

  private:
    char_t* m_ptr;
    char_t m_mask;
  };

  /*
   * A forward iterator for the bits in a view
   *
   * As with the view itself, no bounds-checking is performed--it's the callers
   * responsibility to make sure that every iterator that is dereferenced
   * points to valid memory
   */
  struct iterator {
    iterator(char_t* ptr, char_t mask)
      : m_ptr(ptr)
      , m_mask(mask) {}

    using value_type = bit_reference;
    using reference = bit_reference;
    using pointer = void;
    using difference_type = void;
    using iterator_category = std::forward_iterator_tag;

    bool operator==(const iterator& o) {
      return m_ptr == o.m_ptr && m_mask == o.m_mask;
    }

    bool operator!=(const iterator& o) {
      return !(*this == o);
    }

    bit_reference operator*() const {
      return bit_reference{m_ptr, m_mask};
    }

    bit_reference operator->() const {
      return *(*this);
    }

    iterator& operator++() {
      const auto carry = m_mask >> (sizeof(char_t) * 8 - 1);
      m_ptr += carry;
      m_mask = (m_mask << 1) | carry;
      return *this;
    }

    iterator operator++(int) {
      auto const ret = *this;
      ++(*this);
      return ret;
    }

  private:
    char_t* m_ptr;
    typename std::remove_const<char_t>::type m_mask;
  };

  /*
   * Construct a BitsetView starting at the given byte
   */
  explicit BitsetView(char_t* buf) : m_buf(buf) {}

  /*
   * Return the size (in bytes) for a bitset of the given length
   */
  static size_t sizeFor(size_t len) {
    return (len + CHAR_BIT - 1) / CHAR_BIT;
  }

  /*
   * Access the bit at the given index in the set
   */
  bit_reference operator[](size_t idx) {
    auto const chunk = idx / CHAR_BIT;
    auto const mask = static_cast<char_t>(1 << (idx % CHAR_BIT));
    return bit_reference{m_buf + chunk, mask};
  }

  iterator iteratorAt(size_t off) {
    return iterator{m_buf + off / CHAR_BIT,
                    static_cast<char_t>(1 << (off %  CHAR_BIT))};
  }

  /*
   * Access the memory being viewed as a bitset
   */
  char_t* buffer() { return m_buf; }
  const char_t* buffer() const { return m_buf; }

private:
  char_t* m_buf;
};

}



#endif // incl_HPHP_BITSET_VIEW_H_
