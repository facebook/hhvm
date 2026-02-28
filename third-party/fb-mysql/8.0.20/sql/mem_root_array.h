/* Copyright (c) 2011, 2019, Oracle and/or its affiliates. All rights reserved.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License, version 2.0,
   as published by the Free Software Foundation.

   This program is also distributed with certain software (including
   but not limited to OpenSSL) that is licensed under separate terms,
   as designated in a particular file or component or in included license
   documentation.  The authors of MySQL hereby grant you an additional
   permission to link the program and your derivative works with the
   separately licensed software that they have included with MySQL.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License, version 2.0, for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA */

#ifndef MEM_ROOT_ARRAY_INCLUDED
#define MEM_ROOT_ARRAY_INCLUDED

#include <algorithm>
#include <type_traits>
#include <utility>

#include "my_alloc.h"
#include "my_dbug.h"

/**
   A typesafe replacement for DYNAMIC_ARRAY.
   We use MEM_ROOT for allocating storage, rather than the C++ heap.
   The interface is chosen to be similar to std::vector.

   @remark
   Mem_root_array_YY is constructor-less for use in the parser stack of unions.
   For other needs please use Mem_root_array.

   @remark
   Unlike DYNAMIC_ARRAY, elements are properly copied
   (rather than memcpy()d) if the underlying array needs to be expanded.

   @remark
   Unless Element_type's destructor is trivial, we destroy objects when they are
   removed from the array (including when the array object itself is destroyed).

   @remark
   Note that MEM_ROOT has no facility for reusing free space,
   so don't use this if multiple re-expansions are likely to happen.

   @tparam Element_type The type of the elements of the container.
           Elements must be copyable.
*/
template <typename Element_type>
class Mem_root_array_YY {
  /**
     Is Element_type trivially destructible? If it is, we don't destroy
     elements when they are removed from the array or when the array is
     destroyed.
  */
  static constexpr bool has_trivial_destructor =
      std::is_trivially_destructible<Element_type>::value;

 public:
  /// Convenience typedef, same typedef name as std::vector
  typedef Element_type value_type;

  void init(MEM_ROOT *root) {
    DBUG_ASSERT(root != nullptr);

    m_root = root;
    m_array = nullptr;
    m_size = 0;
    m_capacity = 0;
  }

  /// Initialize empty array that we aren't going to grow
  void init_empty_const() {
    m_root = nullptr;
    m_array = nullptr;
    m_size = 0;
    m_capacity = 0;
  }

  Element_type &at(size_t n) {
    DBUG_ASSERT(n < size());
    return m_array[n];
  }

  const Element_type &at(size_t n) const {
    DBUG_ASSERT(n < size());
    return m_array[n];
  }

  Element_type &operator[](size_t n) { return at(n); }
  const Element_type &operator[](size_t n) const { return at(n); }

  Element_type &back() { return at(size() - 1); }
  const Element_type &back() const { return at(size() - 1); }

  /// Random access iterators to value_type and const value_type.
  typedef Element_type *iterator;
  typedef const Element_type *const_iterator;

  /// Returns a pointer to the first element in the array.
  Element_type *begin() { return &m_array[0]; }
  const Element_type *begin() const { return &m_array[0]; }

  /// Returns a pointer to the past-the-end element in the array.
  Element_type *end() { return &m_array[size()]; }
  const Element_type *end() const { return &m_array[size()]; }

  /// Returns a constant pointer to the first element in the array.
  const_iterator cbegin() const { return begin(); }

  /// Returns a constant pointer to the past-the-end element in the array.
  const_iterator cend() const { return end(); }

  /// Erases all of the elements.
  void clear() {
    if (!empty()) chop(0);
  }

  /**
    Chops the tail off the array, erasing all tail elements.
    @param pos Index of first element to erase.
  */
  void chop(const size_t pos) {
    DBUG_ASSERT(pos < m_size);
    if (!has_trivial_destructor) {
      for (size_t ix = pos; ix < m_size; ++ix) {
        Element_type *p = &m_array[ix];
        p->~Element_type();  // Destroy discarded element.
      }
    }
    m_size = pos;
  }

