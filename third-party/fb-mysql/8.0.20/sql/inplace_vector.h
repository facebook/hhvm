#ifndef INPLACE_VECTOR_INCLUDED
#define INPLACE_VECTOR_INCLUDED

/* Copyright (c) 2014, 2019, Oracle and/or its affiliates. All rights reserved.

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

/* This file defines the Inplace_vector class template. */

#include <vector>

#include "my_dbug.h"
#include "my_sys.h"
#include "mysql/psi/psi_memory.h"
#include "mysql/service_mysql_alloc.h"

/**
  Utility container class to store elements stably and scalably.
  The address of an element stored in the container is stable as long as
  the object is alive, no object is copy-constructed/reassigned by push_back
  operations once it's stored into this container. And users of such containers
  *can* assign to elements stored in the container just like using std::vector.

  It is similar to STL vector but it is uniquely suitable in below situation:
  whenever stable element address, or element copy construction/assignement
  behaviors are forbidden. It only has a limited subset of the std::vector
  interface, and especially it doesn't have an iterator interface or element
  elimination interface, we don't need them for now. And this container
  is not multi-threading safe. It uses my_malloc/my_free
  to allocate/free memory arrays and caller can pass PSI key.

  The container keeps a collection of arrays, each of which has a fixed
  NO. of slots to store elements. When one array is full, another is appended.
  When the vector shrinks at tail, useless arrays are removed and its memory
  space released.

  @tparam objtype The type of the elements to store.
  @tparam array_size The NO. of element slots in each array.
 */
template <typename objtype, size_t array_size = 16>
class Inplace_vector {
 private:
  std::vector<objtype *> m_obj_arrays;
  PSI_memory_key m_psi_key;
  size_t m_obj_count;
  bool m_outof_mem;

  /**
    Return an existing used slot, or append exactly one slot at end of the last
    array and return it. If the last array is already full before the
    append, allocate a new array then allocate one slot and return it.
    @param index The index of the element slot to return. It must be
                 one within valid in-use range of the vector, or be equal to
                 the size of the vector.
    @return the object pointer stored in the specified slot; NULL if need
            to allocate more space but out of memory.
   */
  objtype *get_space(size_t index) {
    DBUG_ASSERT(index <= m_obj_count);
    size_t arr_id = index / array_size;
    size_t slot_id = index % array_size;
    objtype *ptr = nullptr;

    DBUG_ASSERT(arr_id <= m_obj_arrays.size());

    // Appending a new slot causes appending a new array.
    if (arr_id == m_obj_arrays.size()) {
      DBUG_ASSERT(slot_id == 0);
      if (m_outof_mem) return nullptr;
      append_new_array();
      if (m_outof_mem) return nullptr;
    }

    ptr = m_obj_arrays[arr_id];
    ptr += slot_id;
    return ptr;
  }

  void append_new_array() {
    if (m_outof_mem) return;

    void *p = my_malloc(m_psi_key, sizeof(objtype) * array_size, MYF(MY_FAE));

    try {
      m_obj_arrays.push_back(static_cast<objtype *>(p));
    } catch (...) {
      m_outof_mem = true;
      my_free(p);
    }
  }

  Inplace_vector(const Inplace_vector &);
  Inplace_vector &operator=(const Inplace_vector &rhs);

 public:
  explicit Inplace_vector(PSI_memory_key psi_key)
      : m_psi_key(psi_key), m_outof_mem(false) {
    m_obj_count = 0;
    append_new_array();
  }

  /**
    Release memory space and destroy all contained objects.
    */
  ~Inplace_vector() { delete_all_objects(); }

  /**
    Get an existing element's pointer, index must be in [0, m_obj_count).
    @param index The index of the element to return. It must be
                 within valid in-use range of the vector.
    @return The element address specified by index; NULL if out of memory.
   */
  objtype *get_object(size_t index) {
    DBUG_ASSERT(index < m_obj_count);
    return get_space(index);
  }

