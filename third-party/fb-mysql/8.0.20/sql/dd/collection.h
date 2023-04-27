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

#ifndef DD__COLLECTION_IMPL_INCLUDED
#define DD__COLLECTION_IMPL_INCLUDED

#include <stddef.h>
#include <sys/types.h>
#include <algorithm>
#include <iterator>
#include <type_traits>
#include <vector>

#include "my_dbug.h"
#include "my_inttypes.h"

namespace dd {

///////////////////////////////////////////////////////////////////////////

class Object_key;
class Open_dictionary_tables_ctx;
class Raw_table;

template <typename T>
class Collection {
 public:
  // Pointer to abstract type. Template argument.
  typedef T value_type;
  // Abstract type.
  typedef typename std::remove_pointer<T>::type abstract_type;
  // Implementation type. Pointer to this type is actually stored in the vector.
  typedef typename abstract_type::Impl impl_type;
  typedef std::vector<impl_type *> Array;

 private:
  Array m_items;
  Array m_removed_items;

  void clear_all_items();

  void renumerate_items() {
    for (size_t i = 0; i < m_items.size(); ++i)
      m_items[i]->set_ordinal_position(static_cast<uint>(i + 1));
  }

  class Collection_iterator
      : public std::iterator<std::forward_iterator_tag, T> {
   public:
    Collection_iterator(Array *array)
        : m_array(array), m_current(array->begin()), m_current_obj(nullptr) {}

    Collection_iterator(Array *array, typename Array::iterator it)
        : m_array(array), m_current(it), m_current_obj(nullptr) {}

    bool operator==(const Collection_iterator &iter) const {
      return (m_array == iter.m_array && m_current == iter.m_current);
    }

    bool operator!=(const Collection_iterator &iter) const {
      return (m_array != iter.m_array || m_current != iter.m_current);
    }

    Collection_iterator &operator++() {
      if (m_current != m_array->end()) ++m_current;
      return *this;
    }

    T &operator*();

    Collection_iterator &end() {
      m_current = m_array->end();
      return *this;
    }

    typename Array::iterator current() { return m_current; }

   private:
    Array *m_array;
    typename Array::iterator m_current;
    T m_current_obj;
  };

  class Collection_const_iterator
      : public std::iterator<std::forward_iterator_tag, const abstract_type *> {
   public:
    Collection_const_iterator(const Array *array)
        : m_array(array), m_current(array->begin()), m_current_obj(nullptr) {}

    Collection_const_iterator(const Array *array,
                              typename Array::const_iterator it)
        : m_array(array), m_current(it), m_current_obj(nullptr) {}

    bool operator==(const Collection_const_iterator &iter) const {
      return (m_array == iter.m_array && m_current == iter.m_current);
    }

    bool operator!=(const Collection_const_iterator &iter) const {
      return (m_array != iter.m_array || m_current != iter.m_current);
    }

    Collection_const_iterator &operator++() {
      if (m_current != m_array->end()) ++m_current;
      return *this;
    }

    const abstract_type *&operator*();

    Collection_const_iterator &end() {
      m_current = m_array->end();
      return *this;
    }

    typename Array::const_iterator current() const { return m_current; }

   private:
    const Array *m_array;
    typename Array::const_iterator m_current;
    const abstract_type *m_current_obj;
  };

 public:
  Collection() {}

  ~Collection() { clear_all_items(); }

  /**
    Remove elements from m_removed_items.  This is used only in case of
    dropping triggers for now.  See comments in
    Table_impl::store_children() for more details.
  */
  void clear_removed_items();

  Collection(const Collection &) = delete;
  void operator=(Collection &) = delete;

  typedef Collection_iterator iterator;
  typedef Collection_const_iterator const_iterator;

  void push_back(impl_type *item) {
    item->set_ordinal_position(static_cast<uint>(m_items.size() + 1));
    m_items.push_back(item);
  }

  void push_front(impl_type *item) {
    m_items.insert(m_items.begin(), item);
    renumerate_items();
  }

  void insert(iterator it, impl_type *item) {
    m_items.insert(it.current(), item);
    renumerate_items();
  }

