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
#ifndef incl_HHBBC_TYPE_SYSTEM_H_
#define incl_HHBBC_TYPE_SYSTEM_H_

#include <cstdint>
#include <vector>
#include <utility>

#include <folly/Optional.h>

#include "hphp/util/copy-ptr.h"
#include "hphp/util/low-ptr.h"

#include "hphp/runtime/base/repo-auth-type.h"
#include "hphp/runtime/base/repo-auth-type-array.h"
#include "hphp/runtime/base/array-provenance.h"

#include "hphp/hhbbc/array-like-map.h"
#include "hphp/hhbbc/index.h"
#include "hphp/hhbbc/misc.h"

namespace HPHP { namespace HHBBC {

struct Type;

//////////////////////////////////////////////////////////////////////

/*
 * Type system.
 *
 * Here's an unmaintainable ascii-art diagram:
 *
 *                      Top
 *                       |
 *                 +-----+
 *                 |     |             InitCell := Cell - Uninit
 *                Cls    |                   ?X := X + InitNull
 *                 |     |
 *              Cls<=c  Cell
 *                 |     |
 *              Cls=c    +-------------+--------+-------+-------+------------+
 *                       |             |        |       |       |            |
 *                      Unc            |        |      Obj     Res         Record
 *                       | \           |        |      /  \                  |
 *                       |  \          |        |  Obj<=c Obj<=WaitHandle Record<=c
 *                     Prim  \         |        |    |       |               |
 *                     / |   InitUnc   |        |  Obj=c   WaitH<T>       Record=c
 *                    /  |   /  |  |   |        |
 *                   /   |  /   |  |   |        |
 *                  /    | /    |  |   |        |
 *                 /     |/     |  |   |        |
 *              Null  InitPrim  |  |   |        |
 *             /  |    / |      |  |  Arr      Str
 *            /   |   /  |      |  |  / \      / \
 *      Uninit  InitNull |      | SArr  ...   /  CStr
 *                       |      |  |         /
 *                       |      | ...       /
 *                       |      |          /
 *                       |      \         /
 *                       |       \       /
 *                       |        \     /
 *                       |         \   /
 *                       |          SStr
 *                       |           |
 *                       |         SStr=s
 *                       |
 *                       +----------+
 *                       |          |
 *                      Bool       Num
 *                      /  \       |  \
 *                   True  False  Int  Dbl
 *                                 |    |
 *                               Int=n Dbl=n
 *
 * Some notes on some of the basic types:
 *
 *   {Init,}Prim
 *
 *       "Primitive" types---these can be represented in a TypedValue without a
 *       pointer to the heap.
 *
 *   {Init,}Unc
 *
 *       "Uncounted" types---values of these types don't require reference
 *       counting.
 *
 *   WaitH<T>
 *
 *       A WaitHandle that is known will either return a value of type T from
 *       its join() method (or Await), or else throw an exception.
 *
 * Array types:
 *
 *   Arrays are divided along two dimensions: counted or uncounted, and empty
 *   or non-empty.  Unions of either are allowed.  The naming convention is
 *   {S,C,}Arr{N,E,}, where leaving out either bit means it's unknown along
 *   that dimension.  All arrays are subtypes of the Arr type.  The lattice
 *   here looks like this:
 *
 *                         Arr
 *                          |
 *                  +----+--+--+---+
 *                  |    |     |   |
 *                  |  SArr   CArr |
 *                  |   |      |   |
 *              +-------+---------------+
 *              |   |          |   |    |
 *              |  ArrN  +-----+  ArrE  |
 *              | /   \  |     | /   \  |
 *             SArrN  CArrN   CArrE  SArrE
 *
 *   NOTE: Having SArr be a sibling of CArr is problematic. There is
 *   an assumption that types in the index only ever get more refined,
 *   but eg we might "know" that a type is CArr early on (because eg
 *   we stored a value to it, thus modifying it), but later we might
 *   know the value both before and after the modification, and just
 *   replace each with a static array. In fact, this is almost
 *   guaranteed to happen when building arrays from constants (not
 *   literals), because on the first pass we won't know the constant
 *   values, and will produce a CArr, but eventually we could figure
 *   them out, and produce an SArr. This means that in practice, we
 *   can't use CArr anywhere, because it might be improved to SArr,
 *   which is not a subtype. Even if we only generated CArr after all
 *   modifications are done (eg during the insert-assertions phase) we
 *   could still run into problems where we annotate an array as CArr
 *   but jit time analysis/optimization was able to produce a static
 *   array - so it doesn't appear to be useful, except as a superclass
 *   of SArr.
 *
 *   "Specialized" array types may be found as subtypes of any of the above
 *   types except SArrE and CArrE, or of optional versions of the above types
 *   (e.g. as a subtype of ?ArrN).  The information about additional structure
 *   is dispayed in parenthesis, and probably best explained by some examples:
 *
 *     SArrN(Bool,Int)
 *
 *         Tuple-like static two-element array, with integer keys 0 and 1,
 *         containing a Bool and an Int with unknown values.
 *
 *     Arr(Int,Dbl)
 *
 *         An array of unknown countedness that is either empty, or a
 *         tuple-like array with two elements of types Int and Dbl.
 *
 *     CArrN([Bool])
 *
 *         Non-empty reference counted array with contiguous zero-based integer
 *         keys, unknown size, values all are subtypes of Bool.
 *
 *     ArrN(x:Int,y:Int)
 *
 *         Struct-like array with known fields "x" and "y" that have Int
 *         values, and no other fields.  Struct-like arrays always have known
 *         string keys, and the type contains only those array values with
 *         exactly the given key set, in the order specified.
 *
 *     Arr([SStr:InitCell])
 *
 *         Possibly empty map-like array with unknown keys, but all
 *         non-reference counted strings, all values InitCell.  In this case
 *         the array itself may or may not be static.
 *
 *         Note that struct-like arrays will be subtypes of map-like arrays
 *         with string keys.
 *
 *     ArrN([Int:InitPrim])
 *
 *         Map-like array with only integer keys (not-necessarily contiguous)
 *         and values that are all subtypes of InitPrim.  Note that the keys
 *         *may* be contiguous integers, so for example Arr([InitPrim]) <:
 *         Arr([Int => InitPrim]).
 *
 *     Arr([ArrKey:InitCell])
 *
 *         Map-like array with either integer or string keys, and InitCell
 *         values, or empty.  Essentially this is the most generic array that
 *         can't contain php references.
 */

//////////////////////////////////////////////////////////////////////

enum trep : uint64_t {
  BBottom   = 0,

  BUninit   = 1ULL << 0,
  BInitNull = 1ULL << 1,
  BFalse    = 1ULL << 2,
  BTrue     = 1ULL << 3,
  BInt      = 1ULL << 4,
  BDbl      = 1ULL << 5,
  BSStr     = 1ULL << 6,  // static string
  BCStr     = 1ULL << 7,  // counted string
  BFunc     = 1ULL << 8,

  BSPArrE   = 1ULL << 9, // static empty "plain" array
  BCPArrE   = 1ULL << 10, // counted empty "plain" array
  BSPArrN   = 1ULL << 11, // static non-empty "plain" array
  BCPArrN   = 1ULL << 12ULL, // counted non-empty "plain array"

  BSVArrE   = 1ULL << 13, // static empty varray
  BCVArrE   = 1ULL << 14, // counted empty varray
  BSVArrN   = 1ULL << 15, // static non-empty varray
  BCVArrN   = 1ULL << 16, // counted non-empty varray

  BSDArrE   = 1ULL << 17, // static empty darray
  BCDArrE   = 1ULL << 18, // counted empty darray
  BSDArrN   = 1ULL << 19, // static non-empty darray
  BCDArrN   = 1ULL << 20, // counted non-empty darray

