/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2013 Facebook, Inc. (http://www.facebook.com)     |
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
#ifndef incl_HPHP_REPO_AUTH_TYPE_ARRAY_INL_H_
#define incl_HPHP_REPO_AUTH_TYPE_ARRAY_INL_H_

#include <type_traits>
#include <limits>

#include "hphp/util/trace.h"

namespace HPHP {

//////////////////////////////////////////////////////////////////////

template<class SerDe>
typename std::enable_if<SerDe::deserializing>::type
ArrayTypeTable::serde(SerDe& sd) {
  TRACE_SET_MOD(rat);
  uint32_t size;
  sd(size);
  FTRACE(1, "ArrayTypeTable size = {}\n", size);
  decltype(m_arrTypes)(size).swap(m_arrTypes);
  for (auto i = uint32_t{0}; i < size; ++i) {
    m_arrTypes[i] = RepoAuthType::Array::deserialize(sd);
    assert(m_arrTypes[i] != nullptr);
    assert(m_arrTypes[i]->id() == i);
    FTRACE(2, "  {} {}\n", i, show(*m_arrTypes[i]));
  }
}

template<class SerDe>
typename std::enable_if<!SerDe::deserializing>::type
ArrayTypeTable::serde(SerDe& sd) {
  always_assert(m_arrTypes.size() < std::numeric_limits<uint32_t>::max());
  uint32_t const size = m_arrTypes.size();
  sd(size);
  for (auto i = uint32_t{0}; i < size; ++i) {
    m_arrTypes[i]->serialize(sd);
  }
}

template<class SerDe>
RepoAuthType::Array* RepoAuthType::Array::deserialize(SerDe& sd) {
  Tag tag;
  Empty emptiness;
  uint32_t id;
  uint32_t size;
  sd(tag)
    (emptiness)
    (id)
    (size)
    ;

  switch (tag) {
  case Tag::Packed:
    {
      auto const bytes = size * sizeof(RepoAuthType) + sizeof(Array);
      auto const arr   = new (std::malloc(bytes)) Array(tag, emptiness, size);
      auto tyPtr       = arr->types();
      auto const stop  = tyPtr + size;
      arr->m_id = id;
      for (; tyPtr != stop; ++tyPtr) {
        sd(*tyPtr);
      }
      return arr;
    }
  case Tag::PackedN:
    {
      auto const bytes = sizeof(RepoAuthType) + sizeof(Array);
      auto const arr   = new (std::malloc(bytes)) Array(tag, emptiness, size);
      arr->m_id = id;
      sd(arr->types()[0]);
      return arr;
    }
  }
  always_assert(!"deserialized invalid RepoAuthType::Array tag");
}

template<class SerDe>
void RepoAuthType::Array::serialize(SerDe& sd) const {
  sd(m_tag)
    (m_emptiness)
    (m_id)
    (m_size)
    ;

  switch (m_tag) {
  case Tag::Packed:
    {
      auto tyPtr      = types();
      auto const stop = tyPtr + m_size;
      for (; tyPtr != stop; ++tyPtr) {
        sd(*tyPtr);
      }
    }
    break;
  case Tag::PackedN:
    sd(types()[0]);
    break;
  }
}

//////////////////////////////////////////////////////////////////////

}

#endif
