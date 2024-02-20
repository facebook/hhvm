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

#include "hphp/runtime/vm/func.h"

/*
 * Memoization cache support
 *
 * Memoization caches are used to support functions marked <<__Memoize>> and can
 * either be in RDS (for static functions) or hung-off ObjectData* (for
 * methods).
 *
 * All memo caches map some set of keys (which are only integers or strings) to
 * a particular TypedValue value. Caches can either be non-shared or shared. A
 * non-shared cache is for exactly one function, and the keys (corresponding to
 * the parameters) is all that's necessary to lookup the value. Non-shared
 * caches are used for static functions, and (as an optimization) on methods for
 * classes with few memoized functions. A shared cache can be used by different
 * functions, and therefore stores an additional key value distinguishing that
 * function (the form of this key varies by the cache type).
 *
 * Memo caches are only manipulated via their associated getter and setter
 * functions. Getter functions receive a pointer to MemoCacheBase, a pointer to
 * an array of Cells (which must be integers or strings) representing the
 * parameters, and, if shared, some additional value distinguishing that
 * function. If the value is found, a pointer to it is returned. If not, or if
 * the pointer to the cache is null, null is returned. No ref-count manipulation
 * is done in the returned value. Setter functions receive a *reference* to a
 * pointer to MemoCacheBase, a pointer to an array of Cells, if shared, an
 * additional value distinguishing that function, and a TypedValue to store in the
 * cache. The TypedValue is stored in the cache, and its ref-count is incremented
 * (with any previous value being dec-reffed and overwritten). If the pointer to
 * the cache was null, a new cache is allocated, and the pointer is updated to
 * point at it.
 *
 * For optimization, there's a number of specialized memo-caches. These are
 * specialized on a specific number of keys, and perhaps the types of
 * keys. These specializations are not exposed directly, but instead you call a
 * function to obtain the appropriate getter and setter. For a particular memo
 * cache, the same specialized get/set function must always be used! It is not
 * legal to sometimes use a specialized get/set function it, and a generic one
 * elesewhere.
 */

namespace HPHP {

/*
 * The actual implementation of the memo-caches are private to memo-cache.cpp
 * (since they involve a lot of templates). The rest of the runtime interacts
 * with them via a pointer to this base class. The set functions will create a
 * new memo-cache if the pointer is null, and update the pointer. We can destroy
 * them polymorphically when releasing an ObjectData, so the destructor must be
 * virtual.
 */
struct MemoCacheBase {
  virtual ~MemoCacheBase() = default;
  /* Returns a vector of key value pair where first value is FuncId and second
  *  value is the memory footprint for that entry. Puts one entry in the vector
  *  per cache entry. The vector reference is supplied by the caller, so the
  *  caller has the option to group together multiple caches.
  */
  virtual void heapSizesPerCacheEntry(std::vector<std::pair<FuncId, size_t>>&) const = 0;
};

////////////////////////////////////////////////////////////

/*
 * Specialized getters and setters
 *
 * Up to certain limits, we have specialized caches for specific key counts and
 * key types. The exact nature of these specializations is an implementation
 * detail, so instead of enumerating the all appropriate get/set functions here,
 * we provide an interface to get the appropriate function pointer.
 *
 * Each "flavor" has a "GetForKeyTypes" and "GetForKeyCount" lookup
 * function. The first is for when you know the specific types of the keys. Keys
 * can only be integers or strings, and the types are represented by an array of
 * booleans, where true means string. If you don't know the specific types, one
 * can use "GetForKeyCount" to obtain a function specialized on just the key
 * count (Key type specialized and key count specialized get/set functions can
 * be interchanged freely on the same memo cache). If the function returns null,
 * there's no specialization available and one has to use a generic cache.
 *
 * For the shared-case, a FuncId is used to distinguish functions. A key-count
 * of zero never has a specialized representation here. It makes no sense for
 * the non-shared case, and the shared case should just use "shared-only"
 * caches.
 */

// Non-shared getter
using MemoCacheGetter =
  const TypedValue* (*)(const MemoCacheBase*, const TypedValue*);
MemoCacheGetter memoCacheGetForKeyTypes(const bool* types, size_t count);
MemoCacheGetter memoCacheGetForKeyCount(size_t count);

// Non-shared setter
using MemoCacheSetter =
  void (*)(MemoCacheBase*&, const TypedValue*, TypedValue);
MemoCacheSetter memoCacheSetForKeyTypes(const bool* types, size_t count);
MemoCacheSetter memoCacheSetForKeyCount(size_t count);

// Shared getter
using SharedMemoCacheGetter =
  const TypedValue* (*)(const MemoCacheBase*, FuncId, const TypedValue*);
SharedMemoCacheGetter sharedMemoCacheGetForKeyTypes(const bool* types,
                                                    size_t count);
SharedMemoCacheGetter sharedMemoCacheGetForKeyCount(size_t count);

// Shared setter
using SharedMemoCacheSetter =
  void (*)(MemoCacheBase*&, FuncId, const TypedValue*, TypedValue);
SharedMemoCacheSetter sharedMemoCacheSetForKeyTypes(const bool* types,
                                                    size_t count);
SharedMemoCacheSetter sharedMemoCacheSetForKeyCount(size_t count);

////////////////////////////////////////////////////////////

/*
 * Generic getter and setter
 *
 * Generic memo caches can handle keys of any type or length (but are
 * slower). They are used when we don't have a specialized representation for a
 * particular key count.
 *
 * The generic cache get/set functions all take GenericMemoId::Param, which
 * serves the dual purpose of distinguishing different functions (since generic
 * caches can be shared), and encoding the number of keys.
 */
struct GenericMemoId {
  GenericMemoId(FuncId funcId, uint32_t keyCount)
    : m_s{funcId, keyCount}
  {}