  BClsMeth  = 1ULL << 21,

  BObj      = 1ULL << 22,
  BRes      = 1ULL << 23,
  BCls      = 1ULL << 24,

  BSVecE    = 1ULL << 25, // static empty vec
  BCVecE    = 1ULL << 26, // counted empty vec
  BSVecN    = 1ULL << 27, // static non-empty vec
  BCVecN    = 1ULL << 28, // counted non-empty vec
  BSDictE   = 1ULL << 29, // static empty dict
  BCDictE   = 1ULL << 30, // counted empty dict
  BSDictN   = 1ULL << 31, // static non-empty dict
  BCDictN   = 1ULL << 32, // counted non-empty dict
  BSKeysetE = 1ULL << 33, // static empty keyset
  BCKeysetE = 1ULL << 34, // counted empty keyset
  BSKeysetN = 1ULL << 35, // static non-empty keyset
  BCKeysetN = 1ULL << 36, // counted non-empty keyset

  BRecord  = 1ULL << 37,

  BSPArr    = BSPArrE | BSPArrN,
  BCPArr    = BCPArrE | BCPArrN,
  BPArrE    = BSPArrE | BCPArrE,
  BPArrN    = BSPArrN | BCPArrN,
  BPArr     = BPArrE  | BPArrN,

  BSVArr    = BSVArrE | BSVArrN,
  BCVArr    = BCVArrE | BCVArrN,
  BVArrE    = BSVArrE | BCVArrE,
  BVArrN    = BSVArrN | BCVArrN,
  BVArr     = BVArrE  | BVArrN,

  BSDArr    = BSDArrE | BSDArrN,
  BCDArr    = BCDArrE | BCDArrN,
  BDArrE    = BSDArrE | BCDArrE,
  BDArrN    = BSDArrN | BCDArrN,
  BDArr     = BDArrE  | BDArrN,

  BSArrE    = BSPArrE | BSVArrE | BSDArrE,
  BCArrE    = BCPArrE | BCVArrE | BCDArrE,
  BSArrN    = BSPArrN | BSVArrN | BSDArrN,
  BCArrN    = BCPArrN | BCVArrN | BCDArrN,

  BNull     = BUninit | BInitNull,
  BBool     = BFalse | BTrue,
  BNum      = BInt | BDbl,
  BStr      = BSStr | BCStr,
  BSArr     = BSArrE | BSArrN,
  BCArr     = BCArrE | BCArrN,
  BArrE     = BSArrE | BCArrE,
  BArrN     = BSArrN | BCArrN,   // may have value / data
  BArr      = BArrE | BArrN,
  BSVec     = BSVecE | BSVecN,
  BCVec     = BCVecE | BCVecN,
  BVecE     = BSVecE | BCVecE,
  BVecN     = BSVecN | BCVecN,
  BVec      = BVecE | BVecN,
  BSDict    = BSDictE | BSDictN,
  BCDict    = BCDictE | BCDictN,
  BDictE    = BSDictE | BCDictE,
  BDictN    = BSDictN | BCDictN,
  BDict     = BDictE | BDictN,
  BSKeyset  = BSKeysetE | BSKeysetN,
  BCKeyset  = BCKeysetE | BCKeysetN,
  BKeysetE  = BSKeysetE | BCKeysetE,
  BKeysetN  = BSKeysetN | BCKeysetN,
  BKeyset   = BKeysetE | BKeysetN,

  // Nullable types.
  BOptTrue     = BInitNull | BTrue,
  BOptFalse    = BInitNull | BFalse,
  BOptBool     = BInitNull | BBool,
  BOptInt      = BInitNull | BInt,       // may have value
  BOptDbl      = BInitNull | BDbl,       // may have value
  BOptNum      = BInitNull | BNum,
  BOptSStr     = BInitNull | BSStr,      // may have value
  BOptCStr     = BInitNull | BCStr,
  BOptStr      = BInitNull | BStr,
  BOptSArrE    = BInitNull | BSArrE,
  BOptCArrE    = BInitNull | BCArrE,
  BOptSArrN    = BInitNull | BSArrN,     // may have value / data
  BOptCArrN    = BInitNull | BCArrN,     // may have value / data
  BOptSArr     = BInitNull | BSArr,      // may have value / data
  BOptCArr     = BInitNull | BCArr,      // may have value / data
  BOptArrE     = BInitNull | BArrE,      // may have value / data
  BOptArrN     = BInitNull | BArrN,      // may have value / data
  BOptArr      = BInitNull | BArr,       // may have value / data
  BOptObj      = BInitNull | BObj,       // may have data
  BOptRes      = BInitNull | BRes,
  BOptFunc     = BInitNull | BFunc,
  BOptCls      = BInitNull | BCls,
  BOptClsMeth  = BInitNull | BClsMeth,
  BOptSVecE    = BInitNull | BSVecE,
  BOptCVecE    = BInitNull | BCVecE,
  BOptSVecN    = BInitNull | BSVecN,
  BOptCVecN    = BInitNull | BCVecN,
  BOptSVec     = BInitNull | BSVec,
  BOptCVec     = BInitNull | BCVec,
  BOptVecE     = BInitNull | BVecE,
  BOptVecN     = BInitNull | BVecN,
  BOptVec      = BInitNull | BVec,
  BOptSDictE   = BInitNull | BSDictE,
  BOptCDictE   = BInitNull | BCDictE,
  BOptSDictN   = BInitNull | BSDictN,
  BOptCDictN   = BInitNull | BCDictN,
  BOptSDict    = BInitNull | BSDict,
  BOptCDict    = BInitNull | BCDict,
  BOptDictE    = BInitNull | BDictE,
  BOptDictN    = BInitNull | BDictN,
  BOptDict     = BInitNull | BDict,
  BOptSKeysetE = BInitNull | BSKeysetE,
  BOptCKeysetE = BInitNull | BCKeysetE,
  BOptSKeysetN = BInitNull | BSKeysetN,
  BOptCKeysetN = BInitNull | BCKeysetN,
  BOptSKeyset  = BInitNull | BSKeyset,
  BOptCKeyset  = BInitNull | BCKeyset,
  BOptKeysetE  = BInitNull | BKeysetE,
  BOptKeysetN  = BInitNull | BKeysetN,
  BOptKeyset   = BInitNull | BKeyset,
  BOptRecord   = BInitNull | BRecord,

  BOptSPArrE   = BInitNull | BSPArrE,
  BOptCPArrE   = BInitNull | BCPArrE,
  BOptSPArrN   = BInitNull | BSPArrN,
  BOptCPArrN   = BInitNull | BCPArrN,
  BOptSPArr    = BInitNull | BSPArr,
  BOptCPArr    = BInitNull | BCPArr,
  BOptPArrE    = BInitNull | BPArrE,
  BOptPArrN    = BInitNull | BPArrN,
  BOptPArr     = BInitNull | BPArr,

  BOptSVArrE   = BInitNull | BSVArrE,
  BOptCVArrE   = BInitNull | BCVArrE,
  BOptSVArrN   = BInitNull | BSVArrN,
  BOptCVArrN   = BInitNull | BCVArrN,
  BOptSVArr    = BInitNull | BSVArr,
  BOptCVArr    = BInitNull | BCVArr,
  BOptVArrE    = BInitNull | BVArrE,
  BOptVArrN    = BInitNull | BVArrN,
  BOptVArr     = BInitNull | BVArr,

