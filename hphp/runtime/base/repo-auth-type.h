/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2014 Facebook, Inc. (http://www.facebook.com)     |
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
#ifndef incl_HPHP_REPO_AUTH_TYPE_H_
#define incl_HPHP_REPO_AUTH_TYPE_H_

#include "folly/Optional.h"

#include "hphp/runtime/vm/hhbc.h"
#include "hphp/util/compact-sized-ptr.h"

namespace HPHP {

struct StringData;
struct TypedValue;

//////////////////////////////////////////////////////////////////////

/*
 * Representation of types inferred statically for RepoAuthoritative
 * mode, for use in runtime data structures.
 *
 * These basically map to a subset of hhbbc's type system.  It's
 * basically the parts found in AssertT subops and AssertObj subops,
 * plus TGen and TInitGen, so the tags are generated from that enum's
 * table.  TODO(#3559108): TInitGen should be part of AssertTOp.
 *
 * This is encoded to be space efficient, so there's a small
 * abstraction layer.
 */
struct RepoAuthType {
  enum class Tag : uint8_t {

#define ASSERTT_OP(x) x,
    ASSERTT_OPS
#undef ASSERTT_OP

    InitGen,
    Gen,

    // Types where clsName() should be non-null
#define ASSERTOBJ_OP(x) x##Obj,
    ASSERTOBJ_OPS
#undef ASSERTOBJ_OP

  };

  explicit RepoAuthType(Tag tag = Tag::Gen, const StringData* sd = nullptr) {
    m_data.set(static_cast<uint8_t>(tag), sd);
    switch (tag) {
    case Tag::OptSubObj: case Tag::OptExactObj:
    case Tag::SubObj: case Tag::ExactObj:
      assert(sd != nullptr);
      break;
    default:
      break;
    }
  }

  Tag tag() const { return static_cast<Tag>(m_data.size() & 0xff); }
  const StringData* clsName() const { return m_data.ptr(); }

  template<class SerDe>
  void serde(SerDe& sd) {
    auto t = tag();
    auto c = clsName();
    sd(t)(c);
    m_data.set(static_cast<uint8_t>(t), c);
  }

private:
  // This is the type tag, plus an optional pointer to a class name
  // (for the obj_* types).
  CompactSizedPtr<const StringData> m_data;
};

//////////////////////////////////////////////////////////////////////

/*
 * Return the DataType corresponding to a RepoAuthType, if the
 * RepoAuthType is refined enough to specify a DataType that
 * corresponds to a php value.  (I.e., this function won't return
 * things like KindOfUncounted.)
 *
 * Returns KindOfString for both SStr and Str.
 *
 * Otherwise returns folly::none.
 */
folly::Optional<DataType> convertToDataType(RepoAuthType);

/*
 * Return whether a TypedValue is a legal match for a RepoAuthType.
 * This can be used for validating that assumptions from static
 * analysis are not violated (for example, by unserializing objects
 * with changed private property types).
 */
bool tvMatchesRepoAuthType(TypedValue, RepoAuthType);

//////////////////////////////////////////////////////////////////////

inline bool operator==(const RepoAuthType& a, const RepoAuthType& b) {
  return a.tag() == b.tag() && a.clsName() == b.clsName();
}

inline bool operator!=(const RepoAuthType& a, const RepoAuthType& b) {
  return !(a == b);
}

//////////////////////////////////////////////////////////////////////

}

#endif
