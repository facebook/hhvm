/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2016 Facebook, Inc. (http://www.facebook.com)     |
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

#include <string>

#include <folly/Optional.h>

#include "hphp/util/assertions.h"
#include "hphp/util/compact-tagged-ptrs.h"

#include "hphp/runtime/base/datatype.h"

namespace HPHP {

//////////////////////////////////////////////////////////////////////

struct StringData;
struct TypedValue;

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

#define REPO_AUTH_TYPE_TAGS                       \
    TAG(Uninit)                                   \
    TAG(InitNull)                                 \
    TAG(Null)                                     \
    TAG(Int)                                      \
    TAG(OptInt)                                   \
    TAG(Dbl)                                      \
    TAG(OptDbl)                                   \
    TAG(Res)                                      \
    TAG(OptRes)                                   \
    TAG(Bool)                                     \
    TAG(OptBool)                                  \
    TAG(SStr)                                     \
    TAG(OptSStr)                                  \
    TAG(Str)                                      \
    TAG(OptStr)                                   \
    TAG(Obj)                                      \
    TAG(OptObj)                                   \
    TAG(InitUnc)                                  \
    TAG(Unc)                                      \
    TAG(InitCell)                                 \
    TAG(Cell)                                     \
    TAG(Ref)                                      \
    TAG(InitGen)                                  \
    TAG(Gen)                                      \
    /* Types where array() may be non-null. */    \
    TAG(SArr)                                     \
    TAG(OptSArr)                                  \
    TAG(Arr)                                      \
    TAG(OptArr)                                   \
    /* Types where clsName() will be non-null. */ \
    TAG(ExactObj)                                 \
    TAG(SubObj)                                   \
    TAG(OptExactObj)                              \
    TAG(OptSubObj)

  enum class Tag : uint8_t {
#define TAG(x) x,
    REPO_AUTH_TYPE_TAGS
#undef TAG
  };

  explicit RepoAuthType(Tag tag = Tag::Gen, const StringData* sd = nullptr) {
    m_data.set(tag, sd);
    switch (tag) {
    case Tag::OptSubObj: case Tag::OptExactObj:
    case Tag::SubObj: case Tag::ExactObj:
      assert(sd != nullptr);
      break;
    default:
      break;
    }
  }

  explicit RepoAuthType(Tag tag, const Array* ar) {
    m_data.set(tag, ar);
    assert(mayHaveArrData());
  }

  Tag tag() const { return m_data.tag(); }

  bool operator==(RepoAuthType) const;
  bool operator!=(RepoAuthType o) const { return !(*this == o); }
  size_t hash() const;

  const StringData* clsName() const {
    assert(hasClassName());
    return static_cast<const StringData*>(m_data.ptr());
  }

  const Array* array() const {
    assert(mayHaveArrData());
    return static_cast<const Array*>(m_data.ptr());
  }

  bool hasClassName() const {
    switch (tag()) {
    case Tag::SubObj: case Tag::ExactObj:
    case Tag::OptSubObj: case Tag::OptExactObj:
      return true;
    default:
      return false;
    }
    not_reached();
  }

  bool mayHaveArrData() const {
    switch (tag()) {
    case Tag::OptArr: case Tag::OptSArr: case Tag::Arr: case Tag::SArr:
      return true;
    default:
      return false;
    }
    not_reached();
  }

  template<class SerDe>
  void serde(SerDe& sd) {
    auto t = tag();
    sd(t);
    if (SerDe::deserializing) {
      // mayHaveArrData and hasClassName need to read tag().
      m_data.set(t, nullptr);
    }
    auto const vp = [&]() -> const void* {
      if (mayHaveArrData()) {
        auto arr = array();
        sd(arr);
        return arr;
      } else if (hasClassName()) {
        auto c = clsName();
        sd(c);
        return c;
      }
      return nullptr;
    }();
    m_data.set(t, vp);
  }

private:
  // This is the type tag, plus an optional pointer to a class name
  // (for the obj_* types), or an optional pointer to array
  // information for array types.
  CompactTaggedPtr<const void,Tag> m_data;
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
 *
 * Note: this function returns true on array types without checking
 * every element.  This is ok for private properties for now because
 * we don't ever infer inner-array types on properties, but if that
 * changes new mechanisms may be needed.  Relevant to both:
 * TODO(#3696042,#2516227).
 */
bool tvMatchesRepoAuthType(TypedValue, RepoAuthType);

/*
 * Produce a human-readable string from a RepoAuthType.  (Intended for
 * debugging purposes.)
 */
std::string show(RepoAuthType);

//////////////////////////////////////////////////////////////////////

}

#endif