  using Param = uint64_t;

  explicit GenericMemoId(Param combined)
    : combined{combined} {}

  FuncId getFuncId() const { return m_s.funcId; }
  uint32_t getKeyCount() const { return m_s.keyCount; }

  Param asParam() const { return combined; }

  void setKeyCount(uint32_t c) { m_s.keyCount = c; }

private:
  union {
    struct {
      FuncId funcId;
      uint32_t keyCount;
    } m_s;
    uint64_t combined;
  };
};

const TypedValue* memoCacheGetGeneric(MemoCacheBase* base,
                                GenericMemoId::Param id,
                                const TypedValue* keys);
void memoCacheSetGeneric(MemoCacheBase*& base,
                         GenericMemoId::Param id,
                         const TypedValue* keys,
                         TypedValue val);

////////////////////////////////////////////////////////////

/*
 * Shared-only caches
 *
 * Specialization for a shared cache with no keys. In that case, the "key" is
 * just the function's id. As an optimization, we can pre-hash this key and pass
 * it in as a parameter to the getters/setters (the JIT can burn the hashed key
 * into the TC since the FuncId is always statically known).
 */
using SharedOnlyKey = uint64_t;

inline SharedOnlyKey makeSharedOnlyKey(FuncId funcId) {
  static_assert(sizeof(FuncId) == sizeof(uint32_t), "");
  // This is a pseudo-random permutation, so it can be used as both a hash and a
  // key (different FuncIds will never map to the same SharedOnlyKey).
  return folly::hash::twang_mix64(
    (static_cast<uint64_t>(funcId.toInt()) << 32) + funcId.toInt()
  );
}

/*
* Reverses what makeSharedOnlyKey does
* Needs to update if makeSharedOnlyKey updates
*/
inline FuncId unmakeSharedOnlyKey(uint64_t key) {
  auto unmixed_hash = folly::hash::twang_unmix64(key);
  unmixed_hash &= 0xffffffff;
  return FuncId::fromInt(unmixed_hash);
}

const TypedValue* memoCacheGetSharedOnly(const MemoCacheBase* base,
                                   SharedOnlyKey key);
void memoCacheSetSharedOnly(MemoCacheBase*& base,
                            SharedOnlyKey key,
                            TypedValue val);

////////////////////////////////////////////////////////////

// We only have key-count specialized caches for key-counts up to and including
// this. Most of the time you don't need to check this and instead using the
// getter/setter lookup functions defined above.
static constexpr size_t kMemoCacheMaxSpecializedKeys = 6;

}
