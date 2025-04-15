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

#include <limits>
#include <string>

#include "hphp/util/assertions.h"
#include "hphp/util/compact-tagged-ptrs.h"

#include "hphp/runtime/base/datatype.h"
#include "hphp/runtime/base/repo-auth-type-tags.h"
#include "hphp/runtime/base/types.h"

namespace HPHP {

//////////////////////////////////////////////////////////////////////

struct BlobDecoder;
struct BlobEncoder;
struct FuncEmitter;
struct StringData;
struct TypedValue;
struct Unit;
struct UnitEmitter;

//////////////////////////////////////////////////////////////////////

/*
 * Representation of types inferred statically for RepoAuthoritative
 * mode, for use in runtime data structures, or the bytecode stream
 * (see the AssertRAT{L,Stk} opcodes).
 *
 * This is encoded to be space efficient, so there's a small
 * abstraction layer.
 */

//////////////////////////////////////////////////////////////////////

struct RepoAuthType {
  struct Array;

  enum class Tag : uint8_t {
#define TAG(x, ...) x,
    REPO_AUTH_TYPE_TAGS(TAG)
#undef TAG
  };

  explicit RepoAuthType(Tag tag = Tag::Cell, const StringData* sd = nullptr) {
    assertx(tagHasName(tag) == (sd != nullptr));
    assertx(!tagHasArrData(tag));
    m_data.set(tag, sd);
  }

  explicit RepoAuthType(Tag tag, const Array* ar) {
    assertx(ar != nullptr);
    assertx(tagHasArrData(tag));
    m_data.set(tag, ar);
  }

  Tag tag() const { return m_data.tag(); }

  bool operator==(RepoAuthType) const;
  bool operator!=(RepoAuthType o) const { return !(*this == o); }
  size_t hash() const;
  size_t stableHash() const;

  const StringData* name() const {
    if (!tagHasName(tag())) return nullptr;
    auto const s = static_cast<const StringData*>(m_data.ptr());
    assertx(s);
    return s;
  }

  const Array* array() const {
    if (!tagHasArrData(tag())) return nullptr;
    auto const a = static_cast<const Array*>(m_data.ptr());
    assertx(a != nullptr);
    return a;
  }

  template<typename SerDe> void serde(SerDe&);

  static bool tagHasName(Tag t) {
    switch (t) {
      #define TAG(x, ...) case Tag::x: return true;
      REPO_AUTH_TYPE_TAGS_HAS_NAME(TAG)
      #undef TAG
      default: return false;
    }
  }

  static bool tagHasArrData(Tag t) {
    switch (t) {
      #define TAG(x, ...) case Tag::x: return true;
      REPO_AUTH_TYPE_TAGS_HAS_ARRSPEC(TAG)
      #undef TAG
      default: return false;
    }
  }

  static bool tagIsSubName(Tag t) {
    switch (t) {
      #define TAG(x, ...) case Tag::x: return true;
      REPO_AUTH_TYPE_TAGS_SUB_NAME(TAG)
      #undef TAG
      default: return false;
    }
  }

  static bool tagIsExactName(Tag t) {
    switch (t) {
      #define TAG(x, ...) case Tag::x: return true;
      REPO_AUTH_TYPE_TAGS_EXACT_NAME(TAG)
      #undef TAG
      default: return false;
    }
  }

private:
  friend struct ArrayTypeTable;
  friend struct Array;

  // The type tag, along with optional pointer to RepoAuthType::Array
  // or StringData (depending on tag).
  CompactTaggedPtr<const void,Tag> m_data;
};

//////////////////////////////////////////////////////////////////////

/*
 * Deeper information about array types are represented using this
 * structure.
 */
struct RepoAuthType::Array {
  enum class Tag : uint16_t {
    /*
     * Known size with zero-based contiguous integer keys.
     *
     * May be used with list-like dicts as well as with vecs.
     */
    Tuple,
    /*
     * Unknown size, zero-based contiguous integer keys.
     *
     * May be used with list-like dicts as well as with vecs.
     */
    Packed,
  };
  enum class Empty : uint8_t { Maybe, No };