  BOptSDArrE   = BInitNull | BSDArrE,
  BOptCDArrE   = BInitNull | BCDArrE,
  BOptSDArrN   = BInitNull | BSDArrN,
  BOptCDArrN   = BInitNull | BCDArrN,
  BOptSDArr    = BInitNull | BSDArr,
  BOptCDArr    = BInitNull | BCDArr,
  BOptDArrE    = BInitNull | BDArrE,
  BOptDArrN    = BInitNull | BDArrN,
  BOptDArr     = BInitNull | BDArr,

  BUncArrKey    = BInt | BSStr,
  BArrKey       = BUncArrKey | BCStr,
  BOptUncArrKey = BInitNull | BUncArrKey,
  BOptArrKey    = BInitNull | BArrKey,

  BFuncOrCls    = BFunc | BCls,
  BOptFuncOrCls = BInitNull | BFuncOrCls,

  BStrLike    = BFuncOrCls | BStr,
  BUncStrLike = BFuncOrCls | BSStr,

  BOptStrLike    = BInitNull | BStrLike,
  BOptUncStrLike = BInitNull | BUncStrLike,

  BVArrLike   = BClsMeth | BVArr,
  BVArrLikeSA = BClsMeth | BSVArr,

  BOptVArrLike = BInitNull | BVArrLike,
  BOptVArrLikeSA = BInitNull | BVArrLikeSA,

  BVecLike   = BClsMeth | BVec,
  BVecLikeSA = BClsMeth | BSVec,

  BOptVecLike    = BInitNull | BVecLike,
  BOptVecLikeSA  = BInitNull | BVecLikeSA,

  BPArrLike   = BClsMeth | BArr,
  BPArrLikeSA = BClsMeth | BSArr,

  BOptPArrLike   = BInitNull | BPArrLike,
  BOptPArrLikeSA = BInitNull | BPArrLikeSA,

  BInitPrim = BInitNull | BBool | BNum | BFunc | BCls |
              (use_lowptr ? BClsMeth : 0),

  BPrim     = BInitPrim | BUninit,
  BInitUnc  = BInitPrim | BSStr | BSArr | BSVec | BSDict | BSKeyset,
  BUnc      = BInitUnc | BUninit,
  BInitCell = BInitNull | BBool | BInt | BDbl | BStr | BArr | BObj | BRes |
              BVec | BDict | BKeyset | BFunc | BCls | BClsMeth | BRecord,
  BCell     = BUninit | BInitCell,

  BTop      = static_cast<uint64_t>(-1),
};

constexpr trep operator~(trep a) {
  return static_cast<trep>(~static_cast<int64_t>(a));
}

constexpr trep operator&(trep a, trep b) {
  return static_cast<trep>(static_cast<int64_t>(a) & b);
}

constexpr trep operator|(trep a, trep b) {
  return static_cast<trep>(static_cast<int64_t>(a) | b);
}

constexpr const trep& operator&=(trep&a, trep b) {
  a = a & b;
  return a;
}

constexpr const trep& operator|=(trep&a, trep b) {
  a = a | b;
  return a;
}

// Useful constants. Don't put them in the enum itself, because they
// can't actually occur, but are convenient masks.
constexpr auto BArrLikeE = BArrE | BVecE | BDictE | BKeysetE;
constexpr auto BArrLikeN = BArrN | BVecN | BDictN | BKeysetN;
constexpr auto BArrLike = BArrLikeE | BArrLikeN;
constexpr auto BSArrLike = BSArr | BSVec | BSDict | BSKeyset;
constexpr auto BSArrLikeE = BSArrE | BSVecE | BSDictE | BSKeysetE;

#define DATATAGS                                                \
  DT(Str, SString, sval)                                        \
  DT(Int, int64_t, ival)                                        \
  DT(Dbl, double, dval)                                         \
  DT(ArrLikeVal, SArray, aval)                                  \
  DT(Obj, DObj, dobj)                                           \
  DT(Cls, DCls, dcls)                                           \
  DT(Record, DRecord, drec)                                     \
  DT(ArrLikePacked, copy_ptr<DArrLikePacked>, packed)           \
  DT(ArrLikePackedN, copy_ptr<DArrLikePackedN>, packedn)        \
  DT(ArrLikeMap, copy_ptr<DArrLikeMap>, map)                    \
  DT(ArrLikeMapN, copy_ptr<DArrLikeMapN>, mapn)

// Tag for what kind of specialized data a Type object has.
enum class DataTag : uint8_t {
  None,
#define DT(name,...) name,
  DATATAGS
#undef DT
};

//////////////////////////////////////////////////////////////////////

/*
 * Information about a class type.  The class is either exact or a
 * subtype of the supplied class.
 */
struct DCls {
  enum Tag : uint16_t { Exact, Sub };

  DCls(Tag type, res::Class cls)
    : type(type)
    , cls(cls)
  {}

  Tag type;
  bool isCtx = false;
  res::Class cls;
};

/*
 * Information about a specific object type.  The class is either
 * exact or a subtype of the supplied class.
 *
 * If the class is WaitHandle, we can also carry a type that joining
 * the wait handle will produce.
 */
struct DObj {
  enum Tag : uint16_t { Exact, Sub };

  DObj(Tag type, res::Class cls)
    : type(type)
    , cls(cls)
  {}

  Tag type;
  bool isCtx = false;
  res::Class cls;
  copy_ptr<Type> whType;
};

/*
 * Information about a specific record type.  The record type is either
 * exact or a subtype of the supplied record.
 */
struct DRecord {
  enum Tag : uint16_t { Exact, Sub };

  DRecord(Tag type, res::Record rec)
    : type(type)
    , rec(rec)
  {}

  Tag type;
  res::Record rec;
};

struct DArrLikePacked;
struct DArrLikePackedN;
struct DArrLikeMap;
struct DArrLikeMapN;
using MapElems = ArrayLikeMap<TypedValue>;
struct ArrKey;
struct IterTypes;

//////////////////////////////////////////////////////////////////////

/*
 * A provenance tag as tracked on a DArrLike{Packed,Map} (and, at runtime, on
 * various kinds of arrays).  The provenance tag on an array contains a file
 * and line nubmer where that array is "from" (ideally, where the array was
 * allocated---for static arrays the srcloc where the array is first
 * referenced).
 *
 * This is tracked here in hhbbc because we both manipulate and create new
 * static arrays.
 *
 * If the runtime option EvalArrayProvenance is not set, all ProvTags should be
 * equal to ProvTag::Top.
 *
 * ProvTag::Top means the array type could have a provenance tag from anywhere,
 * or no tag at all. (i.e. its provenance is unknown completely, a.k.a. NoTag).
 *
 * ProvTag::Some means the array type has one of several known tags and will
 * be written out with a RepoUnion tag
 *
 * This information forms a sublattice like:
 *
 *                                     top
 *           __________________________/ \_________________
 *          /                                              \
 *       some tag                                           |
 *    ____/|\___________                                    |
 *   /     |            \                                   |
 *  t_1   t_2     ...   t_n (specific arrprov::Tag's)     no tag
 *   \____ | ___________/___________________________________/
 *        \|/
 *       bottom (unrepresentable)
 *
 * If we would produce a Bottom provenance tag, (i.e. in intersectProvTag) we
 * widen the result to ProvTag::Top.
 */
struct ProvTag {
  ProvTag() {}
  /* implicit */ ProvTag(const arrprov::Tag& t)
    : m_tag(t) {}

  enum class KnownEmpty {};
  /* implicit */ ProvTag(KnownEmpty)
    : m_knownEmpty(true)
    , m_tag() {}

  static ProvTag Top;
  static ProvTag NoTag;
  static ProvTag SomeTag;

