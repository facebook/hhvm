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

#include "hphp/runtime/base/runtime-option.h"
#include "hphp/runtime/base/static-string-table.h"
#include "hphp/runtime/base/typed-value.h"
#include "hphp/runtime/base/types.h"

#include "hphp/util/low-ptr.h"
#include "hphp/util/rds-local.h"

#include <folly/Format.h>
#include <folly/Optional.h>

namespace HPHP {

struct APCArray;
struct ArrayData;
struct StringData;
struct c_WaitableWaitHandle;
struct AsioExternalThreadEvent;
struct SrcKey;

namespace arrprov {

///////////////////////////////////////////////////////////////////////////////

/*
 * A provenance annotation
 *
 * We store filenames and line numbers rather than units since we need to
 * manipulate these tags during the repo build. Additionally, we also have
 * several tag types denoting explicitly unknown tags: e.g. when a tag is a
 * result of a union of otherwise-identical arrays in the repo build.
 */
struct Tag {
  enum class Kind {
    /* uninitialized */
    Invalid,
    /* result of a union in a repo build */
    UnknownRepo,
    /* lost original line number as a result of trait ${x}init merges */
    KnownTraitMerge,
    /* Dummy tag for all large enums, which we cache as static arrays */
    KnownLargeEnum,
    /* no vmregs are available, filename and line are runtime locations */
    RuntimeLocation,
    /* some piece of the runtime prevented a backtrace from being collected--
     * e.g. the JIT will use this to prevent a tag being assigned to an array
     * in the JIT corresponding to the PHP location that entered the JIT */
    RuntimeLocationPoison,
    /* known unit + line number. Must be the last kind - see name() for why */
    Known,
    /* NOTE: We CANNOT fit another kind here; kind 7 is reserved */
  };

  constexpr Tag() = default;
  Tag(const Func* func, Offset offset);

  static Tag Known(const StringData* filename, int32_t line) {
    return Tag(Kind::Known, filename, line);
  }
  static Tag RepoUnion() {
    return Tag(Kind::UnknownRepo, nullptr);
  }
  static Tag TraitMerge(const StringData* filename) {
    return Tag(Kind::KnownTraitMerge, filename);
  }
  static Tag LargeEnum(const StringData* classname) {
    return Tag(Kind::KnownLargeEnum, classname);
  }
  static Tag RuntimeLocation(const StringData* filename) {
    return Tag(Kind::RuntimeLocation, filename);
  }
  static Tag RuntimeLocationPoison(const StringData* filename) {
    return Tag(Kind::RuntimeLocationPoison, filename);
  }

  /*
   * `name` means different things for different kinds:
   *  - Kind::Known, Kind::TraitMerge: a Hack filename
   *  - Kind::LargeEnum: a Hack enum class
   *  - Kind::RuntimeLocation, Kind::RuntimeLocationPoison: a C++ file/line
   *
   * `line` will be -1 except for Kind::Known, in which case it may be valid.
   */
  Kind kind() const;
  const StringData* name() const;
  int32_t line() const;

  /* Unique key usable for hashing. */
  uint64_t hash() const;

  /* Return true if this tag is not default-constructed. */
  bool valid() const { return *this != Tag{}; }

  /*
   * Return true if this tag represents a concretely-known location
   * and should be propagated.
   *
   * i.e. if this function returns false, we treat an array with this tag
   * as needing a new tag if we get the opportunity to retag it.
   */
  bool concrete() const {
    switch (kind()) {
    case Kind::Invalid: return false;
    case Kind::Known: return true;
    case Kind::UnknownRepo: return false;
    case Kind::KnownTraitMerge: return true;
    case Kind::KnownLargeEnum: return true;
    case Kind::RuntimeLocation: return true;
    case Kind::RuntimeLocationPoison: return false;
    }
    always_assert(false);
  }

  operator bool() const { return concrete(); }

  bool operator==(const Tag& other) const;
  bool operator!=(const Tag& other) const;

  std::string toString() const;

private:
  Tag(Kind kind, const StringData* name, int32_t line = -1);

  /* these are here since we needed to be friends with these types */
  static Tag get(const ArrayData* ad);
  static Tag get(const APCArray* a);
  static Tag get(const AsioExternalThreadEvent* ev);
  static void set(ArrayData* ad, Tag tag);
  static void set(APCArray* a, Tag tag);
  static void set(AsioExternalThreadEvent* ev, Tag tag);

  /* we are just everybody's best friend */
  friend Tag getTag(const ArrayData* a);
  friend Tag getTag(const APCArray* a);
  friend Tag getTag(const AsioExternalThreadEvent* ev);

  friend void setTag(ArrayData* a, Tag tag);
  friend void setTag(APCArray* a, Tag tag);
  friend void setTag(AsioExternalThreadEvent* ev, Tag tag);

  friend void clearTag(ArrayData* ad);
  friend void clearTag(APCArray* a);
  friend void clearTag(AsioExternalThreadEvent* ev);

private:
  uint32_t m_id = 0;
};

/*
 * This is a separate struct so it can live in RDS and not be GC scanned--the
 * actual RDS-local handle is kept in the implementation
 */
struct ArrayProvenanceTable {
  /* The table itself -- allocated in general heap */
  folly::F14FastMap<const void*, Tag> tags;