  template <typename SerDe> static const Array* deserialize(SerDe&);
  template <typename SerDe> void serialize(SerDe&) const;

  // These are variable-sized heap allocated objects.  We can't copy
  // them around.
  Array(const Array&) = delete;
  Array& operator=(const Array&) = delete;

  /*
   * In addition to the specific information about array structure,
   * every array type has a flag for whether the array is possibly
   * empty.
   */
  Empty emptiness() const { return m_emptiness; }

  /*
   * Query which kind of array type this is.
   *
   * Important note: this is not the same as which runtime array kind
   * the array will have.  We may know the shape of an array without
   * knowing whether it may have escalated or not.
   */
  Tag tag() const { return m_tag; }

  /*
   * Hash for this array which will be the same across processes.
   */
  size_t stableHash() const { return m_hash; }

  /*
   * Returns: how many elements are in the array, if it is non-empty.
   *
   * Pre: tag() == Tag::Tuple
   */
  uint32_t size() const {
    assertx(tag() == Tag::Tuple);
    return m_size;
  }

  /*
   * Return the type of the nth element in a packed-like array.
   *
   * Pre: tag() == Tag::Tuple
   *      idx < size()
   */
  RepoAuthType tupleElem(uint32_t idx) const {
    assertx(tag() == Tag::Tuple);
    assertx(idx < size());
    return types()[idx];
  }

  /*
   * Return a type that is larger than all possible element types.
   *
   * Pre: tag() == Tag::Packed
   */
  RepoAuthType packedElems() const {
    assertx(tag() == Tag::Packed);
    return types()[0];
  }

  static const Array* tuple(Empty emptiness,
                            const std::vector<RepoAuthType>& types);

  static const Array* packed(Empty emptiness, RepoAuthType elemTy);

private:
  Array(Tag tag, Empty emptiness, uint32_t size)
    : m_tag{tag}
    , m_emptiness{emptiness}
    , m_size{size}
  {}

  RepoAuthType* types() {
    return reinterpret_cast<RepoAuthType*>(this + 1);
  }
  const RepoAuthType* types() const {
    return reinterpret_cast<const RepoAuthType*>(this + 1);
  }

private:
  Tag m_tag;
  Empty m_emptiness;
  size_t m_hash;
  uint32_t m_size;
};

//////////////////////////////////////////////////////////////////////

/*
 * Return whether a TypedValue is a legal match for a RepoAuthType.
 * This can be used for validating that assumptions from static
 * analysis are not violated (for example, by unserializing objects
 * with changed private property types).
 *
 * Note: this function returns true on array types without checking
 * every element.
 */
bool tvMatchesRepoAuthType(TypedValue, RepoAuthType);

/*
 * Produce a human-readable string from a RepoAuthType.  (Intended for
 * debugging purposes.)
 */
std::string show(RepoAuthType);
std::string show(const RepoAuthType::Array&);

//////////////////////////////////////////////////////////////////////

/*
 * Decode a RepoAuthType in the format expected in opcode streams.
 *
 * If the RepoAuthType carries strings, they are stored as Id's, and
 * how to look up the Id depends on whether we're pulling it out of a
 * UnitEmitter or a Unit, so it's overloaded.
 *
 * The `pc' parameter is expected to be a pointer into a serialized
 * bytecode array, and is advanced by the amount we consumed.
 */
RepoAuthType decodeRAT(const Unit*, const unsigned char*& pc);
RepoAuthType decodeRAT(const UnitEmitter&, const unsigned char*& pc);

/*
 * Return the size of an encoded RepoAuthType.
 *
 * This is for parsing bytecode (when you don't need to actually
 * create the RepoAuthType).
 */
size_t encodedRATSize(const unsigned char* pc);

/*
 * Encode a RepoAuthType into a FuncEmitter's bytecode stream, in the
 * format used by decodeRAT.
 *
 * This function also merges any litstrs into the unit as appropriate.
 */
void encodeRAT(FuncEmitter& fe, UnitEmitter& ue, RepoAuthType rat);

//////////////////////////////////////////////////////////////////////

}