  /**
    Reserves space for array elements.
    Copies over existing elements, in case we are re-expanding the array.

    @param  n number of elements.
    @retval true if out-of-memory, false otherwise.
  */
  bool reserve(size_t n) {
    if (n <= m_capacity) return false;

    void *mem = m_root->Alloc(n * element_size());
    if (!mem) return true;
    Element_type *array = static_cast<Element_type *>(mem);

    // Copy all the existing elements into the new array.
    for (size_t ix = 0; ix < m_size; ++ix) {
      Element_type *new_p = &array[ix];
      Element_type *old_p = &m_array[ix];
      ::new (new_p)
          Element_type(std::move(*old_p));  // Copy or move into new location.
      if (!has_trivial_destructor)
        old_p->~Element_type();  // Destroy the old element.
    }

    // Forget the old array.
    m_array = array;
    m_capacity = n;
    return false;
  }

  /**
    Adds a new element at the end of the array, after its current last
    element. The content of this new element is initialized to a copy of
    the input argument.

    @param  element Object to copy.
    @retval true if out-of-memory, false otherwise.
  */
  bool push_back(const Element_type &element) {
    const size_t min_capacity = 20;
    const size_t expansion_factor = 2;
    if (0 == m_capacity && reserve(min_capacity)) return true;
    if (m_size == m_capacity && reserve(m_capacity * expansion_factor))
      return true;
    Element_type *p = &m_array[m_size++];
    ::new (p) Element_type(element);
    return false;
  }

  /**
    Adds a new element at the end of the array, after its current last
    element. The content of this new element is initialized by moving
    the input element.

    @param  element Object to move.
    @retval true if out-of-memory, false otherwise.
  */
  bool push_back(Element_type &&element) {
    const size_t min_capacity = 20;
    const size_t expansion_factor = 2;
    if (0 == m_capacity && reserve(min_capacity)) return true;
    if (m_size == m_capacity && reserve(m_capacity * expansion_factor))
      return true;
    Element_type *p = &m_array[m_size++];
    ::new (p) Element_type(std::move(element));
    return false;
  }

  /**
    Removes the last element in the array, effectively reducing the
    container size by one. This destroys the removed element.
   */
  void pop_back() {
    DBUG_ASSERT(!empty());
    if (!has_trivial_destructor) back().~Element_type();
    m_size -= 1;
  }

  /**
    Resizes the container so that it contains n elements.

    If n is smaller than the current container size, the content is
    reduced to its first n elements, removing those beyond (and
    destroying them).

    If n is greater than the current container size, the content is
    expanded by inserting at the end as many elements as needed to
    reach a size of n. If val is specified, the new elements are
    initialized as copies of val, otherwise, they are
    value-initialized.

    If n is also greater than the current container capacity, an automatic
    reallocation of the allocated storage space takes place.

    Notice that this function changes the actual content of the
    container by inserting or erasing elements from it.
   */
  void resize(size_t n, const value_type &val) {
    if (n == m_size) return;
    if (n > m_size) {
      if (!reserve(n)) {
        while (n != m_size) push_back(val);
      }
      return;
    }
    if (!has_trivial_destructor) {
      while (n != m_size) pop_back();
    }
    m_size = n;
  }

  /**
    Same as resize(size_t, const value_type &val), but default-constructs
    the new elements. This allows one to resize containers even if
    value_type is not copy-constructible.
   */
  void resize(size_t n) {
    if (n == m_size) return;
    if (n > m_size) {
      if (!reserve(n)) {
        while (n != m_size) push_back(value_type());
      }
      return;
    }
    if (!has_trivial_destructor) {
      while (n != m_size) pop_back();
    }
    m_size = n;
  }

  /**
    Erase all the elements in the specified range.

    @param first  iterator that points to the first element to remove
    @param last   iterator that points to the element after the
                  last one to remove
    @return an iterator to the first element after the removed range
  */
  iterator erase(const_iterator first, const_iterator last) {
    iterator pos = begin() + (first - cbegin());
    if (first != last) {
      iterator new_end = std::move(last, cend(), pos);
      chop(new_end - begin());
    }
    return pos;
  }

  /**
    Removes a single element from the array.

    @param position  iterator that points to the element to remove

    @return an iterator to the first element after the removed range
  */
  iterator erase(const_iterator position) {
    return erase(position, std::next(position));
  }

  /**
    Removes a single element from the array.

    @param ix  zero-based number of the element to remove

    @return an iterator to the first element after the removed range
  */
  iterator erase(size_t ix) {
    DBUG_ASSERT(ix < size());
    return erase(std::next(this->cbegin(), ix));
  }

