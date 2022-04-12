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

//////////////////////////////////////////////////////////////////////

}
