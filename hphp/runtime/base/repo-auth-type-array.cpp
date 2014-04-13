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
#include "hphp/runtime/base/repo-auth-type-array.h"

#include <cassert>
#include <cstdlib>
#include <mutex>
#include <unordered_set>

#include "hphp/util/compilation-flags.h"
#include "hphp/util/trace.h"

namespace HPHP {

//////////////////////////////////////////////////////////////////////

namespace {

/*
 * Before we've inserted arrays into either a ArrayTypeTable::Builder
 * or an actual ArrayTypeTable, they can't be compared just using
 * their ids, so we have these.
 */

struct repo_auth_array_hash {
  size_t operator()(const RepoAuthType::Array* ar) const {
    size_t hash = folly::hash::hash_combine(
      static_cast<uint32_t>(ar->tag()),
      static_cast<uint32_t>(ar->emptiness()),
      ar->size()
    );
    using T = RepoAuthType::Array::Tag;
    auto const size = ar->size();
    switch (ar->tag()) {
    case T::Packed:
      for (auto i = uint32_t{0}; i < size; ++i) {
        // If we have arrays of arrays, this can try to hash the inner
        // arrays.  This is safe (it uses the array id) because they
        // must already be inserted in the array table builder.
        hash = folly::hash::hash_128_to_64(hash, ar->packedElem(i).hash());
      }
      break;
    }
    return hash;
  }
};

struct repo_auth_array_eq {
  bool operator()(const RepoAuthType::Array* a,
                  const RepoAuthType::Array* b) const {
    if (a->tag() != b->tag() ||
        a->emptiness() != b->emptiness() ||
        a->size() != b->size()) {
      return false;
    }
    using T = RepoAuthType::Array::Tag;
    auto const size = a->size();
    switch (a->tag()) {
    case T::Packed:
      for (auto i = uint32_t{0}; i < size; ++i) {
        if (a->packedElem(i) != b->packedElem(i)) return false;
      }
      return true;
    }
    not_reached();
  }
};

}

//////////////////////////////////////////////////////////////////////

/*
 * Note about nested array types in the builder:
 *
 * The serialization we use for the ArrayTypeTable doesn't currently
 * allow the possibility of recursive array types.  This means that
 * any array type we create can only contain nested array
 * RepoAuthTypes that have smaller array ids (they must already have
 * been created when we create the outer array type).
 */

using Builder = ArrayTypeTable::Builder;

struct Builder::Impl {
  std::mutex mutex;
  std::unordered_set<
    const RepoAuthType::Array*,
    repo_auth_array_hash,
    repo_auth_array_eq
  > types;
  uint32_t nextId{0};
};

Builder::Builder() : m_impl{new Impl()} {}
Builder::~Builder() {}

//////////////////////////////////////////////////////////////////////

const RepoAuthType::Array*
Builder::packed(RepoAuthType::Array::Empty emptiness,
                const std::vector<RepoAuthType>& types) {
  assert(!types.empty());

  auto const size = types.size() * sizeof(RepoAuthType) +
    sizeof(RepoAuthType::Array);
  auto const arr = new (std::malloc(size)) RepoAuthType::Array(
    RepoAuthType::Array::Tag::Packed,
    emptiness,
    types.size()
  );

  auto elems = arr->m_types;
  for (auto& t : types) {
    *elems++ = t;
  }

  // If the table builder doesn't take this one, we have to free it.
  auto const ret = insert(arr);
  if (arr != ret) {
    arr->~Array();
    std::free(arr);
  }

  return ret;
}

//////////////////////////////////////////////////////////////////////

// Returns the `cand' if it was successfully inserted; otherwise it's
// the callers responsibility to free it.
const RepoAuthType::Array* Builder::insert(RepoAuthType::Array* cand) {
  assert(cand->id() == -1u);
  std::lock_guard<std::mutex> g(m_impl->mutex);
  auto ins = m_impl->types.insert(cand);
  if (ins.second) {
    cand->m_id = m_impl->nextId++;
    assert(*ins.first == cand);
    assert((*ins.first)->id() == cand->id());
    return cand;
  }
  return *ins.first;
}

void ArrayTypeTable::repopulate(const Builder& builder) {
  decltype(m_arrTypes){}.swap(m_arrTypes);

  m_arrTypes.resize(builder.m_impl->nextId);
  for (auto& ty : builder.m_impl->types) {
    assert(m_arrTypes[ty->id()] == nullptr);
    m_arrTypes[ty->id()] = ty;
  }
  if (debug) {
    for (auto& t : m_arrTypes) {
      always_assert(t != nullptr);
    }
  }
}

//////////////////////////////////////////////////////////////////////

std::string show(const RepoAuthType::Array& ar) {
  auto ret = std::string{};

  using A = RepoAuthType::Array;
  using T = A::Tag;
  switch (ar.emptiness()) {
  case A::Empty::No:
    ret += "N(";    // non-empty
    break;
  case A::Empty::Maybe:
    ret += '(';
    break;
  }

  switch (ar.tag()) {
  case T::Packed:
    for (auto i = uint32_t{0}; i < ar.size(); ++i) {
      ret += show(ar.packedElem(i));
      if (i != ar.size() - 1) ret += ',';
    }
  }

  ret += ')';
  return ret;
}

//////////////////////////////////////////////////////////////////////

}
