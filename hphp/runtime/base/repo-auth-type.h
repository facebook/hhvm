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
#include "hphp/runtime/base/types.h"

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

constexpr uint16_t kRATPtrBit = 0x4000;

struct RepoAuthType {
  struct Array;

#define REPO_AUTH_TYPE_TAGS                       \
    TAG(Uninit)                                   \
    TAG(InitNull)                                 \
    TAG(Null)                                     \
    TAG(Int)                                      \
    TAG(OptInt)                                   \
    TAG(UninitInt)                                \
    TAG(Dbl)                                      \
    TAG(OptDbl)                                   \
    TAG(Res)                                      \
    TAG(OptRes)                                   \
    TAG(Bool)                                     \
    TAG(OptBool)                                  \
    TAG(UninitBool)                               \
    TAG(SStr)                                     \
    TAG(OptSStr)                                  \
    TAG(UninitSStr)                               \
    TAG(Str)                                      \
    TAG(OptStr)                                   \
    TAG(UninitStr)                                \
    TAG(Obj)                                      \
    TAG(OptObj)                                   \
    TAG(UninitObj)                                \
    TAG(Func)                                     \
    TAG(OptFunc)                                  \
    TAG(Cls)                                      \
    TAG(OptCls)                                   \
    TAG(ClsMeth)                                  \
    TAG(OptClsMeth)                               \
    TAG(Record)                                   \
    TAG(OptRecord)                                \
    TAG(LazyCls)                                  \
    TAG(OptLazyCls)                               \
    TAG(Num)                                      \
    TAG(OptNum)                                   \
    TAG(InitPrim)                                 \
    TAG(InitUnc)                                  \
    TAG(Unc)                                      \
    TAG(UncArrKey)                                \
    TAG(ArrKey)                                   \
    TAG(OptUncArrKey)                             \
    TAG(OptArrKey)                                \
    TAG(UncStrLike)                               \
    TAG(StrLike)                                  \
    TAG(OptUncStrLike)                            \
    TAG(OptStrLike)                               \
    TAG(UncArrKeyCompat)                          \
    TAG(ArrKeyCompat)                             \
    TAG(OptUncArrKeyCompat)                       \
    TAG(OptArrKeyCompat)                          \
    TAG(NonNull)                                  \
    TAG(InitCell)                                 \
    TAG(Cell)                                     \
    /* Types where array() may be non-null. */    \
    TAG(VecCompat)                                \
    TAG(OptVecCompat)                             \
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
    TAG(SArrLike)                                 \
    TAG(OptSArrLike)                              \
    TAG(ArrLike)                                  \
    TAG(OptArrLike)                               \
    TAG(ArrLikeCompat)                            \
    TAG(OptArrLikeCompat)                         \
    /* Types where clsName() will be non-null. */ \
    TAG(ExactObj)                                 \
    TAG(SubObj)                                   \
    TAG(OptExactObj)                              \
    TAG(OptSubObj)                                \
    TAG(UninitExactObj)                           \
    TAG(UninitSubObj)                             \
    TAG(ExactCls)                                 \
    TAG(SubCls)                                   \
    TAG(OptExactCls)                              \
    TAG(OptSubCls)                                \
    /* Types where recordName() will be non-null. */ \
    TAG(ExactRecord)                              \
    TAG(SubRecord)                                \
    TAG(OptExactRecord)                           \
    TAG(OptSubRecord)                             \


  enum class Tag : uint16_t {
#define TAG(x) x,
    REPO_AUTH_TYPE_TAGS
#undef TAG
  };

  explicit RepoAuthType(Tag tag = Tag::Cell, const StringData* sd = nullptr) {
    m_data.set(static_cast<uint16_t>(tag), sd);
    switch (tag) {
    case Tag::OptSubObj: case Tag::OptExactObj:
    case Tag::UninitSubObj: case Tag::UninitExactObj:
    case Tag::SubObj:    case Tag::ExactObj:
    case Tag::OptSubCls: case Tag::OptExactCls:
    case Tag::SubCls:    case Tag::ExactCls:
    case Tag::ExactRecord:  case Tag::OptExactRecord:
    case Tag::SubRecord: case Tag::OptSubRecord:
      assertx(sd != nullptr);
      break;
    default:
      break;
    }
  }

  explicit RepoAuthType(Tag tag, const Array* ar) {
    m_data.set(static_cast<uint16_t>(tag), ar);
    assertx(mayHaveArrData());
  }

  Tag tag() const { return toResolvedTag(m_data.tag()); }

  bool operator==(RepoAuthType) const;
  bool operator!=(RepoAuthType o) const { return !(*this == o); }
  size_t hash() const;

  /*
   * Record names.
   */

  const StringData* recordName() const {
    assertx(hasRecordName());
    return static_cast<const StringData*>(m_data.ptr());
  }