  static ProvTag FromSArr(SArray a) {
    assertx(RO::EvalArrayProvenance);
    // It would be nice to assert a->isStatic() here, but, of course, SArrays
    // (like the ones representing bytecode immediates) are not always static
    // because we muck with them during various optimization passes.
    return arrprov::getTag(a);
  }

  /*
   * Do we have a known provenance tag.
   */
  bool valid() const {
    return !m_knownEmpty && m_tag.valid();
  }

  arrprov::Tag get() const { return m_tag; }

  bool operator==(const ProvTag& o) const {
    return m_knownEmpty == o.m_knownEmpty && m_tag == o.m_tag;
  }
  bool operator!=(const ProvTag& o) const {
    return m_knownEmpty != o.m_knownEmpty || m_tag != o.m_tag;
  }

private:
  bool m_knownEmpty{false};
  arrprov::Tag m_tag{};
};

constexpr auto kProvBits = BVec | BDict | BVArr | BDArr;

//////////////////////////////////////////////////////////////////////

enum class Emptiness {
  Empty,
  NonEmpty,
  Maybe
};

enum class ThrowMode {
  None,
  MaybeMissingElement,
  MaybeBadKey,
  MissingElement,
  BadOperation,
};

//////////////////////////////////////////////////////////////////////

struct Type {
  Type() : m_bits(BTop) {
    assert(checkInvariants());
  }
  explicit Type(trep t) : m_bits(t) {
    assert(checkInvariants());
  }

  Type(const Type&) noexcept;
  Type(Type&&) noexcept;
  Type& operator=(const Type&) noexcept;
  Type& operator=(Type&&) noexcept;
  ~Type() noexcept;

  /*
   * Exact equality or inequality of types, and hashing.
   */
  bool operator==(const Type& o) const;
  bool operator!=(const Type& o) const { return !(*this == o); }
  size_t hash() const;

  const Type& operator |= (const Type& other);
  const Type& operator |= (Type&& other);
  const Type& operator &= (const Type& other);
  const Type& operator &= (Type&& other);

  /*
   * Returns true if this type is equivalently refined, more refined or strictly
   * more refined than `o`.  This is similar to the `==` and subtype operations
   * defined below, except they take into account if a type is tagged as a
   * context.
   */
  bool equivalentlyRefined(const Type& o) const;
  bool moreRefined(const Type& o) const;
  bool strictlyMoreRefined(const Type& o) const;

  /*
   * Returns true if this type is definitely going to be a subtype or a strict
   * subtype of `o' at runtime.  If this function returns false, this may
   * still be a subtype of `o' at runtime, it just may not be known.
   */
  bool subtypeOf(const Type& o) const;
  bool strictSubtypeOf(const Type& o) const;

  /*
   * Similar, but only check the trep (same as subtypeOf(Type{bits}),
   * but cheaper).
   */
  bool subtypeOf(trep bits) const { return (m_bits & bits) == m_bits; }
  bool subtypeOrNull(trep bits) const { return subtypeOf(bits | BNull); }

  /*
   * Subtype of any of the list of types.
   */
  template<class... Types>
  bool subtypeOfAny(const Type& t, Types... ts) const {
    return subtypeOf(t) || subtypeOfAny(ts...);
  }
  bool subtypeOfAny() const { return false; }

  template<bool contextSensitive>
  bool equivImpl(const Type& o) const;
  template<bool contextSensitive>
  bool subtypeOfImpl(const Type& o) const;

  /*
   * Returns whether there are any values of this type that are also
   * values of the type `o'.
   * When this function returns false, it is known that this type
   * must not be in any subtype relationship with the argument Type 'o'.
   * When true is returned the two types may still be unrelated but it is
   * not possible to tell.
   * Essentially this function can conservatively return true but must be
   * precise when returning false.
   */
  bool couldBe(const Type& o) const;
  bool couldBe(trep bits) const { return m_bits & bits; }

  /*
   * Could-be any of the list of types.
   */
  template<class... Types>
  bool couldBeAny(const Type& t, Types... ts) const {
    return couldBe(t) || couldBeAny(ts...);
  }
  bool couldBeAny() const { return false; }

  struct ArrayCat {
    enum { None, Empty, Packed, Struct, Mixed } cat;
    bool hasValue;
  };

private:
  friend folly::Optional<int64_t> arr_size(const Type& t);
  friend ArrayCat categorize_array(const Type& t);
  friend CompactVector<LSString> get_string_keys(const Type& t);
  friend Type wait_handle(const Index&, Type);
  friend bool is_specialized_wait_handle(const Type&);
  friend bool is_specialized_array_like(const Type& t);
  friend bool is_specialized_obj(const Type&);
  friend bool is_specialized_record(const Type&);
  friend bool is_specialized_cls(const Type&);
  friend bool is_specialized_record(const Type&);
  friend bool is_specialized_string(const Type&);
  friend Type wait_handle_inner(const Type&);
  friend Type sval(SString);
  friend Type sval_nonstatic(SString);
  friend Type ival(int64_t);
  friend Type dval(double);
  friend Type aval(SArray);
  friend Type subObj(res::Class);
  friend Type objExact(res::Class);
  friend Type subCls(res::Class);
  friend Type clsExact(res::Class);
  friend Type exactRecord(res::Record);
  friend Type subRecord(res::Record);
  friend Type rname(SString);
  friend Type packed_impl(trep, std::vector<Type>, ProvTag);
  friend Type packedn_impl(trep, Type);
  friend Type map_impl(trep, MapElems, Type, Type, ProvTag);
  friend Type mapn_impl(trep bits, Type k, Type v, ProvTag);
  friend Type mapn_impl_from_map(trep bits, Type k, Type v, ProvTag);
  friend DObj dobj_of(const Type&);
  friend DRecord drec_of(const Type&);
  friend DCls dcls_of(Type);
  friend SString sval_of(const Type&);
  friend Type union_of(Type, Type);
  friend Type intersection_of(Type, Type);
  friend void widen_type_impl(Type&, uint32_t);
  friend Type widen_type(Type);
  friend Type widening_union(const Type&, const Type&);
  friend Type promote_emptyish(Type, Type);
  friend Emptiness emptiness(const Type&);
  friend Type opt(Type);
  friend Type unopt(Type);
  friend Type unnullish(Type);
  friend bool is_opt(const Type&);
  friend bool is_nullish(const Type&);
  friend Type project_data(Type t, trep bits);
  friend bool must_be_counted(const Type&);
  friend bool must_be_counted(const Type&, trep bits);
  friend Type remove_counted(Type t);
  template<typename R, bool>
  friend R tvImpl(const Type&);
  friend Type scalarize(Type t);
  friend folly::Optional<size_t> array_size(const Type& t);

  friend Type return_with_context(Type, Type);
  friend Type setctx(Type, bool);
  friend Type unctx(Type);
  friend std::string show(const Type&);
  friend ArrKey disect_array_key(const Type&);
  friend ProvTag arr_like_update_prov_tag(const Type&, ProvTag);
  friend std::pair<Type,bool> arr_val_elem(const Type& aval, const ArrKey& key);
  friend std::pair<Type,bool> arr_map_elem(const Type& map, const ArrKey& key);
  friend std::pair<Type,bool> arr_packed_elem(const Type& pack,
                                              const ArrKey& key);
  friend std::pair<Type,bool> arr_packedn_elem(const Type& pack,
                                               const ArrKey& key);
  friend std::pair<Type,ThrowMode> array_like_elem(const Type& arr,
                                                   const ArrKey& key,
                                                   const Type& defaultTy);
  friend std::pair<Type,ThrowMode> array_like_set(Type arr,
                                                  const ArrKey& key,
                                                  const Type& val,
                                                  ProvTag src);
  friend std::pair<Type,Type> array_like_newelem(Type arr, const Type& val,
                                                 ProvTag src);
  friend bool arr_map_set(Type& map, const ArrKey& key,
                          const Type& val, ProvTag src);
  friend bool arr_packed_set(Type& pack, const ArrKey& key,
                             const Type& val,
                             ProvTag src);
  friend bool arr_packedn_set(Type& pack, const ArrKey& key,
                              const Type& val, bool maybeEmpty);
  friend bool arr_mapn_set(Type& map, const ArrKey& key, const Type& val);
  friend Type arr_map_newelem(Type& map, const Type& val, ProvTag src);
  friend IterTypes iter_types(const Type&);
  friend RepoAuthType make_repo_type_arr(ArrayTypeTable::Builder&,
    const Type&);