  /**
    Insert an element at a given position.

    @param pos    the new element is inserted before the element
                  at this position
    @param value  the value of the new element
    @return an iterator that points to the inserted element
  */
  iterator insert(const_iterator pos, const Element_type &value) {
    ptrdiff_t idx = pos - cbegin();
    if (!push_back(value)) std::rotate(begin() + idx, end() - 1, end());
    return begin() + idx;
  }

  /**
    Removes a single element from the array by value.
    The removed element is destroyed.  This effectively reduces the
    container size by one.
    Note that if there are multiple elements having the same
    value, only the first element is removed.

    This is generally an inefficient operation, since we need to copy
    elements to fill the "hole" in the array.

    We use std::copy to move objects, hence Element_type must be
    assignable.

    @retval number of elements removed, 0 or 1.
  */
  size_t erase_value(const value_type &val) {
    iterator position = std::find(begin(), end(), val);
    if (position != end()) {
      erase(position);
      return 1;
    }
    return 0;  // Not found
  }

  /**
    Removes a single element from the array.
    The removed element is destroyed.
    This effectively reduces the container size by one.

    This is generally an inefficient operation, since we need to copy
    elements to fill the "hole" in the array.

    We use std::copy to move objects, hence Element_type must be assignable.
  */
  iterator erase(iterator position) {
    DBUG_ASSERT(position != end());
    if (position + 1 != end()) std::copy(position + 1, end(), position);
    this->pop_back();
    return position;
  }

  size_t capacity() const { return m_capacity; }
  size_t element_size() const { return sizeof(Element_type); }
  bool empty() const { return size() == 0; }
  size_t size() const { return m_size; }

 protected:
  MEM_ROOT *m_root;
  Element_type *m_array;
  size_t m_size;
  size_t m_capacity;

  // No CTOR/DTOR for this class!
  // Mem_root_array_YY(const Mem_root_array_YY&);
  // Mem_root_array_YY &operator=(const Mem_root_array_YY&);
};

/**
  A typesafe replacement for DYNAMIC_ARRAY.

  @see Mem_root_array_YY.
*/
template <typename Element_type>
class Mem_root_array : public Mem_root_array_YY<Element_type> {
  typedef Mem_root_array_YY<Element_type> super;

 public:
  /// Convenience typedef, same typedef name as std::vector
  typedef Element_type value_type;

  typedef typename super::const_iterator const_iterator;

  Mem_root_array() { super::init_empty_const(); }

  explicit Mem_root_array(MEM_ROOT *root) { super::init(root); }

  /// Move constructor and assignment.
  Mem_root_array(Mem_root_array &&other) {
    this->m_root = other.m_root;
    this->m_array = other.m_array;
    this->m_size = other.m_size;
    this->m_capacity = other.m_capacity;
    other.init_empty_const();
  }
  Mem_root_array &operator=(Mem_root_array &&other) {
    if (this != &other) {
      this->~Mem_root_array();
      new (this) Mem_root_array(std::move(other));
    }
    return *this;
  }

  Mem_root_array(MEM_ROOT *root, size_t n) {
    super::init(root);
    super::resize(n);
  }

  Mem_root_array(MEM_ROOT *root, size_t n, const value_type &val) {
    super::init(root);
    super::resize(n, val);
  }

  /**
    Range constructor.

    Constructs a container with as many elements as the range [first,last),
    with each element constructed from its corresponding element in that range,
    in the same order.

    @param root   MEM_ROOT to use for memory allocation.
    @param first  iterator that points to the first element to copy
    @param last   iterator that points to the element after the
                  last one to copy
  */
  Mem_root_array(MEM_ROOT *root, const_iterator first, const_iterator last) {
    super::init(root);
    if (this->reserve(last - first)) return;
    for (auto it = first; it != last; ++it) this->push_back(*it);
  }

  Mem_root_array(MEM_ROOT *root, const Mem_root_array &x)
      : Mem_root_array(root, x.cbegin(), x.cend()) {}

  ~Mem_root_array() { super::clear(); }

  // Not (yet) implemented.
  Mem_root_array(const Mem_root_array &) = delete;
  Mem_root_array &operator=(const Mem_root_array &) = delete;
};

#endif  // MEM_ROOT_ARRAY_INCLUDED
