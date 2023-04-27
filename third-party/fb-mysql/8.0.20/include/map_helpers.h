/* Copyright (c) 2017, 2019, Oracle and/or its affiliates. All rights reserved.

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

#ifndef MAP_HELPERS_INCLUDED
#define MAP_HELPERS_INCLUDED

#include <map>
#include <memory>
#include <string>
#include <type_traits>
#include <unordered_map>
#include <unordered_set>
#include <utility>

#include "m_ctype.h"
#include "my_inttypes.h"
#include "sql/malloc_allocator.h"
#include "sql/mem_root_allocator.h"
#include "template_utils.h"

/**
  Some useful helpers for associative arrays with MySQL-specific semantics.
*/

/**
  For unordered_map<Key, T*> or unordered_map<Key, unique_ptr<T>>, does
  find() and returns nullptr if the element was not found.

  It is not possible to distinguish between "not found" and "found, but
  contained nullptr" in this case. Thus, you should normally prefer checking
  against container.end() yourself.
*/
template <class Container, class Key>
static inline auto find_or_nullptr(const Container &container, const Key &key)
    -> typename std::enable_if<
        std::is_pointer<typename Container::value_type::second_type>::value,
        typename Container::value_type::second_type>::type {
  const auto it = container.find(key);
  if (it == container.end())
    return nullptr;
  else
    return it->second;
}

template <class Container, class Key>
static inline auto find_or_nullptr(const Container &container, const Key &key)
    -> typename std::enable_if<
        std::is_pointer<
            typename Container::value_type::second_type::pointer>::value,
        typename Container::value_type::second_type::pointer>::type {
  const auto it = container.find(key);
  if (it == container.end())
    return nullptr;
  else
    return it->second.get();
}

/**
  For unordered_multimap<Key, Value>, erase the first specific element that
  matches _both_ the given key and value.
*/
template <class Container>
typename Container::iterator erase_specific_element(
    Container *container, const typename Container::key_type &key,
    const typename Container::value_type::second_type &value) {
  auto it_range = container->equal_range(key);
  for (auto it = it_range.first; it != it_range.second; ++it) {
    if (it->second == value) return container->erase(it);
  }
  return container->end();
}

/**
  Same as regular erase_specific_element(), but for the case where the
  container holds unique_ptr elements.
*/
template <class Container>
static inline auto erase_specific_element(
    Container *container, const typename Container::key_type &key,
    typename Container::value_type::second_type::pointer value) ->
    typename std::enable_if<
        std::is_pointer<
            typename Container::value_type::second_type::pointer>::value,
        typename Container::iterator>::type {
  auto it_range = container->equal_range(key);
  for (auto it = it_range.first; it != it_range.second; ++it) {
    if (it->second.get() == value) return container->erase(it);
  }
  return container->end();
}

/**
  std::unique_ptr, but with a custom delete function.
  Normally, it is more efficient to have a deleter class instead,
  but this allows you to have a unique_ptr to a forward-declared class,
  so it keeps include dependencies down somewhat.
*/
template <class T>
using unique_ptr_with_deleter = std::unique_ptr<T, void (*)(T *)>;

template <typename T, bool call_destructor>
struct My_free_deleter {
  void operator()(void *ptr) const {
    reinterpret_cast<T *>(ptr)->~T();
    my_free(ptr);
  }
};

template <typename T>
struct My_free_deleter<T, false> {
  void operator()(void *ptr) const { my_free(ptr); }
};

/** std::unique_ptr, but with my_free as deleter. */
template <class T>
using unique_ptr_my_free = std::unique_ptr<
    T, My_free_deleter<T, !std::is_array<T>::value &&
                              !std::is_trivially_destructible<T>::value>>;

struct Free_deleter {
  void operator()(void *ptr) const { free(ptr); }
};

/** std::unique_ptr, but with free as deleter. */
template <class T>
using unique_ptr_free = std::unique_ptr<T, Free_deleter>;

/** A Hasher that hashes std::strings according to a MySQL collation. */
class Collation_hasher {
 public:
  explicit Collation_hasher(const CHARSET_INFO *cs_arg)
      : cs(cs_arg), hash_sort(cs->coll->hash_sort) {}

  size_t operator()(const std::string &s) const {
    uint64 nr1 = 1, nr2 = 4;
    hash_sort(cs, pointer_cast<const uchar *>(s.data()), s.size(), &nr1, &nr2);
    return nr1;
  }

