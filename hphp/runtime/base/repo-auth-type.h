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
#ifndef incl_HPHP_REPO_AUTH_TYPE_H_
#define incl_HPHP_REPO_AUTH_TYPE_H_

#include <limits>
#include <string>

#include <folly/Optional.h>

#include "hphp/util/assertions.h"
#include "hphp/util/compact-tagged-ptrs.h"

#include "hphp/runtime/base/datatype.h"
#include "hphp/runtime/base/runtime-option.h"

namespace HPHP {

//////////////////////////////////////////////////////////////////////

struct StringData;
struct TypedValue;
struct Unit;
struct UnitEmitter;
struct RepoAuthType;

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
    TAG(UncArrKey)                                \
    TAG(ArrKey)                                   \
    TAG(OptUncArrKey)                             \
    TAG(OptArrKey)                                \
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
    TAG(SVec)                                     \
    TAG(OptSVec)                                  \
    TAG(Vec)                                      \
    TAG(OptVec)                                   \
    TAG(SDict)                                    \
    TAG(OptSDict)                                 \
    TAG(Dict)                                     \
    TAG(OptDict)                                  \
    TAG(SKeyset)                                  \
    TAG(OptSKeyset)                               \
    TAG(Keyset)                                   \
    TAG(OptKeyset)                                \
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

  explicit RepoAuthType(Tag tag, const Array* ar) {
    m_data.set(static_cast<uint8_t>(tag), ar);
    assert(mayHaveArrData());
  }

  Tag tag() const { return toResolvedTag(m_data.tag()); }

  bool operator==(RepoAuthType) const;
  bool operator!=(RepoAuthType o) const { return !(*this == o); }
  size_t hash() const;

  const StringData* clsName() const {
    assert(hasClassName());
    return static_cast<const StringData*>(m_data.ptr());
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

  /*
   * Arrays.
   */

   const Array* array() const {
     assert(resolved());
     return static_cast<const Array*>(m_data.ptr());
   }

  // Returns a valid id if there is a corresponding Array* somewhere,
  // or return kInvalidArrayId if Array* is null or if it is unresolved.
  const uint32_t arrayId() const;
  static constexpr auto kInvalidArrayId = std::numeric_limits<uint32_t>::max();

  // Turn an array RAT represented by ID into equivalent array RAT represented
  // by its actual Array*. Should only be called when it is indeed not resolved
  // yet, which should be the place where an RAT is initally loaded from Repo.
  void resolveArray(const Unit& unit);
  void resolveArray(const UnitEmitter& ue);

  bool mayHaveArrData() const {
    switch (tag()) {
    case Tag::OptArr: case Tag::OptSArr: case Tag::Arr: case Tag::SArr:
      return true;
    default:
      return false;
    }
    not_reached();
  }

  // Return true if m_data contains non-null Array*.
  bool hasArrData() const {
    return mayHaveArrData() && resolved() && m_data.ptr();
  }

  /*
   * Serialization/Deserialization
   */

   template<class SerDe> void serde(SerDe& sd) {
     auto t = tag();
     sd(t);

     if (SerDe::deserializing) {
       // mayHaveArrData and hasClassName need to read tag().
       m_data.set(static_cast<uint8_t>(t), nullptr);
     }

     // the 0x40 bit for resolved/unresolved Array* should not be visible
     // to the outside world.
     assert(resolved());

     if (mayHaveArrData()) {
       // serialization
       if (!SerDe::deserializing) {
         // either a valid id for non-null array, or a kInvalidArrayId for null
         uint32_t id = arrayId();
         sd(id);
         return;
       }

       // deserialization
       uint32_t id;
       sd(id);

       // nullptr case, already done
       if (id == kInvalidArrayId) return;

       // RepoAuth case
       if (RuntimeOption::RepoAuthoritative) {
         resolveArrayGlobal(id);
         return;
       }

       // id case
       // this is the only case where we set the highbit
       auto ptr = reinterpret_cast<void*>(id);
       m_data.set(toIdTag(t), ptr);
       return;
     }

     if (hasClassName()) {
       auto c = clsName();
       sd(c);
       m_data.set(static_cast<uint8_t>(t), reinterpret_cast<const void*>(c));
     }
   }

 private:
   void resolveArrayGlobal(uint32_t id);

   #define TAG(x) static_assert((static_cast<uint8_t>(Tag::x) & 0x40) == 0, "");
     REPO_AUTH_TYPE_TAGS
   #undef TAG

   // false if m_data contains an uint32_t id for array type.
   // true otherwise (it may not even be an array type).
   // Note that the 0x80 bit is used by encodeRAT and decodeRAT,
   // and the 0x20 bit is used in the Tag enum.
   const bool resolved() const {
     return (m_data.tag() & 0x40) == 0;
   }
   static uint8_t toIdTag(Tag tag) {
     return static_cast<uint8_t>(tag) | 0x40;
   }
   static Tag toResolvedTag(uint8_t tag) {
     return static_cast<Tag>(tag & ~0x40);
   }

private:
  // This is the type tag (for the lower 6 bits) plus two flag bits (0x80 used
  // by encodeRAT/decodeRAT and 0x40 used by ourselves), plus an optional
  // pointer to a class name (for the obj_* types), or an optional pointer to
  // array information for array types, or alternatively, an optional id to the
  // array information with 0x40 flag set to 1 to differentiate from the pointer
  // case.
  CompactTaggedPtr<const void,uint8_t> m_data;
};

//////////////////////////////////////////////////////////////////////

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