  /**
    Allocate space for an object, and construct it using its default
    constructor, and return its address.
    @return the appended object's address; NULL if out of memory.
   */
  objtype *append_object() {
    // Use placement new operator to construct this object at proper
    // location.
    return ::new (get_space(m_obj_count++)) objtype;
  }

  /**
    STL std::vector::push_back interface. It's guaranteed that existing
    elements stored in the vector is never copy constructed/reassigned
    by this operation. When the last element array is full, a new one is
    allocated and tracked.

    @param obj The element to store into the vector.
    @return The appended object stored in the container; NULL if out of memory.
    */
  objtype *push_back(const objtype &obj) {
    // Use placement new operator to construct this object at proper
    // location.
    return ::new (get_space(m_obj_count++)) objtype(obj);
  }

  /**
    STL std::vector::resize interface. Has identical behavior as STL
    std::vector::resize except that no element copy construction or
    reassignment is ever caused by this operation.

    @param new_size New size of vector. If smaller than current size,
               objects at the tail are removed and destroyed. If greater,
               new objects are added with default value.
    @param val default value assigned to extended slots in the vector. Unused
               if the vector is shrinked.
    @return true if out of memory; false if successful.
    */
  bool resize(size_t new_size, const objtype &val = objtype()) {
    if (new_size > size()) {
      for (size_t i = size(); i < new_size; i++) {
        if (push_back(val) == nullptr) return true;
      }
    } else if (new_size < size()) {
      // Destroy objects at tail.
      for (size_t i = new_size; i < size(); i++) get_object(i)->~objtype();

      // Free useless array space.
      for (size_t j = new_size / array_size + 1; j < m_obj_arrays.size(); j++)
        my_free(m_obj_arrays[j]);

      m_obj_count = new_size;
      m_obj_arrays.resize(new_size / array_size + 1);
    }

    return false;
  }

  /**
    STL std::vector::size interface.
    @return the number of elements effectively stored in the vector.
    */
  size_t size() const { return m_obj_count; }

  /**
    STL std::vector::capacity interface.
    @return the max number of element that can be stored into this vector
            without growing its size.
   */
  size_t capacity() const { return m_obj_arrays.size() * array_size; }

  /**
    STL std::vector::empty interface.
    @return whether size() == 0.
    */
  bool empty() const { return m_obj_count == 0; }

  /**
    STL std::vector::clear interface.
    Destroy all elements (by calling each element's destructor) stored in
    the vector, and then release all memory held by it.
    */
  void clear() { delete_all_objects(); }

  /**
    STL std::vector::back interface.
    @return the reference of the last object stored in the vector.
    */
  const objtype &back() const {
    DBUG_ASSERT(size() > 0);
    objtype *p = get_object(size() - 1);
    return *p;
  }

  /**
    STL std::vector::back interface.
    @return the reference of the last object stored in the vector.
    */
  objtype &back() {
    DBUG_ASSERT(size() > 0);
    objtype *p = get_object(size() - 1);
    return *p;
  }

  /**
    STL std::vector::operator[] interface.
    @param i The index of the element to return. It must be
             within valid in-use range of the vector.
    @return The element reference specified by index.
    */
  const objtype &operator[](size_t i) const {
    DBUG_ASSERT(i < size());
    objtype *p = get_object(i);
    return *p;
  }

  /**
    STL std::vector::operator[] interface.
    @param i The index of the element to return. It must be
             within valid in-use range of the vector.
    @return The element reference specified by index.
    */
  objtype &operator[](size_t i) {
    DBUG_ASSERT(i < size());
    objtype *p = get_object(i);
    return *p;
  }

  /**
    Destroy all elements (by calling each element's destructor) stored in
    the vector, and then release all memory held by it.
   */
  void delete_all_objects() {
    // Call each element's destructor.
    for (size_t i = 0; i < size(); i++) {
      objtype *p = get_object(i);
      p->~objtype();
    }
    for (size_t i = 0; i < m_obj_arrays.size(); ++i) my_free(m_obj_arrays[i]);

    m_obj_arrays.clear();
    m_obj_count = 0;
  }
};

#endif  // !INPLACE_VECTOR_INCLUDED
