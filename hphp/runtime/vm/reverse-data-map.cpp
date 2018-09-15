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

#include "hphp/runtime/vm/reverse-data-map.h"

#include "hphp/runtime/base/string-data.h"
#include "hphp/runtime/vm/class.h"
#include "hphp/runtime/vm/func.h"
#include "hphp/runtime/vm/named-entity.h"
#include "hphp/runtime/vm/unit.h"

#include "hphp/util/assertions.h"
#include "hphp/util/multibitset.h"

#include <algorithm>
#include <mutex>

namespace HPHP { namespace data_map {

///////////////////////////////////////////////////////////////////////////////

namespace {

///////////////////////////////////////////////////////////////////////////////

#define META_TYPES \
  X(Class)\
  X(Func)\
  X(NamedEntity)\
  X(StringData)\
  X(Unit)\
  /* */

enum class Kind : uint8_t {
  Class = 1,
  Func = 2,
  NamedEntity = 3,
  StringData = 4,
  Unit = 5,
};

constexpr size_t kChunkSize = 1 << 21;
constexpr auto kAlignShift = 3;
DEBUG_ONLY constexpr auto kAlignMask = 0x7ull;

/*
 * Multibitset which tracks the logical starts of various objects.
 *
 * Each multibit position represents an 8-byte aligned address.  The enum value
 * it contains indicates what (if any) object starts there.
 *
 * Using this scheme, for every 8 bytes in the total range of allocations of
 * tracked objects (with both start and end aligned to kChunkSize bytes), we
 * need 3 bits.  If the objects are packed tightly, this should result in
 * roughly a 3/64 increase in memory footprint.
 */
chunked_multibitset<3>* s_starts = nullptr;
std::mutex s_lock;

/*
 * Ensure that `s_starts' has been allocated.
 */
void init_if_not() {
  if (s_starts) return;

  std::lock_guard<std::mutex> l(s_lock);
  if (s_starts) return;

  s_starts = new chunked_multibitset<3>(kChunkSize >> 3);
}

void register_start(Kind kind, const void* meta) {
  init_if_not();
  auto& starts = *s_starts;

  auto const imm = reinterpret_cast<uintptr_t>(meta);
  assertx((imm & ~kAlignMask) == imm);

  std::lock_guard<std::mutex> l(s_lock);
  starts[imm >> kAlignShift] = static_cast<uint8_t>(kind);
}

void deregister(Kind /*kind*/, const void* meta) {
  assertx(s_starts);
  auto& starts = *s_starts;

  auto const imm = reinterpret_cast<uintptr_t>(meta);
  assertx((imm & ~kAlignMask) == imm);

  std::lock_guard<std::mutex> l(s_lock);
  assertx(starts[imm >> kAlignShift]);
  starts[imm >> kAlignShift] = 0;
}

/*
 * Is `addr' logically an internal pointer for the given base pointer?
 *
 * These routines account for variable-length prefix or suffix allocations,
 * such as Class's funcVec and classVec, or StringData's data().
 */
bool contains(const Class* cls, const void* addr) {
  return cls->funcVec() <= addr && addr < cls->mallocEnd();
}

bool contains(const Func* func, const void* addr) {
  return func <= addr && addr < func->mallocEnd();
}

bool contains(const NamedEntity* ne, const void* addr) {
  return ne <= addr && addr < ne + 1;
}

bool contains(const StringData* sd, const void* addr) {
  auto const start = reinterpret_cast<const char*>(sd);
  return (start <= addr && addr < start + sd->heapSize());
}

bool contains(const Unit* unit, const void* addr) {
  return unit <= addr && addr < unit + 1;
}

/*
 * Does the object whose start address is represented by the result of
 * `get_pos()' in the reverse map contain `addr' as an internal pointer?
 *
 * @requires: s_start != nullptr
 */
template<class PosFn>
result start_pos_contains(PosFn get_pos, const void* addr) {
  assertx(s_starts);
  auto const& starts = *s_starts;

  size_t pos;
  Kind kind;
  { // Only the multibitset operations are performed under lock.  We assume
    // that all the objects we track are "sufficiently persistent" such that
    // while the request we're in persists, they won't be freed.
    std::lock_guard<std::mutex> l(s_lock);

    pos = get_pos();
    if (pos == starts.npos) return result{};

    assertx(starts[pos] != 0);
    kind = static_cast<Kind>(starts[pos]);
  }

  switch (kind) {
#define X(Meta)       \
    case Kind::Meta:  \
    {                 \
      auto const meta = reinterpret_cast<Meta*>(pos << kAlignShift);  \
      return contains(meta, addr) ? result{meta} : result{};          \
    }
    META_TYPES
#undef X
  }
  not_reached();
}

///////////////////////////////////////////////////////////////////////////////

}

///////////////////////////////////////////////////////////////////////////////

#define X(Meta) \
  void register_start(const Meta* meta) { \
    register_start(Kind::Meta, meta);     \
  }                                       \
  void deregister(const Meta* meta) {     \
    deregister(Kind::Meta, meta);         \
  }
META_TYPES
#undef X

result find_containing(const void* addr) {
  init_if_not();
  auto const& starts = *s_starts;

  auto const pos = reinterpret_cast<uintptr_t>(addr) >> kAlignShift;

  // Search before `addr' first, in the belief that internal pointers are more
  // likely to be after the object's start address than before.
  auto const res = start_pos_contains([&] { return starts.fls(pos); }, addr);
  if (!res.empty()) return res;
  return start_pos_contains([&] { return starts.ffs(pos); }, addr);
}

///////////////////////////////////////////////////////////////////////////////

}}
