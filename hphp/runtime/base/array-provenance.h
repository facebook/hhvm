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

#ifndef incl_HPHP_ARRAY_PROVENANCE_H
#define incl_HPHP_ARRAY_PROVENANCE_H

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
    /* known unit + line number */
    Known,
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
     * inside of the JIT corresponding to the PHP location that entered the JIT
     */
    RuntimeLocationPoison,
  };

private:
  static auto constexpr kKindMask = 0x7;

  template <typename T>
  static const char* ptrAndKind(Kind k, const T* filename) {
    auto const ptr = reinterpret_cast<uintptr_t>(filename);
    assertx(!(ptr & kKindMask));
    return reinterpret_cast<const char*>(
      ptr + static_cast<uintptr_t>(k)
    );
  }

  static const char* ptrAndKind(Kind k, std::nullptr_t) {
    return reinterpret_cast<const char*>(
      static_cast<uintptr_t>(k)
    );
  }

  template <typename T>
  static const T* removeKind(const char* filename) {
    return reinterpret_cast<T*>(
      reinterpret_cast<uintptr_t>(filename) & ~kKindMask
    );
  }

  static Kind extractKind(const char* filename) {
    return static_cast<Kind>(
      reinterpret_cast<uintptr_t>(filename) & kKindMask
    );
  }

public:
  constexpr Tag() = default;
  Tag(const StringData* filename, int32_t line);

  static Tag RepoUnion() {
    Tag tag;
    tag.m_filename = ptrAndKind(Kind::UnknownRepo, nullptr);
    tag.m_line = -1;
    return tag;
  }

  static Tag TraitMerge(const StringData* filename) {
    Tag tag;
    tag.m_filename = ptrAndKind(Kind::KnownTraitMerge, filename);
    tag.m_line = -1;
    return tag;
  }

  static Tag LargeEnum(const StringData* classname) {
    Tag tag;
    tag.m_filename = ptrAndKind(Kind::KnownLargeEnum, classname);
    tag.m_line = -1;
    return tag;
  }

  static Tag RuntimeLocation(
    const StringData* filename,
    int line
  ) {
    Tag tag;
    tag.m_filename = ptrAndKind(Kind::RuntimeLocation, filename);
    tag.m_line = line;
    return tag;
  }

  static Tag RuntimeLocation(
    const char* file,
    int line
  ) {
    return RuntimeLocation(makeStaticString(file), line);
  }

  static Tag RuntimeLocationPoison(
    const StringData* filename,
    int line
  ) {
    Tag tag;
    tag.m_filename = ptrAndKind(Kind::RuntimeLocationPoison, filename);
    tag.m_line = line;
    return tag;
  }

  static Tag RuntimeLocationPoison(
    const char* file,
    int line
  ) {
    return RuntimeLocationPoison(makeStaticString(file), line);
  }

  Kind kind() const { return extractKind(m_filename.get()); }
  const StringData* filename() const {
    return removeKind<StringData>(m_filename.get());
  }
  int32_t line() const { return m_line; }

  /* return true if this tag is not default-constructed */
  bool valid() const { return kind() != Kind::Invalid; }

  /*
   * return true if this tag represents a concretely-known location
   * and should be propagated
   *
   * i.e. if this function returns false, we treat an array with this tag
   * as needing a new one if we get the opportunity to retag it
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

  operator bool() const {
    return concrete();
  }

  bool operator==(const Tag& other) const {
    if (kind() != other.kind()) return false;
    switch (kind()) {
    case Kind::Invalid:
    case Kind::UnknownRepo:
      return true;
    case Kind::KnownTraitMerge:
    case Kind::KnownLargeEnum:
      return m_filename == other.m_filename;
    case Kind::Known:
    case Kind::RuntimeLocation:
    case Kind::RuntimeLocationPoison:
      return m_filename == other.m_filename &&
        m_line == other.m_line;
    }
    always_assert(false);
  }
  bool operator!=(const Tag& other) const { return !(*this == other); }

  std::string toString() const;

private:
  /* dumb pointee type because we don't want to break aliasing
   * rules and don't guarantee the actual type of this pointer */
  LowPtr<const char> m_filename{nullptr};
  int32_t m_line{0};
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
 * Attempts to sync VM regs and returns folly::none on failure.
 */
Tag tagFromPC();

/*
 * Create a tag based on the given SrcKey
 */
Tag tagFromSK(SrcKey sk);

/*
 * RAII struct for modifying the behavior of tagFromPC().
 *
 * When this is in effect we use the tag provided instead of computing a
 * backtrace
 */
struct TagOverride {
  explicit TagOverride(Tag tag);
  ~TagOverride();

  TagOverride(TagOverride&&) = delete;
  TagOverride(const TagOverride&) = delete;

  TagOverride& operator=(const TagOverride&) = delete;
  TagOverride& operator=(TagOverride&&) = delete;

private:
  bool m_valid;
  Tag m_saved_tag;
};

#define ARRPROV_HERE() (                        \
    ::HPHP::arrprov::Tag::RuntimeLocation(      \
      __FILE__,                                 \
      __LINE__                                  \
    )                                           \
  )

#define ARRPROV_USE_RUNTIME_LOCATION() \
  ::HPHP::arrprov::TagOverride ap_override(ARRPROV_HERE())

#define ARRPROV_USE_POISONED_LOCATION()           \
  ::HPHP::arrprov::TagOverride ap_override(       \
    ::HPHP::arrprov::Tag::RuntimeLocationPoison(  \
      __FILE__,                                   \
      __LINE__                                    \
    )                                             \
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

/*
 * Space requirement for a tag for `a'.
 */
template<typename A>
size_t tagSize(const A* a) {
  return RO::EvalArrayProvenance && arrayWantsTag(a) ? sizeof(Tag) : 0;
}

/*
 * Get the provenance tag for `a`.
 */
Tag getTag(const ArrayData* a);
Tag getTag(const APCArray* a);
Tag getTag(const AsioExternalThreadEvent* ev);

/*
 * Set mode: insert or emplace.
 *
 * Just controls whether we assert about provenance not already being set: we
 * assert for Insert mode, and not for Emplace.
 */
enum class Mode { Insert, Emplace };

/*
 * Set the provenance tag for `a` to `tag`.
 */
template<Mode mode = Mode::Insert> void setTag(ArrayData* a, Tag tag);
template<Mode mode = Mode::Insert> void setTag(APCArray* a, Tag tag);
template<Mode mode = Mode::Insert> void setTag(AsioExternalThreadEvent* ev, Tag tag);

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
constexpr int64_t DONT_WARN_ON_OBJECTS = 1 << 0;
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
 * Recursively mark the given TV as being a legacy array. This function has the
 * same recursive behavior as tagTvRecursively, except that in addition to
 * raising a notice on encountering an object, it will also raise (up to one)
 * notice on encountering a vec or dict.
 *
 * The extra notice is needed because we won't be able to distinguish between
 * vecs and varrays, or between dicts and darrays, post the HAM flag flip.
 *
 * This method will return a new TypedValue or modify and inc-ref `in`.
 */
TypedValue markTvRecursively(TypedValue in);

/*
 * Mark the given TV as being a legacy array.
 *
 * This method will return a new TypedValue or modify and inc-ref `in`.
 */
TypedValue markTvShallow(TypedValue in);

///////////////////////////////////////////////////////////////////////////////

}}

#endif