  void remove(impl_type *item);

  /**
    Remove all items and move it to m_removed_items items.
  */

  void remove_all() { m_removed_items = std::move(m_items); }

  /**
    Find item and return the position.

    @returns iterator pointing to found element.
  */

  iterator find(const impl_type *item);

  iterator begin() { return iterator(&m_items); }

  const_iterator begin() const { return const_iterator(&m_items); }

  iterator end() {
    iterator it(&m_items);
    it.end();
    return it;
  }

  const_iterator end() const {
    const_iterator it(&m_items);
    it.end();
    return it;
  }

  const_iterator cbegin() const { return begin(); }

  const_iterator cend() const { return end(); }

  bool empty() const { return m_items.empty() && m_removed_items.empty(); }

  /**
    Check if some of collection elements are removed.

    @returns void.
  */

  bool has_removed_items() const { return !m_removed_items.empty(); }

  size_t size() const { return m_items.size(); }

  const abstract_type *at(size_t n) const;

  T at(size_t n) {
    DBUG_ASSERT(n < size());
    return m_items[n];
  }

  const abstract_type *front() const { return at(0); }
  T front() { return at(0); }

  const abstract_type *back() const { return at(size() - 1); }
  T back() { return at(size() - 1); }

  const abstract_type *operator[](size_t n) const { return at(n); }
  T operator[](size_t n) { return at(n); }

  /**
    @brief
    Populate collection with items read from DD table.

    @details
    Iterate through DD tables to find rows that match the 'Object_key'
    supplied. Create collection item for each row we find and populate
    the item with data read from DD. Sort items in collection by their
    ordinal position property.

    @param parent - Object owning the restored object.
    @param otx - Context with information about open tables.
    @param table - The DD table from which read rows for items.
    @param key - The search key to be used to find rows.

    @return true - on failure and error is reported.
    @return false - on success.
  */
  template <typename Parent_item>
  bool restore_items(Parent_item *parent, Open_dictionary_tables_ctx *otx,
                     Raw_table *table, Object_key *key);

  /**
    Populate collection with items read from DD table.

    @details
    Iterate through DD tables to find rows that match the 'Object_key'
    supplied. Create collection item for each row we find and populate
    the item with data read from DD. Sort items in collection using
    comparator provided.

    @param parent   Object owning the restored object.
    @param otx      Context with information about open tables.
    @param table    The DD table from which read rows for items.
    @param key      The search key to be used to find rows.
    @param comp     Comparator to be used for sorting items.

    @retval True    on failure and error is reported.
    @retval False   on success.
  */
  template <typename Parent_item, typename Compare>
  bool restore_items(Parent_item *parent, Open_dictionary_tables_ctx *otx,
                     Raw_table *table, Object_key *key, Compare comp);

  /**
    @brief
    store items in collection on to DD table.

    @details
    Iterate through collection and stores them in DD tables.

    @param otx - Context with information about open tables.

    @return true - on failure and error is reported.
    @return false - on success.
  */
  bool store_items(Open_dictionary_tables_ctx *otx);

  /**
    @brief
    Remove all items details from DD table.

    @details
    Iterate through the collection and remove respective rows
    from DD tables.

    @param otx - Context with information about open tables.
    @param table - The DD table from which rows are removed.
    @param key - The search key to use to find rows.

    @return true - on failure and error is reported.
    @return false - on success.
  */
  bool drop_items(Open_dictionary_tables_ctx *otx, Raw_table *table,
                  Object_key *key) const;

  /**
    @brief
    Do a deep copy of a given collection.

    @details
    Calls clone() on the items in the given collection and
    stores the result in this collection.

    @param src - Collection to do a deep copy of.
    @param parent - Object "owning" the items in the collection.
                    E.g. Columns are owned by Table.
  */
  template <typename Parent_item>
  void deep_copy(const Collection<T> &src, Parent_item *parent);
};

///////////////////////////////////////////////////////////////////////////

}  // namespace dd

#endif  // DD__COLLECTION_PARENT_INCLUDED