 private:
  const CHARSET_INFO *cs;
  decltype(cs->coll->hash_sort) hash_sort;
};

/** A KeyEqual that compares std::strings according to a MySQL collation. */
class Collation_key_equal {
 public:
  explicit Collation_key_equal(const CHARSET_INFO *cs_arg)
      : cs(cs_arg), strnncollsp(cs->coll->strnncollsp) {}

  size_t operator()(const std::string &a, const std::string &b) const {
    return strnncollsp(cs, pointer_cast<const uchar *>(a.data()), a.size(),
                       pointer_cast<const uchar *>(b.data()), b.size()) == 0;
  }

 private:
  const CHARSET_INFO *cs;
  decltype(cs->coll->strnncollsp) strnncollsp;
};

/**
  std::unordered_map, but with my_malloc, so that you can track the memory
  used using PSI memory keys.
*/
template <class Key, class Value, class Hash = std::hash<Key>,
          class KeyEqual = std::equal_to<Key>>
class malloc_unordered_map
    : public std::unordered_map<Key, Value, Hash, KeyEqual,
                                Malloc_allocator<std::pair<const Key, Value>>> {
 public:
  /*
    In theory, we should be allowed to send in the allocator only, but GCC 4.8
    is missing several unordered_map constructors, so let's give in everything.
  */
  malloc_unordered_map(PSI_memory_key psi_key)
      : std::unordered_map<Key, Value, Hash, KeyEqual,
                           Malloc_allocator<std::pair<const Key, Value>>>(
            /*bucket_count=*/10, Hash(), KeyEqual(),
            Malloc_allocator<>(psi_key)) {}
};

/**
  std::unordered_set, but with my_malloc, so that you can track the memory
  used using PSI memory keys.
*/
template <class Key, class Hash = std::hash<Key>,
          class KeyEqual = std::equal_to<Key>>
class malloc_unordered_set
    : public std::unordered_set<Key, Hash, KeyEqual, Malloc_allocator<Key>> {
 public:
  /*
    In theory, we should be allowed to send in the allocator only, but GCC 4.8
    is missing several unordered_set constructors, so let's give in everything.
  */
  malloc_unordered_set(PSI_memory_key psi_key)
      : std::unordered_set<Key, Hash, KeyEqual, Malloc_allocator<Key>>(
            /*bucket_count=*/10, Hash(), KeyEqual(),
            Malloc_allocator<>(psi_key)) {}
};

/**
  std::unordered_multimap, but with my_malloc, so that you can track the memory
  used using PSI memory keys.
*/
template <class Key, class Value, class Hash = std::hash<Key>,
          class KeyEqual = std::equal_to<Key>>
class malloc_unordered_multimap
    : public std::unordered_multimap<
          Key, Value, Hash, KeyEqual,
          Malloc_allocator<std::pair<const Key, Value>>> {
 public:
  /*
    In theory, we should be allowed to send in the allocator only, but GCC 4.8
    is missing several unordered_multimap constructors, so let's give in
    everything.
  */
  malloc_unordered_multimap(PSI_memory_key psi_key)
      : std::unordered_multimap<Key, Value, Hash, KeyEqual,
                                Malloc_allocator<std::pair<const Key, Value>>>(
            /*bucket_count=*/10, Hash(), KeyEqual(),
            Malloc_allocator<>(psi_key)) {}
};

/**
  std::unordered_map, but with my_malloc and collation-aware comparison.
*/
template <class Key, class Value>
class collation_unordered_map
    : public std::unordered_map<Key, Value, Collation_hasher,
                                Collation_key_equal,
                                Malloc_allocator<std::pair<const Key, Value>>> {
 public:
  collation_unordered_map(const CHARSET_INFO *cs, PSI_memory_key psi_key)
      : std::unordered_map<Key, Value, Collation_hasher, Collation_key_equal,
                           Malloc_allocator<std::pair<const Key, Value>>>(
            /*bucket_count=*/10, Collation_hasher(cs), Collation_key_equal(cs),
            Malloc_allocator<>(psi_key)) {}
};

/**
  std::unordered_multimap, but with my_malloc and collation-aware comparison.
*/
template <class Key, class Value>
class collation_unordered_multimap
    : public std::unordered_multimap<
          Key, Value, Collation_hasher, Collation_key_equal,
          Malloc_allocator<std::pair<const Key, Value>>> {
 public:
  collation_unordered_multimap(CHARSET_INFO *cs, PSI_memory_key psi_key)
      : std::unordered_multimap<Key, Value, Collation_hasher,
                                Collation_key_equal,
                                Malloc_allocator<std::pair<const Key, Value>>>(
            /*bucket_count=*/10, Collation_hasher(cs), Collation_key_equal(cs),
            Malloc_allocator<>(psi_key)) {}
};