  friend struct ArrKey disect_vec_key(const Type&);
  friend struct ArrKey disect_strict_key(const Type&);

  friend Type spec_array_like_union(Type&, Type&, trep, trep);
  friend Type aempty_varray(ProvTag);
  friend Type aempty_darray(ProvTag);
  friend Type vec_val(SArray);
  friend Type vec_empty(ProvTag);
  friend Type dict_val(SArray);
  friend Type dict_empty(ProvTag);
  friend Type some_aempty_darray(ProvTag);
  friend Type some_vec_empty(ProvTag);
  friend Type some_dict_empty(ProvTag);
  friend Type keyset_val(SArray);
  friend bool could_contain_objects(const Type&);
  friend bool could_copy_on_write(const Type&);
  friend Type loosen_staticness(Type);
  friend Type loosen_dvarrayness(Type);
  friend Type loosen_provenance(Type);
  friend Type loosen_values(Type);
  friend Type loosen_emptiness(Type);
  friend Type loosen_likeness(Type);
  friend Type add_nonemptiness(Type);
  friend Type assert_emptiness(Type);
  friend Type assert_nonemptiness(Type);
  friend Type set_trep(Type&, trep);
  friend Type remove_uninit(Type t);
  friend Type to_cell(Type t);
  friend bool inner_types_might_raise(const Type& t1, const Type& t2);
private:
  union Data {
    Data() {}
    ~Data() {}

#define DT(tag_name,type,name) type name;
  DATATAGS
#undef DT
  };

  template<class Ret, class T, class Function>
  struct DDHelperFn;

private:
  static Type unctxHelper(Type, bool&);
  static Type unionArrLike(Type a, Type b);
  template<class Ret, class T, class Function>
  DDHelperFn<Ret,T,Function> ddbind(const Function& f, const T& t) const;
  template<class Ret, class T, class Function>
  Ret dd2nd(const Type&, DDHelperFn<Ret,T,Function>) const;
  template<class Function> typename Function::result_type
  dualDispatchDataFn(const Type&, Function) const;
  bool hasData() const;
  template<bool contextSensitive>
  bool equivData(const Type&) const;
  template<bool contextSensitive>
  bool subtypeData(const Type&) const;
  bool couldBeData(const Type&) const;
  bool checkInvariants() const;
  ProvTag getProvTag() const;

private:
  trep m_bits;
  DataTag m_dataTag = DataTag::None;
  Data m_data;
};

//////////////////////////////////////////////////////////////////////

struct ArrKey {
  folly::Optional<int64_t> i;
  folly::Optional<SString> s;
  Type type;
  bool mayThrow = false;

  folly::Optional<TypedValue> tv() const {
    assert(!i || !s);
    if (i) {
      return make_tv<KindOfInt64>(*i);
    }
    if (s) {
      return make_tv<KindOfPersistentString>(*s);
    }
    return folly::none;
  }
};

struct DArrLikePacked {
  explicit DArrLikePacked(std::vector<Type> elems, ProvTag tag)
    : elems(std::move(elems))
    , provenance(tag)
  {}

