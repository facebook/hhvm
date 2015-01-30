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
#ifndef incl_HPHP_REPO_AUTH_TYPE_DEFS_H_
#define incl_HPHP_REPO_AUTH_TYPE_DEFS_H_

#include <cstdint>
#include <vector>
#include <type_traits>
#include <string>

#include "hphp/runtime/base/repo-auth-type.h"

namespace HPHP {

//////////////////////////////////////////////////////////////////////

/*
 * An array type table is a registry of known array types in a
 * program.  In RepoAuthoritative mode, with an optimized repo, it's
 * used for assert opcodes and RepoAuthTypes for arrays.
 *
 * This class has the same semantics for thread safey as primitive
 * types: multiple concurrent threads may read from this table (using
 * the const member functions), but it is not safe to concurrently
 * read and write.
 */
struct ArrayTypeTable {
  struct Builder;

  /*
   * Re-populate an ArrayTypeTable using a builder object.
   *
   * This function invalidates array type ids for array types that
   * weren't built with the supplied builder.  The builder must not be
   * concurrently written to while it's being used to repopulate an
   * ArrayTypeTable.
   */
  void repopulate(const Builder&);

  /*
   * Find an array type description by id.
   */
  const RepoAuthType::Array* lookup(uint32_t id) const {
    assert(id < m_arrTypes.size());
    return m_arrTypes[id];
  }

  template<class SerDe>
  typename std::enable_if<SerDe::deserializing>::type serde(SerDe&);
  template<class SerDe>
  typename std::enable_if<!SerDe::deserializing>::type serde(SerDe&);

private:
  std::vector<const RepoAuthType::Array*> m_arrTypes;
};

//////////////////////////////////////////////////////////////////////

/*
 * Deeper information about array types are represented using this
 * structure.
 */
struct RepoAuthType::Array {
  enum class Tag : uint8_t {
    /*
     * Known size with zero-based contiguous integer keys.
     *
     * Does not currently imply kPackedKind at runtime.
     */
    Packed,
    /*
     * Unknown size, zero-based contiguous integer keys.
     *
     * Does not currently imply kPackedKind at runtime.
     */
    PackedN,
  };
  enum class Empty : uint8_t { Maybe, No };

  /*
   * Actually serialize or deserialize an array type.
   *
   * The normal blob_helper serialization of RepoAuthType::Array*'s
   * just serializes the ids.  These functions are what the
   * ArrayTypeTable uses to (de)serialize the actual data.
   */
  template<class SerDe> static Array* deserialize(SerDe&);
  template<class SerDe> void serialize(SerDe&) const;

  // These are variable-sized heap allocated objects.  We can't copy
  // them around.
  Array(const Array&) = delete;
  Array& operator=(const Array&) = delete;

  /*
   * Every RepoAuthType::Array in the repo has a globally unique id,
   * which is accessible here.  This is a key that can be used to find
   * it in the ArrayTypeTable.
   */
  uint32_t id() const { return m_id; }

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
   * Returns: how many elements are in the array, if it is non-empty.
   *
   * Pre: tag() == Tag::Packed
   */
  uint32_t size() const {
    assert(tag() == Tag::Packed);
    return m_size;
  }

  /*
   * Return the type of the nth element in a packed-like array.
   *
   * Pre: tag() == Tag::Packed
   *      idx < size()
   */
  RepoAuthType packedElem(uint32_t idx) const {
    assert(tag() == Tag::Packed);
    assert(idx < size());
    return types()[idx];
  }

  /*
   * Return a type that is larger than all possible element types.
   *
   * Pre: tag() == Tag::PackedN
   */
  RepoAuthType elemType() const {
    assert(tag() == Tag::PackedN);
    return types()[0];
  }

private:
  Array(Tag tag, Empty emptiness, uint32_t size)
    : m_tag(tag)
    , m_emptiness(emptiness)
    , m_id(-1u)
    , m_size(size)
  {}
  ~Array() {}

  RepoAuthType* types() {
    return reinterpret_cast<RepoAuthType*>(this + 1);
  }
  const RepoAuthType* types() const {
    return reinterpret_cast<const RepoAuthType*>(this + 1);
  }

private:
  friend struct ArrayTypeTable::Builder;

private:
  Tag m_tag;
  Empty m_emptiness;
  uint32_t m_id;
  uint32_t m_size;
};

//////////////////////////////////////////////////////////////////////

/*
 * Creating an ArrayTypeTable (during repo-building time) is done
 * using this builder object.
 *
 * During normal program operation the set of types in the array type
 * table is fixed, and there's no need to do any locking.  However
 * when it's being build (in hhbbc) it's creating them in parallel,
 * and probably will lots of duplicate types.  This object handles
 * coalescing the duplicates and packing them into a format usable for
 * the actual table.
 *
 * Instances of this class may be concurrently accessed for any
 * routines except move-construction and move assignment, or while it
 * is being used in a call to ArrayTypeTable::repopulate.
 */
struct ArrayTypeTable::Builder {
  Builder();
  Builder(const Builder&) = delete;
  Builder& operator=(const Builder&) = delete;
  ~Builder();

  /*
   * Create a new Packed array type descriptor, using this table
   * builder.  May return an existing descriptor if it has the same
   * shape as an array type that already exists.
   *
   * Pre: !types.empty()
   */
  const RepoAuthType::Array* packed(
    RepoAuthType::Array::Empty emptiness,
    const std::vector<RepoAuthType>& types);

  /*
   * Create a new PackedN array type descriptor, using this table
   * builder.  May return an existing descriptor if it has the same
   * shape as a PackedN type that already exists.
   */
  const RepoAuthType::Array* packedn(RepoAuthType::Array::Empty emptiness,
                                     RepoAuthType elemTy);

private:
  const RepoAuthType::Array* insert(RepoAuthType::Array*);

private:
  friend struct ArrayTypeTable;
  struct Impl;
  std::unique_ptr<Impl> m_impl;
};

//////////////////////////////////////////////////////////////////////

/*
 * Create a readable string for this type.
 *
 * This string is also in a parsable format used for the assembler.
 */
std::string show(const RepoAuthType::Array&);

//////////////////////////////////////////////////////////////////////

}

#include "hphp/runtime/base/repo-auth-type-array-inl.h"

#endif