  /*
   * We never dereference ArrayData*s from this table--so it's safe for the GC
   * to ignore them in this table
   */
  TYPE_SCAN_IGNORE_FIELD(tags);
};

///////////////////////////////////////////////////////////////////////////////

/*
 * Create a tag based on the current PC and unit.
 *
 * Returns an invalid tag if arrprov is off, or if we can't sync the VM regs.
 */
Tag tagFromPC();

/*
 * Create a tag based on `sk`. Returns an invalid tag if arrprov is off.
 */
Tag tagFromSK(SrcKey sk);

/*
 * RAII struct for modifying the behavior of tagFromPC().
 *
 * When this is in effect we use the tag provided instead of computing a
 * backtrace
 */
struct TagOverride {
  enum class ForceTag {};

  explicit TagOverride(Tag tag);
  TagOverride(Tag tag, ForceTag);
  ~TagOverride();

  TagOverride(TagOverride&&) = delete;
  TagOverride(const TagOverride&) = delete;

  TagOverride& operator=(const TagOverride&) = delete;
  TagOverride& operator=(TagOverride&&) = delete;

private:
  bool m_valid;
  Tag m_saved_tag;
};

#define ARRPROV_STR_IMPL(X) #X
#define ARRPROV_STR(X) ARRPROV_STR_IMPL(X)

#define ARRPROV_HERE() ([&]{                            \
    static auto const file = makeStaticString(          \
        __FILE__ ":" ARRPROV_STR(__LINE__));            \
    return ::HPHP::arrprov::Tag::RuntimeLocation(file); \
  }())

#define ARRPROV_HERE_POISON() ([&]{                           \
    static auto const file = makeStaticString(                \
        __FILE__ ":" ARRPROV_STR(__LINE__));                  \
    return ::HPHP::arrprov::Tag::RuntimeLocationPoison(file); \
  }())

#define ARRPROV_USE_RUNTIME_LOCATION() \
  ::HPHP::arrprov::TagOverride ap_override(ARRPROV_HERE())

#define ARRPROV_USE_POISONED_LOCATION() \
  ::HPHP::arrprov::TagOverride ap_override(ARRPROV_HERE_POISON())

// Set tag even if provenanance is currently disabled.
// This is useful for runtime initialization and config parsing code, where
// Eval.ArrayProvenance may change as result of config parsing.
#define ARRPROV_USE_RUNTIME_LOCATION_FORCE()      \
  ::HPHP::arrprov::TagOverride ap_override(       \
      ARRPROV_HERE(),                             \
      ::HPHP::arrprov::TagOverride::ForceTag{}    \
  )

#define ARRPROV_USE_VMPC() \
  ::HPHP::arrprov::TagOverride ap_override({})

/*
 * Whether `a` admits a provenance tag.
 *
 * Depends on the ArrProv.* runtime options.
 */
bool arrayWantsTag(const ArrayData* a);
bool arrayWantsTag(const APCArray* a);
bool arrayWantsTag(const AsioExternalThreadEvent* a);

auto constexpr kAPCTagSize = 8;

/*
 * Get the provenance tag for `a`.
 */
Tag getTag(const ArrayData* a);
Tag getTag(const APCArray* a);
Tag getTag(const AsioExternalThreadEvent* ev);

/*
 * Set the provenance tag for `a` to `tag`.
 */
void setTag(ArrayData* a, Tag tag);
void setTag(APCArray* a, Tag tag);
void setTag(AsioExternalThreadEvent* ev, Tag tag);

/*
 * Clear a tag for a released array---only call this if the array is henceforth
 * unreachable or no longer of a kind that accepts provenance tags
 */
void clearTag(ArrayData* ad);
void clearTag(APCArray* a);
void clearTag(AsioExternalThreadEvent* ev);

/*
 * Invalidates the old tag on the provided array and reassigns one from the
 * current PC, if the array still admits a tag.
 *
 * If the array no longer admits a tag, but has one set, clears it.
 *
 */
void reassignTag(ArrayData* ad);

/*
 * Produce a static array with the given provenance tag.
 *
 * If an invalid tag is provided, we attempt to make one from vmpc(), and
 * failing that we just return the input array.
 */
ArrayData* tagStaticArr(ArrayData* ad, Tag tag = {});

///////////////////////////////////////////////////////////////////////////////

namespace TagTVFlags {
}

/*
 * Recursively tag the given TypedValue, tagging it (if necessary), and if it is
 * an array-like, recursively tagging of its values (if necessary).
 *
 * This function will tag values within, say, a dict, even if it doesn't tag the
 * dict itself. This behavior is important because it allows us to implement
 * provenance for (nested) static arrays in ProvenanceSkipFrame functions.
 *
 * The only other type that can contain nested arrays are objects. This function
 * does NOT tag through objects; instead, it raises notices that it found them.
 * (It will emit at most one notice per call.)
 *
 * This method will return a new TypedValue or modify and inc-ref `in`.
 */
TypedValue tagTvRecursively(TypedValue in, int64_t flags = 0);

/*
 * Recursively mark/unmark the given TV as being a legacy array.
 * This function has the same recursive behavior as tagTvRecursively,
 * except that in addition to raising a notice on encountering an object,
 * it will also raise (up to one) notice on encountering a vec or dict.
 *
 * The extra notice is needed because we won't be able to distinguish between
 * vecs and varrays, or between dicts and darrays, post the HAM flag flip.
 *
 * This method will return a new TypedValue or modify and inc-ref `in`.
 */
TypedValue markTvRecursively(TypedValue in, bool legacy);

/*
 * Mark/unmark the given TV as being a legacy array.
 *
 * This method will return a new TypedValue or modify and inc-ref `in`.
 */
TypedValue markTvShallow(TypedValue in, bool legacy);

///////////////////////////////////////////////////////////////////////////////

}}