  std::vector<Type> elems;
  ProvTag provenance;
};

struct DArrLikePackedN {
  explicit DArrLikePackedN(Type t) : type(std::move(t)) {}
  Type type;
};

struct DArrLikeMap {
  DArrLikeMap() {}
  explicit DArrLikeMap(MapElems map, Type optKey, Type optVal, ProvTag tag)
    : map(std::move(map))
    , optKey(std::move(optKey))
    , optVal(std::move(optVal))
    , provenance(tag)
  {}
  bool hasOptElements() const { return !optKey.subtypeOf(BBottom); }
  // The array always starts with these known keys
  MapElems map;
  // Key/value types for optional elements after known keys. Bottom if
  // none.
  Type optKey;
  Type optVal;
  ProvTag provenance;
};

struct DArrLikeMapN {
  explicit DArrLikeMapN(Type key, Type val)
    : key(std::move(key))
    , val(std::move(val))
  {}
  Type key;
  Type val;
};

//////////////////////////////////////////////////////////////////////

#define TYPES(X)                                \
X(Bottom)                                       \
X(Uninit)                                       \
X(InitNull)                                     \
X(False)                                        \
X(True)                                         \
X(Int)                                          \
X(Dbl)                                          \
X(SStr)                                         \
X(SArrE)                                        \
X(SArrN)                                        \
X(Obj)                                          \
X(Res)                                          \
X(Cls)                                          \
X(Func)                                         \
X(ClsMeth)                                      \
X(Record)                                       \
X(SVecE)                                        \
X(SVecN)                                        \
X(SDictE)                                       \
X(SDictN)                                       \
X(SKeysetE)                                     \
X(SKeysetN)                                     \
X(Null)                                         \
X(Bool)                                         \
X(Num)                                          \
X(Str)                                          \
X(SArr)                                         \
X(ArrE)                                         \
X(ArrN)                                         \
X(Arr)                                          \
X(SVec)                                         \
X(VecE)                                         \
X(VecN)                                         \
X(Vec)                                          \
X(SDict)                                        \
X(DictE)                                        \
X(DictN)                                        \
X(Dict)                                         \
X(SKeyset)                                      \
X(KeysetE)                                      \
X(KeysetN)                                      \
X(Keyset)                                       \
X(SPArrE)                                       \
X(SPArrN)                                       \
X(SPArr)                                        \
X(PArrE)                                        \
X(PArrN)                                        \
X(PArr)                                         \
X(SVArrE)                                       \
X(SVArrN)                                       \
X(SVArr)                                        \
X(VArrE)                                        \
X(VArrN)                                        \
X(VArr)                                         \
X(SDArrE)                                       \
X(SDArrN)                                       \
X(SDArr)                                        \
X(DArrE)                                        \
X(DArrN)                                        \
X(DArr)                                         \
X(UncArrKey)                                    \
X(ArrKey)                                       \
X(FuncOrCls)                                    \
X(UncStrLike)                                   \
X(StrLike)                                      \
X(PArrLikeSA)                                   \
X(PArrLike)                                     \
X(VArrLikeSA)                                   \
X(VArrLike)                                     \
X(VecLikeSA)                                    \
X(VecLike)                                      \
X(InitPrim)                                     \
X(Prim)                                         \
X(InitUnc)                                      \
X(Unc)                                          \
X(OptTrue)                                      \
X(OptFalse)                                     \
X(OptBool)                                      \
X(OptInt)                                       \
X(OptDbl)                                       \
X(OptNum)                                       \
X(OptSStr)                                      \
X(OptStr)                                       \
X(OptSArrE)                                     \
X(OptSArrN)                                     \
X(OptSArr)                                      \
X(OptArrE)                                      \
X(OptArrN)                                      \
X(OptArr)                                       \
X(OptObj)                                       \
X(OptRes)                                       \
X(OptFunc)                                      \
X(OptCls)                                       \
X(OptClsMeth)                                   \
X(OptRecord)                                    \
X(OptSVecE)                                     \
X(OptSVecN)                                     \
X(OptSVec)                                      \
X(OptVecE)                                      \
X(OptVecN)                                      \
X(OptVec)                                       \
X(OptSDictE)                                    \
X(OptSDictN)                                    \
X(OptSDict)                                     \
X(OptDictE)                                     \
X(OptDictN)                                     \
X(OptDict)                                      \
X(OptSKeysetE)                                  \
X(OptSKeysetN)                                  \
X(OptSKeyset)                                   \
X(OptKeysetE)                                   \
X(OptKeysetN)                                   \
X(OptKeyset)                                    \
X(OptSPArrE)                                    \
X(OptSPArrN)                                    \
X(OptSPArr)                                     \
X(OptPArrE)                                     \
X(OptPArrN)                                     \
X(OptPArr)                                      \
X(OptSVArrE)                                    \
X(OptSVArrN)                                    \
X(OptSVArr)                                     \
X(OptVArrE)                                     \
X(OptVArrN)                                     \
X(OptVArr)                                      \
X(OptSDArrE)                                    \
X(OptSDArrN)                                    \
X(OptSDArr)                                     \
X(OptDArrE)                                     \
X(OptDArrN)                                     \
X(OptDArr)                                      \
X(OptUncArrKey)                                 \
X(OptArrKey)                                    \
X(OptFuncOrCls)                                 \
X(OptUncStrLike)                                \
X(OptStrLike)                                   \
X(OptPArrLikeSA)                                \
X(OptPArrLike)                                  \
X(OptVArrLikeSA)                                \
X(OptVArrLike)                                  \
X(OptVecLikeSA)                                 \
X(OptVecLike)                                   \
X(InitCell)                                     \
X(Cell)                                         \
X(Top)

#define X(y) extern const Type T##y;
TYPES(X)
#undef X

// These are treps that have B* names, but which are not "predefined"
// types. They are only allowed in combination with the corresponding
// S types.
#define NON_TYPES(X)                            \
  X(CStr)                                       \
  X(CPArrE)                                     \
  X(CPArrN)                                     \
  X(CVArrE)                                     \
  X(CVArrN)                                     \
  X(CDArrE)                                     \
  X(CDArrN)                                     \
  X(CArrE)                                      \
  X(CArrN)                                      \
  X(CVecE)                                      \
  X(CVecN)                                      \
  X(CDictE)                                     \
  X(CDictN)                                     \
  X(CKeysetE)                                   \
  X(CKeysetN)                                   \
  X(CPArr)                                      \
  X(CVArr)                                      \
  X(CDArr)                                      \
  X(CArr)                                       \
  X(CVec)                                       \
  X(CDict)                                      \
  X(CKeyset)                                    \
  X(OptCStr)                                    \
  X(OptCPArrE)                                  \
  X(OptCPArrN)                                  \
  X(OptCPArr)                                   \
  X(OptCVArrE)                                  \
  X(OptCVArrN)                                  \
  X(OptCVArr)                                   \
  X(OptCDArrE)                                  \
  X(OptCDArrN)                                  \
  X(OptCDArr)                                   \
  X(OptCArrE)                                   \
  X(OptCArrN)                                   \
  X(OptCArr)                                    \
  X(OptCVecE)                                   \
  X(OptCVecN)                                   \
  X(OptCVec)                                    \
  X(OptCDictE)                                  \
  X(OptCDictN)                                  \
  X(OptCDict)                                   \
  X(OptCKeysetE)                                \
  X(OptCKeysetN)                                \
  X(OptCKeyset)

//////////////////////////////////////////////////////////////////////

/*
 * Return WaitH<T> for a type t.
 */
Type wait_handle(const Index&, Type t);

/*
 * Return T from a WaitH<T>.
 *
 * Pre: is_specialized_handle(t);
 */
Type wait_handle_inner(const Type& t);

/*
 * Create Types that represent constant values.
 */
Type sval(SString);
Type ival(int64_t);
Type dval(double);
Type aval(SArray);
Type vec_val(SArray);
Type dict_val(SArray);
Type keyset_val(SArray);
Type sval_nonstatic(SString);

/*
 * Create static empty array or string types.
 */
Type sempty();
Type aempty();
Type aempty_varray(ProvTag = ProvTag::Top);
Type aempty_darray(ProvTag = ProvTag::Top);
Type vec_empty(ProvTag = ProvTag::Top);
Type dict_empty(ProvTag = ProvTag::Top);
Type keyset_empty();

/*
 * Create an any-countedness empty array/vec/dict type.
 */
Type some_aempty();
Type some_aempty_darray(ProvTag = ProvTag::Top);
Type some_vec_empty(ProvTag = ProvTag::Top);
Type some_dict_empty(ProvTag = ProvTag::Top);
Type some_keyset_empty();

/*
 * Create types for objects or classes with some known constraint on
 * which res::Class is associated with them.
 */
Type subObj(res::Class);
Type objExact(res::Class);
Type subCls(res::Class);
Type clsExact(res::Class);

/*
 * Create types for records with some known constraint on an associated
 * res::Record.
 */
Type exactRecord(res::Record);
Type subRecord(res::Record);

/*
 * Packed array types with known size.
 *
 * Pre: !v.empty()
 */
Type arr_packed(std::vector<Type> v);
Type arr_packed_varray(std::vector<Type> v, ProvTag = ProvTag::Top);
Type sarr_packed(std::vector<Type> v);

/*
 * Packed array types of unknown size.
 *
 * Note that these types imply the arrays are non-empty.
 */
Type arr_packedn(Type);
Type sarr_packedn(Type);

/*
 * Struct-like arrays.
 *
 * Pre: !m.empty()
 */
Type arr_map(MapElems m, Type optKey = TBottom, Type optVal = TBottom);
Type arr_map_darray(MapElems m, ProvTag = ProvTag::Top);
Type sarr_map(MapElems m, Type optKey = TBottom, Type optVal = TBottom);

/*
 * Map-like arrays.
 */
Type arr_mapn(Type k, Type v);
Type sarr_mapn(Type k, Type v);

/*
 * vec types with known size.
 *
 * Pre: !v.empty()
 */
Type vec(std::vector<Type> v, ProvTag);
Type svec(std::vector<Type> v, ProvTag);

/*
 * Vec type of unknown size.
 */
Type vec_n(Type);
Type svec_n(Type);

/*
 * Struct-like dicts.
 *
 * Pre: !m.empty()
 */
Type dict_map(MapElems m, ProvTag,
              Type optKey = TBottom,
              Type optVal = TBottom);

/*
 * Dict with key/value types.
 */
Type dict_n(Type, Type, ProvTag);
Type sdict_n(Type, Type, ProvTag);

/*
 * Keyset with key (same as value) type.
 */
Type keyset_n(Type);
Type ckeyset_n(Type);

/*
 * Keyset from MapElems
 */
inline Type keyset_map(MapElems m) {
  return map_impl(BKeysetN, std::move(m), TBottom, TBottom, ProvTag::Top);
}

/*
 * Create the optional version of the Type t.
 *
 * Pre: there must be an optional version of the type t.
 */
Type opt(Type t);

/*
 * Return the non-optional version of the Type t.
 *
 * Pre: is_opt(t)
 */
Type unopt(Type t);

/*
 * Returns whether a given type is a subtype of one of the predefined
 * optional types.  (Note that this does not include types like
 * TInitUnc---it's only the TOpt* types.)
 */
bool is_opt(const Type& t);

/*
 * Return t with BNull removed from its trep.
 *
 * Pre: is_nullish(t)
 */
Type unnullish(Type t);

/*
 * Returns whether a given type couldBe TNull, and would still be
 * predefined if BNull was removed from its trep.
 */
bool is_nullish(const Type& t);

/*
 * Improves the type `t` given the current context.  This returns the
 * intersection of the type `t` with the `context` if the `context` is a valid
 * class or object, and `t` is tagged as being the context.  If `context` is
 * a class we will first convert it to an object.  This returns an optional type
 * if `t` was optional.
 */
Type return_with_context(Type t, Type context);

/*
 * If `to` is false.  This is an identity operation.  If `to` is true, and
 * `t` is a specialized object or class, this will return `t` tagged as a
 * context.
 */
Type setctx(Type t, bool to = true);

/*
 * This removes any context tags in the type `t`, even if nested inside other
 * types.
 */
Type unctx(Type t);

/*
 * Discards any data associated with `t` that isn't valid for the given trep
 *
 * At the moment this will do nothing beyond removing empty ArrLikeDatas from
 * types that used to only have ArrE bits set but now have a ArrN bit set
 */
Type project_data(Type t, trep bits);

/*
 * Returns true if the type can only be counted. We don't allow the
 * the usage of the counted side of the type lattice, so this checks
 * if the type is a definitely counted type (IE, object or resource),
 * or if it has an array specialization which contains such a type
 * recursively (an array which contains a definitely counted type must
 * itself by counted).
 */
bool must_be_counted(const Type&);

/*
 * Refinedness equivalence checks.
 */
bool equivalently_refined(const Type&, const Type&);

template<
  typename Iterable,
  typename = std::enable_if<
    std::is_same<typename Iterable::value_type, Type>::value
  >
>
bool equivalently_refined(const Iterable& a, const Iterable& b) {
  if (a.size() != b.size()) return false;
  for (auto ita = a.begin(), itb = b.begin(); ita != a.end(); ++ita, ++itb) {
    if (!equivalently_refined(*ita, *itb)) return false;
  }
  return true;
}

/*
 * Returns true if type 't' represents a "specialized" object, that is an
 * object of a known class, or an optional object of a known class.
 */
bool is_specialized_obj(const Type&);

/*
 * Returns true if type 't' represents a "specialized" class---i.e. a class
 * with a DCls structure.
 */
bool is_specialized_cls(const Type&);

/*
 * Returns whether `t' is a WaitH<T> or ?WaitH<T> for some T.
 *
 * Note that this function returns false for Obj<=WaitHandle with no
 * tracked inner type.
 */
bool is_specialized_wait_handle(const Type& t);

/*
 * Returns whether `t' is a one of the array like types, or
 * an optional version of one of those types.  That is, with
 * either a constant value or some (maybe partially) known shape.
 */
bool is_specialized_array(const Type& t);
bool is_specialized_vec(const Type& t);
bool is_specialized_dict(const Type& t);
bool is_specialized_keyset(const Type& t);

/*
 * Returns the best known instantiation of a class type.  Or returns the
 * provided object.
 *
 * Pre: t.subypeOfAny(TObj, TCls)
 */
Type toobj(const Type& t);

/*
 * Returns the best known TCls subtype for an object type.  Otherwise return
 * the passed in type.
 */
Type objcls(const Type& t);

/*
 * If the type t has a known constant value, return it as a TypedValue.
 * Otherwise return folly::none.
 *
 * The returned TypedValue can only contain non-reference-counted types.
 */
folly::Optional<TypedValue> tv(const Type& t);

/*
 * If the type t has a known constant value, return it as a TypedValue.
 * Otherwise return folly::none.
 *
 * The returned TypedValue may contain reference-counted types.
 *
 * You are responsible for any required ref-counting.
 */
folly::Optional<TypedValue> tvNonStatic(const Type& t);

/*
 * If the type t has a known constant value, return true.
 * Otherwise return false.
 */
bool is_scalar(const Type& t);

/*
 * Return the canonical scalar type for t - equivalent to
 * from_cell(*tv(t)).
 *
 * This can be used to ensure that the arguments in a CallContext are
 * canonicalized, so that immaterial changes to them (eg TArrN ->
 * TSArrN or DArrLikeMap -> DArrLikeVal) don't affect which entry gets
 * looked up.
 *
 * pre: is_scalar(t).
 */
Type scalarize(Type t);

/*
 * If t represents an array, and we know its size, return it.
 */
folly::Optional<size_t> array_size(const Type& t);

/*
 * Get the type in our typesystem that corresponds to an hhbc
 * IsTypeOp.
 *
 * Pre: op != IsTypeOp::Scalar
 */
Type type_of_istype(IsTypeOp op);

/*
 * Get the hhbc IsTypeOp that corresponds to the type in our typesystem.
 * Returns folly::none if no matching IsTypeOp is found.
 */
folly::Optional<IsTypeOp> type_to_istypeop(const Type& t);

/*
 * Get the type in our typesystem that corresponds to type given by the
 * potentially unresolved type structure.
 * Returns folly::none if the type structure is unresolved or
 * no matching Type is found.
 *
 */
folly::Optional<Type> type_of_type_structure(const Index&, Context, SArray ts);

/*
 * Return the DObj structure for a strict subtype of TObj or TOptObj.
 *
 * Pre: is_specialized_obj(t)
 */
DObj dobj_of(const Type& t);

/*
 * Return the DCls structure for a strict subtype of TCls.
 *
 * Pre: is_specialized_cls(t)
 */
DCls dcls_of(Type t);

/*
 * Return the SString for a strict subtype of TStr.
 *
 * Pre: is_specialized_string(t)
 */
SString sval_of(const Type& t);

/*
 * Create a Type from a TypedValue.
 *
 * Pre: the cell must contain a non-reference-counted type.
 * Post: returned type is a subtype of TUnc
 */
Type from_cell(TypedValue tv);

/*
 * Create a Type from a DataType. KindOfString and KindOfPersistentString
 * are both treated as TStr.
 *
 * Pre: dt is one of the DataTypes that actually represent php values
 * (or KindOfUninit).
 */
Type from_DataType(DataType dt);

/*
 * Create a Type from a builtin type specification string.
 *
 * This is used for HNI class properties.  We assume that these are
 * accurate.  `s' may be nullptr.
 */
Type from_hni_constraint(SString s);

/*
 * Make a type that represents values from the intersection of the
 * supplied types.
 */
Type intersection_of(Type a, Type b);

/*
 * Make a type that represents values from either of the supplied
 * types.
 *
 * Importantly, note that there are infinitely long chains of array
 * types that continue to become less specialized, so chains of
 * union_of operations are not guaranteed to reach a stable point in
 * finite steps.
 */
Type union_of(Type a, Type b);

/*
 * Widening union.
 *
 * This operation returns a type T, such that a is a subtype of T, b
 * is a subtype of T, and union_of(a, b) is a subtype of T.  The
 * widening union also has the property that every possible chain of
 * successive applications of the function eventually reaches a stable
 * point.
 *
 * This is currently implemented by unioning the types, then applying
 * widen_type() to the result.
 *
 * For portions of our analysis that rely on growing types reaching
 * stable points for termination, this function must occasionally be
 * used instead of union_of to guarantee termination.  See details in
 * analyze.cpp.
 */
Type widening_union(const Type& a, const Type& b);

/*
 * Widen a type to one which has a finite chain under the union operator. This
 * generally involves restricting the type's nesting depth to a fixed limit and
 * preventing a specialized array type from growing larger unbounded.
 */
Type widen_type(Type t);

/*
 * A sort of union operation that also attempts to remove "emptyish" types from
 * union_of(a, b).  This is useful for promoting emptyish types (sempty(),
 * false, and null) to stdClass or to arrays, in member instructions.
 *
 * This function currently doesn't give specific guarantees about exactly when
 * the emptyish types will not be part of the return type (informally it only
 * happens in the "easy" cases right now), so you should not use it in
 * situations where union_of(a, b) would not also be correct.
 */
Type promote_emptyish(Type a, Type b);


/*
 * Returns what we know about the emptiness of the type.
 */
Emptiness emptiness(const Type&);

/*
 * Returns whether a Type could hold an object that has a custom
 * boolean conversion function.
 */
bool could_have_magic_bool_conversion(const Type&);

/*
 * Returns the smallest type that `a' is a subtype of, from the
 * following set: TInitCell, TUninit.
 *
 * Pre: `a' is a subtype of TCell.
 */
Type stack_flav(Type a);

/*
 * Discard any countedness information about the type. Force any type
 * (recursively) which contains only static or counted variants to contain
 * both. Doesn't change the type otherwise.
 */
Type loosen_staticness(Type);

/*
 * Discard any specific knowledge about whether the type is a d/varray. Force
 * any type which might contain any sub-types of PArr, VArr, or DArr to contain
 * Arr, while keeping the same staticness and emptiness information.
 */
Type loosen_dvarrayness(Type);

/*
 * Discard any specific provenance tag on this type and any sub-arrays
 */
Type loosen_provenance(Type);

/*
 * Force any type which might contain any sub-types of Arr, Vec, Dict, and
 * Keyset to contain Arr, Vec, Dict, and Keyset. This is needed for some
 * operations whose effects on arrays cannot be predicted. Doesn't change the
 * type otherwise.
 */
Type loosen_arrays(Type);

/*
 * Drop any data from the type (except for object class information) and force
 * TTrue or TFalse to TBool. Doesn't change the type otherwise.
 */
Type loosen_values(Type t);

/*
 * Discard any emptiness information about the type. Force any type which
 * contains only empty or non-empty variants to contain both. Doesn't change the
 * type otherwise.
 */
Type loosen_emptiness(Type t);

/*
 * Force all TFunc and TCls types to TUncStrLike, and all TClsMeth to either
 * TVArrLike or TVecLike.
 */
Type loosen_likeness(Type t);

/*
 * Loosens staticness, emptiness, and values from the type. This forces a type
 * to its most basic form (except for object class information).
 */
Type loosen_all(Type t);

/*
 * If t contains TUninit, returns the best type we can that contains
 * at least everything t contains, but doesn't contain TUninit.  Note
 * that this function will return TBottom for TUninit.
 */
Type remove_uninit(Type t);

/*
 * If t is not a TCell, returns TInitCell. Otherwise, if t contains
 * TUninit, return union_of(remove_uninit(t), TInitCell).
 */
Type to_cell(Type t);

/*
 * Add non-empty variants of the type to the type if not already
 * present. Doesn't change the type otherwise.
 */
Type add_nonemptiness(Type t);

/*
 * Force `t` to only contain static types, including any specialized
 * data recursively. If `t` is definitely counted, TBottom will be
 * returned.
 */
Type remove_counted(Type t);

/*
 * Produced the most refined type possible, given that
 * t passed/failed an emptiness check.
 */
Type assert_emptiness(Type);
Type assert_nonemptiness(Type);

/*
 * (array|vec|dict|keyset)_elem
 *
 * Returns the best known type of an array inner element given a type
 * for the key.  The returned type is always a subtype of TInitCell.
 *
 * The returned type will be TBottom if the operation will always throw.
 * ThrowMode indicates what kind of failures may occur.
 *
 * Pre: first arg is a subtype of TArr, TVec, TDict, TKeyset respectively.
 */
std::pair<Type,ThrowMode> array_elem(const Type& arr, const Type& key,
                                     const Type& defaultTy = TInitNull);
std::pair<Type, ThrowMode> vec_elem(const Type& vec, const Type& key,
                                    const Type& defaultTy = TBottom);
std::pair<Type, ThrowMode> dict_elem(const Type& dict, const Type& key,
                                     const Type& defaultTy = TBottom);
std::pair<Type, ThrowMode> keyset_elem(const Type& keyset, const Type& key,
                                       const Type& defaultTy = TBottom);

/*
 * (array|vec|dict|keyset)_set
 *
 * Perform an array-like set on types.  Returns a type that represents the
 * effects of arr[key] = val.
 *
 * The returned type will be TBottom if the operation will always throw.
 * ThrowMode indicates what kind of failures may occur.
 *
 * Pre: first arg is a subtype of TArr, TVec, TDict, TKeyset respectively.
 */
std::pair<Type, ThrowMode> array_set(Type arr, const Type& key,
                                     const Type& val, ProvTag src);
std::pair<Type, ThrowMode> vec_set(Type vec, const Type& key, const Type& val,
                                   ProvTag src);
std::pair<Type, ThrowMode> dict_set(Type dict, const Type& key,
                                    const Type& val, ProvTag src);
std::pair<Type, ThrowMode> keyset_set(Type keyset, const Type& key,
                                      const Type& val);

/*
 * (array|vec|dict|keyset)_newelem
 *
 * Perform a newelem operation on an array-like type.  Returns an
 * array that contains a new pushed-back element with the supplied
 * value, in the sense of arr[] = val, and the best known type of the
 * key that was added.
 *
 * Pre: first arg is a subtype of TArr, TVec, TDict, TKeyset respectively.
 */
std::pair<Type,Type> array_newelem(Type arr, const Type& val, ProvTag src);
std::pair<Type,Type> vec_newelem(Type vec, const Type& val, ProvTag src);
std::pair<Type,Type> dict_newelem(Type dict, const Type& val, ProvTag src);
std::pair<Type,Type> keyset_newelem(Type keyset, const Type& val);

/*
 * Return the best known information for iteration of the supplied type. This is
 * only intended for non-mutable iteration, so the returned types are at worst
 * InitCell.
 */
struct IterTypes {
  Type key;
  Type value;
  // The number of elements we're iterating over:
  enum struct Count {
    Empty,     // No elements
    Single,    // Exactly one element
    ZeroOrOne, // Less than 2 elements
    NonEmpty,  // Unknown upper bound, but non-empty
    Any        // Nothing known
  };
  Count count;
  // Can a IterInit[K] op throw on this iterator?
  bool mayThrowOnInit;
  // Can a IterNext[K] op throw on this iterator? Can only happen for object
  // types.
  bool mayThrowOnNext;
};
IterTypes iter_types(const Type&);

/*
 * Create a RepoAuthType for a Type.
 *
 * RepoAuthTypes may contain things like RepoAuthType::Array*'s or
 * SStrings for class names.  The emit code needs to handle making
 * sure these things are merged into the appropriate unit or repo.
 *
 * Pre: !t.couldBe(BCls)
 *      !t.subtypeOf(BBottom)
 */
RepoAuthType make_repo_type(ArrayTypeTable::Builder&, const Type& t);

/*
 * Returns true iff an IsType testing for testTy/testOp on valTy might raise.
 */
bool is_type_might_raise(const Type& testTy, const Type& valTy);
bool is_type_might_raise(IsTypeOp testOp, const Type& valTy);

/*
 * Returns true iff a compare of two types might raise a HAC notice
 */
bool compare_might_raise(const Type& t1, const Type& t2);

/*
 * Given a type, adjust the type for the given type-constraint. If there's no
 * type-constraint, or if property type-hints aren't being enforced, then return
 * the type as is. This might return TBottom if the type is not compatible with
 * the type-hint.
 */
Type adjust_type_for_prop(const Index& index,
                          const php::Class& propCls,
                          const TypeConstraint* tc,
                          const Type& ty);

//////////////////////////////////////////////////////////////////////

}}

#endif