  bool hasRecordName() const {
    switch (tag()) {
    case Tag::SubRecord: case Tag::ExactRecord:
    case Tag::OptSubRecord: case Tag::OptExactRecord:
      return true;
    default:
      return false;
    }
    not_reached();
  }

  /*
   * Class Names.
   */

  const StringData* clsName() const {
    assertx(hasClassName());
    return static_cast<const StringData*>(m_data.ptr());
  }

  bool hasClassName() const {
    switch (tag()) {
    case Tag::SubObj:    case Tag::ExactObj:
    case Tag::OptSubObj: case Tag::OptExactObj:
    case Tag::UninitSubObj: case Tag::UninitExactObj:
    case Tag::SubCls:    case Tag::ExactCls:
    case Tag::OptSubCls: case Tag::OptExactCls:
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
    assertx(resolved());
    return static_cast<const Array*>(m_data.ptr());
  }

  // Returns a valid id if there is a corresponding Array* somewhere,
  // or return kInvalidArrayId if Array* is null or if it is unresolved.
  const uint32_t arrayId() const;
  static constexpr auto kInvalidArrayId = std::numeric_limits<uint32_t>::max();

  // Turn an array RAT represented by ID into equivalent array RAT represented
  // by its actual Array*. Should only be called when it is indeed not resolved
  // yet, which should be the place where an RAT is initally loaded from Repo.
  void resolveArray(const UnitEmitter& ue);

  bool mayHaveArrData() const {
    switch (tag()) {
    case Tag::OptVec:  case Tag::OptSVec:  case Tag::Vec:  case Tag::SVec:
    case Tag::OptDict: case Tag::OptSDict: case Tag::Dict: case Tag::SDict:
    case Tag::OptKeyset: case Tag::OptSKeyset:
    case Tag::Keyset:    case Tag::SKeyset:
    case Tag::VecCompat: case Tag::OptVecCompat:
    case Tag::OptArrLike: case Tag::OptSArrLike:
    case Tag::ArrLike:    case Tag::SArrLike:
    case Tag::ArrLikeCompat: case Tag::OptArrLikeCompat:
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

  template <class SerDe>
  void serde(SerDe& sd) {
    auto t = tag();
    sd(t);

    if (SerDe::deserializing) {
      // mayHaveArrData and hasClassName need to read tag().
      m_data.set(static_cast<uint16_t>(t), nullptr);
    }

    // the kRATPtrBit bit for resolved/unresolved Array* should not be visible
    // to the outside world.
    assertx(resolved());

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

      // id case
      // this is the only case where we set the kRATPtrBit bit
      auto ptr = reinterpret_cast<const void*>(id);
      m_data.set(toIdTag(t), ptr);
      return;
    }

    if (hasClassName()) {
      // Use a LowStringPtr for the blob encoder/decoder so we take advantage
      // of the litstr table.
      LowStringPtr lc;
      if (!SerDe::deserializing) {
        lc = clsName();
      }
      sd(lc);
      if (SerDe::deserializing) {
        m_data.set(static_cast<uint16_t>(t),
                   reinterpret_cast<const void*>(lc.get()));
      }
    }

    if (hasRecordName()) {
      auto r = recordName();
      sd(r);
      m_data.set(static_cast<uint16_t>(t), reinterpret_cast<const void*>(r));
    }
  }

private:
   #define TAG(x) \
    static_assert((static_cast<uint16_t>(Tag::x) & kRATPtrBit) == 0, "");
     REPO_AUTH_TYPE_TAGS
   #undef TAG

   friend struct ArrayTypeTable;
   friend struct Array;

   template <class LookupFn>
   void doResolve(LookupFn fn) {
     if (!mayHaveArrData() || resolved()) return;

     auto const id = arrayId();
     assertx(id != kInvalidArrayId); // this case is handled in deser time.
     auto const array = fn(id);
     m_data.set(static_cast<uint16_t>(tag()), array);
   }

   // false if m_data contains an uint32_t id for array type.
   // true otherwise (it may not even be an array type).
   // Note that the kRATArrayDataBit bit is used by encodeRAT and decodeRAT,
   // and the 0x20 bit is used in the Tag enum.
   const bool resolved() const {
     return (m_data.tag() & kRATPtrBit) == 0;
   }
   static uint16_t toIdTag(Tag tag) {
     return static_cast<uint16_t>(tag) | kRATPtrBit;
   }
   static Tag toResolvedTag(uint16_t tag) {
     return static_cast<Tag>(tag & ~kRATPtrBit);
   }

private:
  // This is the type tag (for the lower 6 bits) plus two flag bits (
  // kRATArrayDataBit used by encodeRAT/decodeRAT and
  // kRATPtrBit used by ourselves), plus an optional pointer to a class name
  // (for the obj_* types), or an optional pointer to
  // array information for array types, or alternatively, an optional id to the
  // array information with kRATPtrBit flag set to 1 to differentiate from the
  // pointer case.
  CompactTaggedPtr<const void,uint16_t> m_data;
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