/**
  std::unordered_set, but with my_malloc and collation-aware comparison.
*/
template <class Key>
class collation_unordered_set
    : public std::unordered_set<Key, Collation_hasher, Collation_key_equal,
                                Malloc_allocator<Key>> {
 public:
  collation_unordered_set(CHARSET_INFO *cs, PSI_memory_key psi_key)
      : std::unordered_set<Key, Collation_hasher, Collation_key_equal,
                           Malloc_allocator<Key>>(
            /*bucket_count=*/10, Collation_hasher(cs), Collation_key_equal(cs),
            Malloc_allocator<>(psi_key)) {}
  collation_unordered_set(std::initializer_list<Key> il, CHARSET_INFO *cs,
                          PSI_memory_key psi_key)
      : std::unordered_set<Key, Collation_hasher, Collation_key_equal,
                           Malloc_allocator<Key>>(
            il, /*bucket_count=*/10, Collation_hasher(cs),
            Collation_key_equal(cs), Malloc_allocator<>(psi_key)) {}
};

/** std::unordered_set, but allocated on a MEM_ROOT.  */
template <class Key, class Hash = std::hash<Key>,
          class KeyEqual = std::equal_to<Key>>
class mem_root_unordered_set
    : public std::unordered_set<Key, Hash, KeyEqual, Mem_root_allocator<Key>> {
 public:
  /*
    In theory, we should be allowed to send in the allocator only, but GCC 4.8
    is missing several unordered_set constructors, so let's give in everything.
  */
  mem_root_unordered_set(MEM_ROOT *mem_root)
      : std::unordered_set<Key, Hash, KeyEqual, Mem_root_allocator<Key>>(
            /*bucket_count=*/10, Hash(), KeyEqual(),
            Mem_root_allocator<Key>(mem_root)) {}
};

/**
  std::unordered_map, but allocated on a MEM_ROOT.
*/
template <class Key, class Value, class Hash = std::hash<Key>,
          class KeyEqual = std::equal_to<Key>>
class mem_root_unordered_map
    : public std::unordered_map<
          Key, Value, Hash, KeyEqual,
          Mem_root_allocator<std::pair<const Key, Value>>> {
 public:
  mem_root_unordered_map(MEM_ROOT *mem_root)
      : std::unordered_map<Key, Value, Hash, KeyEqual,
                           Mem_root_allocator<std::pair<const Key, Value>>>(
            /*bucket_count=*/10, Hash(), KeyEqual(),
            Mem_root_allocator<std::pair<const Key, Value>>(mem_root)) {}
};

// std::unordered_multimap, but allocated on a MEM_ROOT.
template <class Key, class Value, class Hash,
          class KeyEqual = std::equal_to<Key>>
class mem_root_unordered_multimap
    : public std::unordered_multimap<
          Key, Value, Hash, KeyEqual,
          Mem_root_allocator<std::pair<const Key, Value>>> {
 public:
  mem_root_unordered_multimap(MEM_ROOT *mem_root, Hash hash)
      : std::unordered_multimap<
            Key, Value, Hash, KeyEqual,
            Mem_root_allocator<std::pair<const Key, Value>>>(
            /*bucket_count=*/10, hash, KeyEqual(),
            Mem_root_allocator<std::pair<const Key, Value>>(mem_root)) {}
};

/**
  std::unordered_map, but collation aware and allocated on a MEM_ROOT.
*/
template <class Key, class Value>
class mem_root_collation_unordered_map
    : public std::unordered_map<
          Key, Value, Collation_hasher, Collation_key_equal,
          Mem_root_allocator<std::pair<const Key, Value>>> {
 public:
  mem_root_collation_unordered_map(const CHARSET_INFO *cs, MEM_ROOT *mem_root)
      : std::unordered_map<Key, Value, Collation_hasher, Collation_key_equal,
                           Mem_root_allocator<std::pair<const Key, Value>>>(
            /*bucket_count=*/10, Collation_hasher(cs), Collation_key_equal(cs),
            Mem_root_allocator<std::pair<const Key, Value>>(mem_root)) {}
};

#endif  // MAP_HELPERS_INCLUDED
