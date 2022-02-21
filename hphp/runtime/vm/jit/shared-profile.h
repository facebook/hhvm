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

#pragma once

#include "hphp/runtime/base/rds.h"
#include "hphp/runtime/base/runtime-option.h"
#include "hphp/runtime/base/types.h"

#include "hphp/runtime/vm/jit/bc-marker.h"
#include "hphp/runtime/vm/jit/print.h"
#include "hphp/runtime/vm/jit/translator.h"
#include "hphp/runtime/vm/jit/types.h"

#include "hphp/util/assertions.h"
#include "hphp/util/tiny-vector.h"

#include <tbb/concurrent_hash_map.h>

#include <typeindex>
#include <utility>

namespace HPHP { namespace jit {

struct ProfDataDeserializer;
struct ProfDataSerializer;

//////////////////////////////////////////////////////////////////////////////

char* fetchSharedProfile(
    TransID tid, Offset bcOff, const StringData* name,
    bool create, std::type_index type);

void deserializeSharedProfiles(ProfDataDeserializer& des);
void serializeSharedProfiles(ProfDataSerializer& ser);

template <typename T>
struct alignas(64) SharedProfileEntry {
  std::mutex mutex;
  T value;

  template <typename F>
  void update(F&& f) {
    std::unique_lock<std::mutex> lock(mutex, std::try_to_lock);
    if (!lock.owns_lock()) return;
    f(value);
  }
};

template <typename T>
SharedProfileEntry<T>* fetchSharedProfile(
    TransID tid, Offset bcOff, const StringData* name, bool create) {
  auto const type = std::type_index(typeid(T));
  auto const result = fetchSharedProfile(tid, bcOff, name, create, type);
  return reinterpret_cast<SharedProfileEntry<T>*>(result);
}

//////////////////////////////////////////////////////////////////////////////

template<class T>
struct SharedProfile {
  using Entry = SharedProfileEntry<T>;
  using Links = TinyVector<Entry*, 1>;

  SharedProfile(const TransContext& context,
                const BCMarker& marker,
                const StringData* name)
    : m_kind(context.kind)
    , m_links(createLinks(context, marker, name))
  {}

  /*
   * Access the data we collected during profiling.
   *
   * Pre: optimizing()
   */
  T data() const {
    assertx(optimizing());
    auto result = T{};
    for (auto const link : m_links) {
      if (!link) continue;
      std::lock_guard<std::mutex> _(link->mutex);
      T::reduce(result, link->value);
    }
    return result;
  }

  /*
   * Access this profile's entry in order to update it.
   *
   * Pre: profiling()
   */
  Entry* entry() const {
    assertx(profiling());
    assertx(m_links.size() == 1);
    assertx(m_links[0] != nullptr);
    return m_links[0];
  }

  /*
   * Query whether we are profiling or optimizing via this handle to a given
   * shared profile. It's possible that neither is true, e.g. if we're JIT-ing
   * a live translation, or an optimized one without a link from profiling.
   */
  bool optimizing() const {
    if (m_kind != TransKind::Optimize) {
      return false;
    }
    for (auto const link : m_links) {
      if (link) return true;
    }
    return false;
  }
  bool profiling() const {
    return m_kind == TransKind::Profile;
  }

private:
  static constexpr Offset kPrologueOffset = -1;
  static constexpr Offset kFuncEntryOffset = -2;

  static Offset bcOffForProfileKey(SrcKey sk) {
    // Use a placeholder value for prologues and function entries, as they
    // are not part of a bytecode at any offset. Profiling of prologues for
    // different number of arguments is still differentiated by profTransID.
    if (sk.prologue()) return kPrologueOffset;
    if (sk.funcEntry()) return kFuncEntryOffset;
    return sk.offset();
  }

  static Entry* createEntry(TransID profTransID,
                            TransKind kind,
                            Offset bcOff,
                            const StringData* name) {
    switch (kind) {
      case TransKind::Profile:
        return fetchSharedProfile<T>(profTransID, bcOff, name, true);

      case TransKind::Optimize:
        if (!isValidTransID(profTransID)) return nullptr;
        return fetchSharedProfile<T>(profTransID, bcOff, name, false);

      case TransKind::Anchor:
      case TransKind::Interp:
      case TransKind::Live:
      case TransKind::LivePrologue:
      case TransKind::ProfPrologue:
      case TransKind::OptPrologue:
      case TransKind::Invalid:
        return nullptr;
    }
    always_assert(false);
  }

  static Links createLinks(const TransContext& context,
                           const BCMarker& marker,
                           const StringData* name) {
    auto const kind = context.kind;
    auto const profiling = context.kind == TransKind::Profile;
    auto const& tids = profiling ? context.transIDs : marker.profTransIDs();
    auto const bcOff = bcOffForProfileKey(marker.sk());

    auto result = Links{};
    result.reserve(std::max(tids.size(), static_cast<size_t>(1)));

    // NB: tids can be empty during tracelet formation. In this case, create
    // a dummy link corresponding to kInvalidTransID.
    if (tids.empty()) {
      result.push_back(createEntry(kInvalidTransID, kind, bcOff, name));
    }
    for (auto const tid : tids) {
      result.push_back(createEntry(tid, kind, bcOff, name));
    }
    return result;
  }

private:
  const TransKind m_kind;
  const Links m_links;
};

///////////////////////////////////////////////////////////////////////////////

}}
