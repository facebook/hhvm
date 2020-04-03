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
#include "hphp/hhbbc/type-system.h"

#include <type_traits>
#include <cmath>
#include <algorithm>
#include <iterator>
#include <vector>

#include <folly/Optional.h>
#include <folly/Traits.h>
#include <folly/Hash.h>

#include "hphp/runtime/base/array-init.h"
#include "hphp/runtime/base/array-iterator.h"
#include "hphp/runtime/base/double-to-int64.h"
#include "hphp/runtime/base/mixed-array.h"
#include "hphp/runtime/base/packed-array.h"
#include "hphp/runtime/base/repo-auth-type-array.h"
#include "hphp/runtime/base/repo-auth-type.h"
#include "hphp/runtime/base/set-array.h"
#include "hphp/runtime/base/tv-comparisons.h"
#include "hphp/runtime/base/type-structure.h"
#include "hphp/runtime/base/type-structure-helpers-defs.h"

#include "hphp/hhbbc/context.h"
#include "hphp/hhbbc/eval-cell.h"
#include "hphp/hhbbc/index.h"

namespace HPHP { namespace HHBBC {

TRACE_SET_MOD(hhbbc);

#define X(y) const Type T##y{B##y};
TYPES(X)
#undef X

ProvTag ProvTag::SomeTag = arrprov::Tag::RepoUnion();
ProvTag ProvTag::Top = {};
ProvTag ProvTag::NoTag = ProvTag::KnownEmpty{};

namespace {

//////////////////////////////////////////////////////////////////////

const StaticString s_Awaitable("HH\\Awaitable");

//////////////////////////////////////////////////////////////////////

// When widening a type, allow no specialized information at a nesting depth
// greater than this. This keeps any such types from growing unbounded.
constexpr int kTypeWidenMaxDepth = 8;

//////////////////////////////////////////////////////////////////////

// Legal to call with !isPredefined(bits)
bool mayHaveData(trep bits) {
  bits &= ~BUninit;
  switch (bits) {
  case BSStr:    case BStr:
  case BOptSStr: case BOptStr:
  case BObj:     case BInt:    case BDbl:     case BRecord:
  case BOptObj:  case BOptInt: case BOptDbl:  case BOptRecord:
  case BCls:
  case BArr:     case BSArr:     case BCArr:
  case BArrN:    case BSArrN:    case BCArrN:
  case BOptArr:  case BOptSArr:  case BOptCArr:
  case BOptArrN: case BOptSArrN: case BOptCArrN:
  case BFunc:
  case BVec:      case BSVec:      case BCVec:
  case BVecN:     case BSVecN:     case BCVecN:
  case BOptVec:   case BOptSVec:   case BOptCVec:
  case BOptVecN:  case BOptSVecN:  case BOptCVecN:
  case BDict:     case BSDict:     case BCDict:
  case BDictN:    case BSDictN:    case BCDictN:
  case BOptDict:  case BOptSDict:  case BOptCDict:
  case BOptDictN: case BOptSDictN: case BOptCDictN:
  case BKeyset:     case BSKeyset:     case BCKeyset:
  case BKeysetN:    case BSKeysetN:    case BCKeysetN:
  case BOptKeyset:  case BOptSKeyset:  case BOptCKeyset:
  case BOptKeysetN: case BOptSKeysetN: case BOptCKeysetN:
    return true;

  case BPArr:     case BSPArr:     case BCPArr:
  case BPArrN:    case BSPArrN:    case BCPArrN:
  case BOptPArr:  case BOptSPArr:  case BOptCPArr:
  case BOptPArrN: case BOptSPArrN: case BOptCPArrN:
  case BVArr:     case BSVArr:     case BCVArr:
  case BVArrN:    case BSVArrN:    case BCVArrN:
  case BOptVArr:  case BOptSVArr:  case BOptCVArr:
  case BOptVArrN: case BOptSVArrN: case BOptCVArrN:
  case BDArr:     case BSDArr:     case BCDArr:
  case BDArrN:    case BSDArrN:    case BCDArrN:
  case BOptDArr:  case BOptSDArr:  case BOptCDArr:
  case BOptDArrN: case BOptSDArrN: case BOptCDArrN:
    return true;

  case BSArrE: case BCArrE:
  case BSVArrE:    case BCVArrE:    case BVArrE:
  case BOptSVArrE: case BOptCVArrE: case BOptVArrE:
  case BSDArrE:    case BCDArrE:    case BDArrE:
  case BOptSDArrE: case BOptCDArrE: case BOptDArrE:
  case BCVecE:     case BSVecE:     case BVecE:
  case BOptCVecE:  case BOptSVecE:  case BOptVecE:
  case BCDictE:    case BSDictE:    case BDictE:
  case BOptCDictE: case BOptSDictE: case BOptDictE:
    return true;

  case BVArrLike: case BVArrLikeSA:
  case BOptVArrLike: case BOptVArrLikeSA:
  case BVecLike: case BVecLikeSA:
  case BOptVecLike: case BOptVecLikeSA:
  case BPArrLike: case BPArrLikeSA:
  case BOptPArrLike: case BOptPArrLikeSA:
    return true;

  case BBottom:
  case BUninit:
  case BInitNull:
  case BFalse:
  case BTrue:
  case BCStr:
  case BSKeysetE:
  case BCKeysetE:
  case BSPArrE:
  case BCPArrE:
  case BPArrE:
  case BRes:
  case BNull:
  case BNum:
  case BBool:
  case BArrE:
  case BKeysetE:
  case BInitPrim:
  case BPrim:
  case BInitUnc:
  case BUnc:
  case BArrKey:
  case BUncArrKey:
  case BFuncOrCls:
  case BOptFuncOrCls:
  case BStrLike:
  case BUncStrLike:
  case BOptTrue:
  case BOptFalse:
  case BOptBool:
  case BOptNum:
  case BOptCStr:
  case BOptSArrE:
  case BOptCArrE:
  case BOptArrE:
  case BOptSKeysetE:
  case BOptCKeysetE:
  case BOptKeysetE:
  case BOptSPArrE:
  case BOptCPArrE:
  case BOptPArrE:
  case BOptRes:
  case BOptArrKey:
  case BOptUncArrKey:
  case BOptStrLike:
  case BOptUncStrLike:
  case BOptFunc:
  case BOptCls:
  case BClsMeth:
  case BOptClsMeth:
  case BInitCell:
  case BCell:
  case BTop:
    break;
  }
  return false;
}

// Pre: isPredefined(bits)
bool canBeOptional(trep bits) {
  if (bits & BUninit) return false;
  switch (bits) {
  case BBottom:
    return false;

  case BNull:
  case BUninit:
  case BInitNull:
    return false;

  case BFalse:
  case BTrue:
  case BInt:
  case BDbl:
  case BSStr:
  case BSArrE:
  case BSArrN:
  case BSVecE:
  case BSVecN:
  case BSDictE:
  case BSDictN:
  case BSKeysetE:
  case BSKeysetN:
  case BObj:
  case BRes:
  case BFunc:
  case BCls:
  case BClsMeth:
  case BRecord:
    return true;

  case BSPArrE:
  case BSPArrN:
  case BSPArr:
  case BPArrE:
  case BPArrN:
  case BPArr:
  case BSVArrE:
  case BSVArrN:
  case BSVArr:
  case BVArrE:
  case BVArrN:
  case BVArr:
  case BSDArrE:
  case BSDArrN:
  case BSDArr:
  case BDArrE:
  case BDArrN:
  case BDArr:
    return true;

  case BNum:
  case BBool:
  case BStr:
  case BUncArrKey:
  case BArrKey:
  case BFuncOrCls:
  case BUncStrLike:
  case BStrLike:
  case BPArrLikeSA:
  case BPArrLike:
  case BVArrLikeSA:
  case BVArrLike:
  case BVecLikeSA:
  case BVecLike:
  case BSArr:
  case BArrE:
  case BArrN:
  case BArr:
  case BSVec:
  case BVecE:
  case BVecN:
  case BVec:
  case BSDict:
  case BDictE:
  case BDictN:
  case BDict:
  case BSKeyset:
  case BKeysetE:
  case BKeysetN:
  case BKeyset:
    return true;

  case BOptTrue:
  case BOptFalse:
  case BOptBool:
  case BOptInt:
  case BOptDbl:
  case BOptNum:
  case BOptSStr:
  case BOptStr:
  case BOptSArrE:
  case BOptSArrN:
  case BOptSArr:
  case BOptArrN:
  case BOptArrE:
  case BOptArr:
  case BOptSVecE:
  case BOptSVecN:
  case BOptSVec:
  case BOptVecN:
  case BOptVecE:
  case BOptVec:
  case BOptSDictE:
  case BOptSDictN:
  case BOptSDict:
  case BOptDictN:
  case BOptDictE:
  case BOptDict:
  case BOptSKeysetE:
  case BOptSKeysetN:
  case BOptSKeyset:
  case BOptKeysetN:
  case BOptKeysetE:
  case BOptKeyset:
  case BOptSPArrE:
  case BOptSPArrN:
  case BOptSPArr:
  case BOptPArrE:
  case BOptPArrN:
  case BOptPArr:
  case BOptSVArrE:
  case BOptSVArrN:
  case BOptSVArr:
  case BOptVArrE:
  case BOptVArrN:
  case BOptVArr:
  case BOptSDArrE:
  case BOptSDArrN:
  case BOptSDArr:
  case BOptDArrE:
  case BOptDArrN:
  case BOptDArr:
  case BOptObj:
  case BOptRes:
  case BOptUncArrKey:
  case BOptArrKey:
  case BOptFuncOrCls:
  case BOptUncStrLike:
  case BOptStrLike:
  case BOptPArrLikeSA:
  case BOptPArrLike:
  case BOptVArrLikeSA:
  case BOptVArrLike:
  case BOptVecLikeSA:
  case BOptVecLike:
  case BOptFunc:
  case BOptCls:
  case BOptClsMeth:
  case BOptRecord:
    return false;

  case BInitPrim:
  case BPrim:
  case BInitUnc:
  case BUnc:
  case BInitCell:
  case BCell:
  case BTop:
    return false;

#define CASE(n) case B##n:
    NON_TYPES(CASE)
    always_assert(false);
#undef CASE
  }
  not_reached();
}

/*
 * Note: currently we're limiting all represented types to predefined
 * bit patterns (instead of arbitrary unions), so this function is
 * around for assertions.
 *
 * This may be relaxed later if we think we can get more out of it,
 * but for now the thought is that the likelihood of getting
 * significantly better types from the inference algorithm might be
 * counter-balanced by the increased chance of hard-to-track
 * type-system bugs, so at least for now (at the time of this writing,
 * before shipping hhbbc) we're ruling out a bunch of edge cases.
 *
 * Aside from types like Obj<= or things like TSStr, a lot of cases
 * with arbitrary-unions may not lead to better code at JIT time
 * (unless the inference turns them back into a predefined type),
 * since we'll have to guard anyway to see which one it was.  We'll
 * try it later.
 */
bool isPredefined(trep bits) {
  if (bits & BUninit &&
      isPredefined(bits & ~BNull) &&
      canBeOptional(bits & ~BNull)) {
    return true;
  }
#define CASE(n) case B##n:
  switch (bits) {
    TYPES(CASE)
    return true;
    NON_TYPES(CASE)
    break;
  }
#undef CASE
  return false;
}

/*
 * Combine array bits.  Our type system currently avoids arbitrary unions (see
 * rationale above), so we don't have predefined types like CArr|SArrN, or
 * SArrN|CArrE.  This function checks a few cases to ensure combining array
 * type bits leaves it predefined.
 */
template<trep B>
trep combine_arrish_bits(trep a, trep b) {
  DEBUG_ONLY constexpr trep NullishB = BNull | B;
  auto const combined = a | (b & NullishB);
  assertx((combined & NullishB) == combined);
  auto const arr_part = combined & B;
  if (!isPredefined(arr_part)) return combined | B;
  assertx(isPredefined(combined));
  return combined;
}

/*
 * Like combine_arrish_bits, but meant for combining bits representing two
 * different types with respect to d/varray-ness (IE, varray and darray). Takes
 * care of promoting to the right TArr union while maintaining the staticness
 * and emptiness bits.
 */
trep combine_dv_arrish_bits(trep a, trep b) {
  auto const combined = a | (b & (BArr | BNull));
  auto const nonnull = combined & ~BNull;
  auto const check = [&] (trep x) { return (nonnull & x) == nonnull; };
  auto const ret = [&] (trep x) {
    return x | (combined & BNull);
  };
  if (check(BSArrE)) return ret(BSArrE);
  if (check(BSArrN)) return ret(BSArrN);
  if (check(BSArr))  return ret(BSArr);
  if (check(BArrE))  return ret(BArrE);
  if (check(BArrN))  return ret(BArrN);
  if (check(BArr))   return ret(BArr);
  always_assert(false);
}

trep combine_arr_bits(trep a, trep b) {
  return combine_arrish_bits<BArr>(a, b);
}

trep combine_parr_bits(trep a, trep b) {
  return combine_arrish_bits<BPArr>(a, b);
}

trep combine_varr_bits(trep a, trep b) {
  return combine_arrish_bits<BVArr>(a, b);
}

trep combine_darr_bits(trep a, trep b) {
  return combine_arrish_bits<BDArr>(a, b);
}

trep combine_vec_bits(trep a, trep b) {
  return combine_arrish_bits<BVec>(a, b);
}

trep combine_dict_bits(trep a, trep b) {
  return combine_arrish_bits<BDict>(a, b);
}

trep combine_keyset_bits(trep a, trep b) {
  return combine_arrish_bits<BKeyset>(a, b);
}

// Combine bits; a must be a valid array-like trep; b should either be
// valid, or a repeated set such as (BSArrE | BSVecE | BSDictE |
// BSKeysetE). This lets you union in a particular set of the S,C,E
// and Opt bits without having to know which kind of array like
// structure you're dealing with (the bits that don't correspond to
// a's type will be dropped.
//
// Note that this allows you to combine bits representing different d/varray
// types. They'll promote to a TArr type.
trep combine_arr_like_bits(trep a, trep b) {
  auto check = [] (trep a, trep x) { return (a & (x | BNull)) == a; };
  assert(isPredefined(a) && !check(a, BNull));
  // If both bits have the same d/varray-ness, combine them as normal.
  if (check(a, BPArr) && check(b, BPArr)) return combine_parr_bits(a, b);
  if (check(a, BVArr) && check(b, BVArr)) return combine_varr_bits(a, b);
  if (check(a, BDArr) && check(b, BDArr)) return combine_darr_bits(a, b);
  // If they're all arrays, combine them and promote it to the right TArr union.
  if (check(a, BArr))    return combine_dv_arrish_bits(a, b);
  if (check(a, BVec))    return combine_vec_bits(a,       b);
  if (check(a, BDict))   return combine_dict_bits(a,      b);
  if (check(a, BKeyset)) return combine_keyset_bits(a,    b);
  not_reached();
}

/*
 * Like combine_arr_like_bits, but treats d/varrays are completely separate
 * types from other arrays. Unlike combine_arr_like_bits, they'll never promote
 * to a TArr union. Useful for when you want to union in something like
 * TArrLikeN, but not promote a T[P,V,D]Arr to TArr.
 */
trep combine_dv_arr_like_bits(trep a, trep b) {
  auto check = [] (trep a, trep x) { return (a & (x | BNull)) == a; };
  assert(isPredefined(a) && !check(a, BNull));
  if (check(a, BPArr))   return combine_parr_bits(a,   b);
  if (check(a, BVArr))   return combine_varr_bits(a,   b);
  if (check(a, BDArr))   return combine_darr_bits(a,   b);
  if (check(a, BArr))    return combine_arr_bits(a,    b);
  if (check(a, BVec))    return combine_vec_bits(a,    b);
  if (check(a, BDict))   return combine_dict_bits(a,   b);
  if (check(a, BKeyset)) return combine_keyset_bits(a, b);
  not_reached();
}

trep maybe_promote_varray(trep a) {
  auto const check = [&] (trep b, trep c) {
    if (a & b) a |= c;
  };
  assert(isPredefined(a));
  check(BSVArrE, BSArrE);
  check(BCVArrE, BCArrE);
  check(BSVArrN, BSArrN);
  check(BCVArrN, BCArrN);
  assert(isPredefined(a));
  return a;
}

trep promote_varray(trep a) {
  auto const check = [&] (trep b, trep c) {
    if (a & b) a = (a | c) & ~b;
  };
  assert(isPredefined(a));
  // If the array is more than just a varray, we can't just switch the bits and
  // keep the combination predefined. Just use the maybe path which will keep
  // the bits predefined.
  if ((a & (BVArr | BNull)) != a) return maybe_promote_varray(a);
  check(BSVArrE, BSDArrE);
  check(BCVArrE, BCDArrE);
  check(BSVArrN, BSDArrN);
  check(BCVArrN, BCDArrN);
  assert(isPredefined(a));
  return a;
}

//////////////////////////////////////////////////////////////////////

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * For more info on what ProvTag is for and how it's represented, see
 * the comment in type-system.h on the using ProvTag declaration
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

/*
 * Determine if one provenance tag is a (non-strict) subtype of another
 */
bool subtypeProvTag(ProvTag p1, ProvTag p2) {
  return p2 == ProvTag::Top ||
    (p2 == ProvTag::SomeTag && p1 != ProvTag::NoTag) ||
    p1 == p2;
}

/*
 * Compute the union of two provenance tags
 */
ProvTag unionProvTag(ProvTag p1, ProvTag p2) {
  if (p1 == p2) {
    return p1;
  } else if (p1 == ProvTag::NoTag ||
             p2 == ProvTag::NoTag ||
             p1 == ProvTag::Top ||
             p2 == ProvTag::Top) {
    return ProvTag::Top;
  } else {
    return ProvTag::SomeTag;
  }
}

/*
 * Compute the intersection of two provenance tags
 *
 * XXX: If both p1 and p2 are specific tags and they differ, the result should
 * be bottom, but we widen to 'top' since it prevents provenance data from
 * producing bottoms and is still correct since intersections are allowed to be
 * wider than either input type.
 */
ProvTag intersectProvTag(ProvTag p1, ProvTag p2) {
  if (p1 == ProvTag::Top) {
    return p2;
  } else if (p2 == ProvTag::Top) {
    return p1;
  } else if (p1 == p2) {
    return p1;
  } else if (p1 == ProvTag::NoTag || p2 == ProvTag::NoTag) {
    return ProvTag::Top;
  } else if (p1 == ProvTag::SomeTag) {
    return p2;
  } else if (p2 == ProvTag::SomeTag) {
    return p1;
  } else {
    return ProvTag::Top;
  }
}

/*
 * Determine if p1 and p2 could possibly be in a subtype relationship (i.e.
 * their intersection is non-empty).
 *
 * For provenance tags, this only isn't the case if both tags are known and
 * they do not match.
 */
bool couldBeProvTag(ProvTag p1, ProvTag p2) {
  return p1 == ProvTag::Top ||
    p2 == ProvTag::Top ||
    (p1 == ProvTag::SomeTag && p2 != ProvTag::NoTag) ||
    (p2 == ProvTag::SomeTag && p1 != ProvTag::NoTag) ||
    p1 == p2;
}

//////////////////////////////////////////////////////////////////////

template <typename T, typename... Args>
void construct(T& dest, Args&&... src) {
  new (&dest) T { std::forward<Args>(src)... };
}

template <typename T>
void destroy(T& t) { t.~T(); }

template <typename T, typename... Args>
void construct_inner(T& dest, Args&&... args) {
  construct(dest);
  dest.emplace(std::forward<Args>(args)...);
}

/*
 * The following functions make DArr* structs out of static arrays, to
 * simplify implementing some of the type system operations on them.
 *
 * When they return folly::none it is not a conservative thing: it
 * implies the array is definitely not packed, packedN, struct-like,
 * etc (we use this to return false in couldBe).
 */

folly::Optional<DArrLikePacked> toDArrLikePacked(SArray ar) {
  assert(!ar->empty());

  std::vector<Type> elems;
  auto idx = size_t{0};
  for (ArrayIter iter(ar); iter; ++iter, ++idx) {
    auto const key = *iter.first().asTypedValue();
    if (key.m_type != KindOfInt64) return folly::none;
    if (key.m_data.num != idx)     return folly::none;
    elems.push_back(
      from_cell(iter.secondVal())
    );
  }

  return DArrLikePacked {
    std::move(elems),
    RuntimeOption::EvalArrayProvenance
      ? ProvTag::FromSArr(ar)
      : ProvTag::Top
  };
}

folly::Optional<DArrLikePackedN> toDArrLikePackedN(SArray ar) {
  assert(!ar->empty());

  auto t = TBottom;
  auto idx = int64_t{0};
  for (ArrayIter iter(ar); iter; ++iter, ++idx) {
    auto const key = *iter.first().asTypedValue();
    if (key.m_type != KindOfInt64) return folly::none;
    if (key.m_data.num != idx)     return folly::none;
    t |= from_cell(iter.secondVal());
  }

  return DArrLikePackedN { std::move(t) };
}

folly::Optional<DArrLikeMap> toDArrLikeMap(SArray ar) {
  assert(!ar->empty());
  auto map = MapElems{};
  auto idx = int64_t{0};
  auto packed = true;
  for (ArrayIter iter(ar); iter; ++iter) {
    auto const key = *iter.first().asTypedValue();
    if (packed) {
      packed = isIntType(key.m_type) && key.m_data.num == idx;
      ++idx;
    }
    auto const value = iter.secondVal();
    map.emplace_back(key, from_cell(value));
  }
  if (packed) return folly::none;
  return DArrLikeMap {
    std::move(map),
    TBottom,
    TBottom,
    RuntimeOption::EvalArrayProvenance
      ? ProvTag::FromSArr(ar)
      : ProvTag::Top
  };
}

folly::Optional<DArrLikeMapN> toDArrLikeMapN(SArray ar) {
  assert(!ar->empty());

  auto k = TBottom;
  auto v = TBottom;
  auto idx = int64_t{0};
  auto packed = true;
  for (ArrayIter iter(ar); iter; ++iter) {
    auto const key = *iter.first().asTypedValue();
    k |= from_cell(key);
    v |= from_cell(iter.secondVal());
    if (packed) {
      packed = isIntType(key.m_type) && key.m_data.num == idx;
      ++idx;
    }
  }

  if (packed || tv(k)) return folly::none;
  return DArrLikeMapN { std::move(k), std::move(v) };
}

//////////////////////////////////////////////////////////////////////

template<bool contextSensitive>
bool subtypePacked(const DArrLikePacked& a, const DArrLikePacked& b) {
  auto const asz = a.elems.size();
  auto const bsz = b.elems.size();
  if (asz != bsz) return false;
  if (!subtypeProvTag(a.provenance, b.provenance)) return false;
  for (auto i = size_t{0}; i < asz; ++i) {
    if (!a.elems[i].subtypeOfImpl<contextSensitive>(b.elems[i])) {
      return false;
    }
  }
  return true;
}

template<bool contextSensitive>
bool subtypeMap(const DArrLikeMap& a, const DArrLikeMap& b) {
  // If both A and B both don't have optional elements, their values
  // are completely disjoint if there's a different number of keys.
  if (!a.hasOptElements() && !b.hasOptElements() &&
      a.map.size() != b.map.size()) {
    return false;
  }
  if (!subtypeProvTag(a.provenance, b.provenance)) return false;

  // Check the common prefix of known keys. The keys must be the same
  // and have compatible types.
  auto aIt = begin(a.map);
  auto bIt = begin(b.map);
  while (aIt != end(a.map) && bIt != end(b.map)) {
    if (!tvSame(aIt->first, bIt->first)) return false;
    if (!aIt->second.subtypeOfImpl<contextSensitive>(bIt->second)) return false;
    ++aIt;
    ++bIt;
  }
  // If B has more known keys than A, A cannot be a subtype of B. It
  // doesn't matter if A has optional elements since there's values in
  // A which doesn't have them, while B always has the keys.
  if (bIt != end(b.map)) return false;

  // If A has any remaining known keys, check they're compatible with
  // B's optional elements (if any).
  while (aIt != end(a.map)) {
    if (!from_cell(aIt->first).subtypeOfImpl<contextSensitive>(b.optKey)) {
      return false;
    }
    if (!aIt->second.subtypeOfImpl<contextSensitive>(b.optVal)) return false;
    ++aIt;
  }

  // Finally the optional values (if any) of A and B must be
  // compatible.
  return
    a.optKey.subtypeOfImpl<contextSensitive>(b.optKey) &&
    a.optVal.subtypeOfImpl<contextSensitive>(b.optVal);
}

bool couldBePacked(const DArrLikePacked& a, const DArrLikePacked& b) {
  auto const asz = a.elems.size();
  auto const bsz = b.elems.size();
  if (asz != bsz) return false;
  if (!couldBeProvTag(a.provenance, b.provenance)) return false;
  for (auto i = size_t{0}; i < asz; ++i) {
    if (!a.elems[i].couldBe(b.elems[i])) {
      return false;
    }
  }
  return true;
}

bool couldBeMap(const DArrLikeMap& a, const DArrLikeMap& b) {
  // If A and B don't have optional elements, their values are
  // completely disjoint if the number of keys are different.
  if (!a.hasOptElements() && !b.hasOptElements() &&
      a.map.size() != b.map.size()) {
    return false;
  }
  if (!couldBeProvTag(a.provenance, b.provenance)) return false;

  // Check the common prefix of A and B. The keys must be the same and
  // there must be an intersection among the values.
  auto aIt = begin(a.map);
  auto bIt = begin(b.map);
  while (aIt != end(a.map) && bIt != end(b.map)) {
    if (!tvSame(aIt->first, bIt->first)) return false;
    if (!aIt->second.couldBe(bIt->second)) return false;
    ++aIt;
    ++bIt;
  }

  // Process the remaining known keys (only A or B will be
  // processed). The known keys must have an intersection with the
  // other map's optional values (if any).
  while (aIt != end(a.map)) {
    if (!from_cell(aIt->first).couldBe(b.optKey)) return false;
    if (!aIt->second.couldBe(b.optVal)) return false;
    ++aIt;
  }

  while (bIt != end(b.map)) {
    if (!from_cell(bIt->first).couldBe(a.optKey)) return false;
    if (!bIt->second.couldBe(a.optVal)) return false;
    ++bIt;
  }

  return true;
}

namespace {

/*
 * Walk a pair of arrays (already known to be tvSame) to compare the provenance
 * tags in each of their subarrays. Calls the given function on each matched
 * pair of provenance tags and returns false if any of these comparisions
 * returns false.
 */
template <typename Visit>
bool visitDifferingProvTagsInSimilarArrays(SArray arr1,
                                           SArray arr2,
                                           Visit&& visit) {
  if (!RuntimeOption::EvalArrayProvenance) return true;
  if (!visit(ProvTag::FromSArr(arr1), ProvTag::FromSArr(arr2))) return false;
  bool ret = true;
  IterateKV(
    arr1,
    [&](TypedValue k, TypedValue tv1) {
      auto const bail = [&] {
        ret = false;
        return true;
      };
      assert(arr2->exists(k));
      auto tv2 = arr2->get(k).tv();
      assert(type(tv1) == type(tv2));
      if (isArrayLikeType(type(tv1))) {
        if (!visitDifferingProvTagsInSimilarArrays(
              tv1.m_data.parr,
              tv2.m_data.parr,
              std::forward<Visit>(visit)
            )) {
          return bail();
        }
      }
      return false;
    }
  );
  return ret;
}

}

bool subtypeArrLike(SArray arr1, SArray arr2) {
  if (arr1 == arr2) return true;
  if (arr1->size() != arr2->size()) return false;
  SuppressHACCompareNotices _;
  if (!arr1->equal(arr2, true)) return false;
  return visitDifferingProvTagsInSimilarArrays(arr1, arr2, subtypeProvTag);
}

bool couldBeArrLike(SArray arr1, SArray arr2) {
  if (arr1 == arr2) return true;
  if (arr1->size() != arr2->size()) return false;
  SuppressHACCompareNotices _;
  if (!arr1->equal(arr2, true)) return false;
  return visitDifferingProvTagsInSimilarArrays(arr1, arr2, couldBeProvTag);
}

//////////////////////////////////////////////////////////////////////

std::pair<Type,Type> val_key_values(SArray a) {
  auto ret = std::make_pair(TBottom, TBottom);
  for (ArrayIter iter(a); iter; ++iter) {
    ret.first |= from_cell(*iter.first().asTypedValue());
    ret.second |= from_cell(iter.secondVal());
  }
  return ret;
}

std::pair<Type,Type> map_key_values(const DArrLikeMap& a) {
  auto ret = std::make_pair(a.optKey, a.optVal);
  for (auto const& kv : a.map) {
    ret.first |= from_cell(kv.first);
    ret.second |= kv.second;
  }
  return ret;
}

Type packed_values(const DArrLikePacked& a) {
  auto ret = TBottom;
  for (auto const& e : a.elems) ret |= e;
  return ret;
}

//////////////////////////////////////////////////////////////////////
template <typename T>
struct DataTagTrait {};

template<> struct DataTagTrait<SString>      { using tag = SString; };

template<> struct DataTagTrait<DArrLikePacked>  { using tag = SArray; };
template<> struct DataTagTrait<DArrLikePackedN> { using tag = SArray; };
template<> struct DataTagTrait<DArrLikeMap>     { using tag = SArray; };
template<> struct DataTagTrait<DArrLikeMapN>    { using tag = SArray; };
template<> struct DataTagTrait<SArray>          { using tag = SArray; };

/*
 * Helper for dealing with dualDispatchDataFn's---most of them are commutative.
 * This shuffles values to the right in a canonical order to need less
 * overloads.
 */
template<class InnerFn>
struct Commute : InnerFn {

  template<class... Args>
  explicit Commute(Args&&... args) : InnerFn(std::forward<Args>(args)...) {}

  using result_type = typename InnerFn::result_type;

  using InnerFn::operator();

  template<class T1, class T2>
  typename std::enable_if<!std::is_same<T1, T2>::value &&
                          std::is_same<typename DataTagTrait<T1>::tag,
                                       typename DataTagTrait<T2>::tag>::value,
                          result_type>::type
  operator()(const T1& a, const T2& b) const {
    return InnerFn::operator()(b, a);
  }

};

template<bool contextSensitive>
struct DualDispatchEqImpl {
  static constexpr bool disjoint = true;
  using result_type = bool;

  bool operator()() const { return false; }

  bool operator()(const DArrLikePacked& a, SArray b) const {
    if (a.elems.size() != b->size()) return false;
    if (RuntimeOption::EvalArrayProvenance) {
      auto const tag = ProvTag::FromSArr(b);
      if (a.provenance != tag) return false;
    }
    auto const p = toDArrLikePacked(b);
    if (!p) return false;
    for (auto i = 0; i < a.elems.size(); i++) {
      if (!a.elems[i].equivImpl<contextSensitive>(p->elems[i])) return false;
    }
    return true;
  }

  bool operator()(const DArrLikeMap& a, SArray b) const {
    // A map with optional elements contains multiple values, so it
    // can never be equal to a static array (which is just one value).
    if (a.hasOptElements()) return false;
    if (a.map.size() != b->size()) return false;
    if (RuntimeOption::EvalArrayProvenance) {
      auto const tag = ProvTag::FromSArr(b);
      if (a.provenance != tag) return false;
    }
    auto const m = toDArrLikeMap(b);
    if (!m) return false;
    auto it = m->map.begin();
    for (auto const& kv : a.map) {
      if (!ArrayLikeMapEqual{}(kv.first, it->first)) return false;
      if (!kv.second.equivImpl<contextSensitive>(it->second)) return false;
      ++it;
    }
    return true;
  }

  bool operator()(const DArrLikePackedN& /*a*/, SArray /*b*/) const {
    return false;
  }
  bool operator()(const DArrLikeMapN& /*a*/, SArray /*b*/) const {
    return false;
  }
  bool
  operator()(const DArrLikePacked& /*a*/, const DArrLikePackedN& /*b*/) const {
    return false;
  }
  bool operator()(const DArrLikePacked& /*a*/, const DArrLikeMap& /*b*/) const {
    return false;
  }
  bool operator()(const DArrLikePacked&, const DArrLikeMapN&) const {
    return false;
  }

  bool
  operator()(const DArrLikePackedN& /*a*/, const DArrLikeMap& /*b*/) const {
    return false;
  }
  bool operator()(const DArrLikePackedN&, const DArrLikeMapN&) const {
    return false;
  }
  bool operator()(const DArrLikeMap&, const DArrLikeMapN&) const {
    return false;
  }
};

struct DualDispatchCouldBeImpl {
  static constexpr bool disjoint = true;
  using result_type = bool;

  bool operator()() const { return false; }

  bool operator()(const DArrLikePacked& a, SArray b) const {
    if (a.elems.size() != b->size()) return false;
    if (RuntimeOption::EvalArrayProvenance) {
      auto const tag = ProvTag::FromSArr(b);
      if (!couldBeProvTag(a.provenance, tag)) return false;
    }
    auto const p = toDArrLikePacked(b);
    return p && couldBePacked(a, *p);
  }

  bool operator()(const DArrLikeMap& a, SArray b) const {
    // If A doesn't have optional elements, there's no intersection if
    // they have a different number of known keys.
    if (!a.hasOptElements() && a.map.size() != b->size()) return false;
    if (RuntimeOption::EvalArrayProvenance) {
      auto const tag = ProvTag::FromSArr(b);
      if (!couldBeProvTag(a.provenance, tag)) return false;
    }
    auto const m = toDArrLikeMap(b);
    return m && couldBeMap(a, *m);
  }

  bool operator()(const DArrLikePackedN& a, SArray b) const {
    auto const p = toDArrLikePackedN(b);
    return p && a.type.couldBe(p->type);
  }

  bool operator()(const DArrLikeMapN& a, SArray b) const {
    assert(!b->empty());
    bool bad = false;
    IterateKV(
      b,
      [&] (TypedValue k, TypedValue v) {
        bad |= !(a.key.couldBe(from_cell(k)) && a.val.couldBe(from_cell(v)));
        return bad;
      }
    );
    return !bad;
  }

  bool operator()(const DArrLikePacked& a, const DArrLikePackedN& b) const {
    for (auto const& t : a.elems) {
      if (!t.couldBe(b.type)) return false;
    }
    return true;
  }

  bool operator()(const DArrLikePackedN& a, const DArrLikeMapN& b) const {
    return TInt.couldBe(b.key) && a.type.couldBe(b.val);
  }

  bool operator()(const DArrLikeMap& a, const DArrLikeMapN& b) const {
    for (auto const& kv : a.map) {
      if (!from_cell(kv.first).couldBe(b.key)) return false;
      if (!kv.second.couldBe(b.val)) return false;
    }
    // We can ignore optional elements here. If the values
    // corresponding to just the known keys already intersect with B,
    // then we have an intersection so we're done.
    return true;
  }

  bool operator()(const DArrLikePacked& /*a*/, const DArrLikeMap& /*b*/) const {
    // Map does not contain any packed arrays.
    return false;
  }
  bool operator()(const DArrLikePacked& a, const DArrLikeMapN& b) const {
    if (!TInt.couldBe(b.key)) return false;
    for (auto const& t : a.elems) {
      if (!t.couldBe(b.val)) return false;
    }
    return true;
  }
  bool
  operator()(const DArrLikePackedN& /*a*/, const DArrLikeMap& /*b*/) const {
    // Map does not contain any packed arrays.
    return false;
  }
};

// pre: neither type is a subtype of the other
struct DualDispatchIntersectionImpl {
  static constexpr bool disjoint = false;
  using result_type = Type;

  explicit DualDispatchIntersectionImpl(trep b) : bits(b) {}

  Type operator()() const { not_reached(); }

  template <typename F>
  Type intersect_packed(std::vector<Type> elems, F next, ProvTag tag) const {
    for (auto& e : elems) {
      e &= next();
      if (e == TBottom) return TBottom;
    }
    return packed_impl(bits, std::move(elems), tag);
  }

  // The SArray is known to not be a subtype, so the intersection must be empty
  Type operator()(const DArrLikePacked& /*a*/, const SArray /*b*/) const {
    return TBottom;
  }
  Type operator()(const DArrLikePackedN& /*a*/, const SArray /*b*/) const {
    return TBottom;
  }
  Type operator()(const DArrLikeMapN& /*a*/, const SArray /*b*/) const {
    return TBottom;
  }
  Type operator()(const DArrLikeMap& /*a*/, const SArray /*b*/) const {
    return TBottom;
  }
  Type operator()(const SArray /*a*/, const SArray /*b*/) const {
    return TBottom;
  }

  Type operator()(const DArrLikePacked& a, const DArrLikePacked& b) const {
    if (a.elems.size() != b.elems.size()) return TBottom;

    auto i = size_t{};
    return intersect_packed(
      a.elems,
      [&] { return b.elems[i++]; },
      intersectProvTag(a.provenance, b.provenance)
    );
  }
  Type operator()(const DArrLikePacked& a, const DArrLikePackedN& b) const {
    return intersect_packed(a.elems, [&] { return b.type; }, a.provenance);
  }
  Type operator()(const DArrLikePacked& a, const DArrLikeMapN& b) const {
    if (b.key.couldBe(BInt)) {
      return intersect_packed(a.elems, [&] { return b.val; }, a.provenance);
    }
    return TBottom;
  }
  Type operator()(const DArrLikePacked& /*a*/, const DArrLikeMap& /*b*/) const {
    // We don't allow DArrLikeMaps which are packed
    return TBottom;
  }

  Type operator()(const DArrLikePackedN& a, const DArrLikePackedN& b) const {
    auto isect = intersection_of(a.type, b.type);
    if (isect == TBottom) return TBottom;
    return packedn_impl(bits, isect);
  }
  Type operator()(const DArrLikePackedN& a, const DArrLikeMapN& b) const {
    if (b.key.couldBe(BInt)) {
      auto val = intersection_of(b.val, a.type);
      if (val != TBottom) return packedn_impl(bits, std::move(val));
    }
    return TBottom;
  }
  Type
  operator()(const DArrLikePackedN& /*a*/, const DArrLikeMap& /*b*/) const {
    return TBottom;
  }

  Type operator()(const DArrLikeMapN& a, const DArrLikeMapN& b) const {
    auto k = intersection_of(a.key, b.key);
    auto v = intersection_of(a.val, b.val);
    if (k == TBottom || v == TBottom) return TBottom;
    return mapn_impl(bits, k, v, ProvTag::Top);
  }
  Type operator()(const DArrLikeMapN& a, const DArrLikeMap& b) const {
    auto map = MapElems{};

    for (auto const& kv : b.map) {
      if (!from_cell(kv.first).couldBe(a.key)) return TBottom;
      auto val = intersection_of(kv.second, a.val);
      if (val == TBottom) return TBottom;
      map.emplace_back(kv.first, std::move(val));
    }

    auto optKey = TBottom;
    auto optVal = TBottom;
    if (b.hasOptElements()) {
      optKey = intersection_of(b.optKey, a.key);
      optVal = intersection_of(b.optVal, a.val);
      if (optKey == TBottom || optVal == TBottom) {
        optKey = TBottom;
        optVal = TBottom;
      }
    }

    return map_impl(
      bits,
      std::move(map),
      std::move(optKey),
      std::move(optVal),
      b.provenance
    );
  }

  Type operator()(const DArrLikeMap& a, const DArrLikeMap& b) const {
    // Two maps without optional elements have no values in common if
    // they have different sets of keys.
    if (!a.hasOptElements() && !b.hasOptElements() &&
        a.map.size() != b.map.size()) {
      return TBottom;
    }

    auto map = MapElems{};

    auto aIt = begin(a.map);
    auto bIt = begin(b.map);

    // Intersect the common known keys
    while (aIt != end(a.map) && bIt != end(b.map)) {
      if (!tvSame(aIt->first, bIt->first)) return TBottom;
      auto val = intersection_of(aIt->second, bIt->second);
      if (val == TBottom) return TBottom;
      map.emplace_back(aIt->first, std::move(val));
      ++aIt;
      ++bIt;
    }

    // Any remaining known keys are only in A, or in B. Intersect them
    // with the optional elements in the other map. If the other map
    // doesn't have optional elements, the intersection will be
    // Bottom.
    while (aIt != end(a.map)) {
      if (!from_cell(aIt->first).couldBe(b.optKey)) return TBottom;
      auto val = intersection_of(aIt->second, b.optVal);
      if (val == TBottom) return TBottom;
      map.emplace_back(aIt->first, std::move(val));
      ++aIt;
    }

    while (bIt != end(b.map)) {
      if (!from_cell(bIt->first).couldBe(a.optKey)) return TBottom;
      auto val = intersection_of(bIt->second, a.optVal);
      if (val == TBottom) return TBottom;
      map.emplace_back(bIt->first, std::move(val));
      ++bIt;
    }

    auto optKey = TBottom;
    auto optVal = TBottom;
    if (a.hasOptElements() && b.hasOptElements()) {
      optKey = intersection_of(a.optKey, b.optKey);
      optVal = intersection_of(a.optVal, b.optVal);
      if (optKey == TBottom || optVal == TBottom) {
        optKey = TBottom;
        optVal = TBottom;
      }
    }

    return map_impl(
      bits,
      std::move(map),
      std::move(optKey),
      std::move(optVal),
      intersectProvTag(a.provenance, b.provenance)
    );
  }

  Type operator()(const SString /*a*/, const SString /*b*/) const {
    not_reached();
  }
private:
  trep bits;
};

struct DualDispatchUnionImpl {
  static constexpr bool disjoint = false;
  using result_type = Type;

  explicit DualDispatchUnionImpl(trep b) : bits(b) {}

  Type operator()() const { not_reached(); }

  Type operator()(const DArrLikePacked& a, const DArrLikePacked& b) const {
    if (a.elems.size() != b.elems.size()) {
      return packedn_impl(bits, union_of(packed_values(a), packed_values(b)));
    }
    auto ret = a.elems;
    for (auto i = size_t{0}; i < a.elems.size(); ++i) {
      ret[i] |= b.elems[i];
    }
    return packed_impl(
      bits,
      std::move(ret),
      unionProvTag(a.provenance, b.provenance)
    );
  }

  Type operator()(const DArrLikePackedN& a, const DArrLikePackedN& b) const {
    return packedn_impl(bits, union_of(a.type, b.type));
  }

  Type operator()(const DArrLikePacked& a, const DArrLikePackedN& b) const {
    return (*this)(DArrLikePackedN { packed_values(a) }, b);
  }

  Type operator()(const DArrLikeMap& a, const DArrLikeMap& b) const {
    auto map = MapElems{};

    // Keep the common prefix of known keys in both A and B.
    auto aIt = begin(a.map);
    auto bIt = begin(b.map);
    while (aIt != end(a.map) && bIt != end(b.map)) {
      if (!tvSame(aIt->first, bIt->first)) break;
      map.emplace_back(aIt->first, union_of(aIt->second, bIt->second));
      ++aIt;
      ++bIt;
    }

    // If there's no common prefix, fall back to MapN.
    if (map.empty()) {
      auto mkva = map_key_values(a);
      auto mkvb = map_key_values(b);
      return mapn_impl_from_map(
        bits,
        union_of(std::move(mkva.first), std::move(mkvb.first)),
        union_of(std::move(mkva.second), std::move(mkvb.second)),
        unionProvTag(a.provenance, b.provenance)
      );
    }

    // Any non-common known keys will be combined into the optional
    // elements.
    auto optKey = union_of(a.optKey, b.optKey);
    auto optVal = union_of(a.optVal, b.optVal);

    while (aIt != end(a.map)) {
      optKey |= from_cell(aIt->first);
      optVal |= aIt->second;
      ++aIt;
    }
    while (bIt != end(b.map)) {
      optKey |= from_cell(bIt->first);
      optVal |= bIt->second;
      ++bIt;
    }

    return map_impl(
      bits,
      std::move(map),
      std::move(optKey),
      std::move(optVal),
      unionProvTag(a.provenance, b.provenance)
    );
  }

  Type operator()(SArray a, SArray b) const {
    assert(a != b); // Should've been handled earlier in union_of.
    if (a->empty() && b->empty()) {
      auto const aTag = ProvTag::FromSArr(a);
      auto const bTag = ProvTag::FromSArr(b);
      assert(aTag != bTag ||
             a->kind() != b->kind());

      if (a->kind() != b->kind() || a->dvArray() != b->dvArray()) {
        return Type { bits };
      }

      if (!arrprov::arrayWantsTag(a)) return Type { bits };

      auto r = [&] {
        auto const tag = unionProvTag(aTag, bTag);
        if (a->isVecArrayType()) return vec_empty(tag);
        if (a->isDictType()) return dict_empty(tag);
        if (a->isVArray()) return aempty_varray(tag);
        if (a->isDArray()) return aempty_darray(tag);
        always_assert(false);
      }();
      set_trep(r, bits);
      return r;
    }

    auto const p1 = toDArrLikePacked(a);
    auto const p2 = toDArrLikePacked(b);
    assert(!(bits & BVec) || (p1 && p2));

    if (p1) {
      if (p2) return (*this)(*p1, *p2);
      return (*this)(*p1, *toDArrLikeMap(b));
    } else if (p2) {
      return (*this)(*p2, *toDArrLikeMap(a));
    } else {
      return (*this)(*toDArrLikeMap(a), *toDArrLikeMap(b));
    }
  }

  Type operator()(const DArrLikeMapN& a, const DArrLikeMapN& b) const {
    return mapn_impl(
      bits,
      union_of(a.key, b.key),
      union_of(a.val, b.val),
      ProvTag::Top
    );
  }

  Type operator()(const DArrLikePacked& a, SArray b) const {
    assert(!b->empty());
    auto const p = toDArrLikePacked(b);
    if (p) return (*this)(a, *p);
    return (*this)(a, *toDArrLikeMap(b));
  }

  Type operator()(const DArrLikePackedN& a, SArray b) const {
    assert(!b->empty());
    auto const p = toDArrLikePackedN(b);
    if (p) return (*this)(a, *p);
    return (*this)(a, *toDArrLikeMap(b));
  }

  Type operator()(const DArrLikeMap& a, SArray b) const {
    assert(!b->empty());
    auto const m = toDArrLikeMap(b);
    if (m) return (*this)(a, *m);
    return (*this)(*toDArrLikePacked(b), a);
  }

  Type operator()(const DArrLikeMapN& a, SArray b) const {
    assert(!b->empty());
    auto const m1 = toDArrLikeMapN(b);
    if (m1) return (*this)(a, *m1);
    auto const m2 = toDArrLikeMap(b);
    if (m2) return (*this)(*m2, a);
    return (*this)(*toDArrLikePackedN(b), a);
  }

  Type operator()(const DArrLikePacked& a, const DArrLikeMap& b) const {
    auto mkv = map_key_values(b);
    return mapn_impl_from_map(
      bits,
      union_of(TInt, std::move(mkv.first)),
      union_of(packed_values(a), std::move(mkv.second)),
      unionProvTag(a.provenance, b.provenance)
    );
  }

  Type operator()(const DArrLikePacked& a, const DArrLikeMapN& b) const {
    return mapn_impl(
      bits, union_of(b.key, TInt),
      union_of(packed_values(a), b.val),
      ProvTag::Top
    );
  }

  Type operator()(const DArrLikePackedN& a, const DArrLikeMap& b) const {
    auto mkv = map_key_values(b);
    return mapn_impl_from_map(
      bits,
      union_of(TInt, std::move(mkv.first)),
      union_of(a.type, std::move(mkv.second)),
      ProvTag::Top
    );
  }

  Type operator()(const DArrLikePackedN& a, const DArrLikeMapN& b) const {
    return mapn_impl(
      bits,
      union_of(TInt, b.key),
      union_of(a.type, b.val),
      ProvTag::Top
    );
  }

  Type operator()(const DArrLikeMap& a, const DArrLikeMapN& b) const {
    auto mkv = map_key_values(a);
    return mapn_impl_from_map(
      bits,
      union_of(std::move(mkv.first), b.key),
      union_of(std::move(mkv.second), b.val),
      ProvTag::Top
    );
  }

  Type operator()(const SString /*a*/, const SString /*b*/) const {
    not_reached();
  }

private:
  trep bits;
};

/*
 * Subtype is not a commutative relation, so this is the only
 * dualDispatchDataFn helper that doesn't use Commute<>.
 */
template<bool contextSensitive>
struct DualDispatchSubtype {
  static constexpr bool disjoint = true;
  using result_type = bool;

  bool operator()() const { return false; }

  bool operator()(const DArrLikeMap& a, SArray b) const {
    // A map with optional elements contains more than one value,
    // while a static array is always one value.
    if (a.hasOptElements()) return false;
    if (a.map.size() != b->size()) return false;
    auto const m = toDArrLikeMap(b);
    return m && subtypeMap<contextSensitive>(a, *m);
  }

  bool operator()(SArray a, const DArrLikeMap& b) const {
    // If the map doesn't have optional elements, its values are
    // disjoint from the static array if the keys are of different
    // sizes.
    if (!b.hasOptElements() && a->size() != b.map.size()) return false;
    auto const m = toDArrLikeMap(a);
    return m && subtypeMap<contextSensitive>(*m, b);
  }

  bool operator()(SArray a, const DArrLikePacked& b) const {
    if (a->size() != b.elems.size()) return false;
    auto const p = toDArrLikePacked(a);
    return p && subtypePacked<contextSensitive>(*p, b);
  }

  bool operator()(const DArrLikePacked& a, SArray b) const {
    if (a.elems.size() != b->size()) return false;
    auto const p = toDArrLikePacked(b);
    return p && subtypePacked<contextSensitive>(a, *p);
  }

  bool operator()(const DArrLikePackedN& a, const DArrLikeMapN& b) const {
    return b.key.couldBe(BInt) && a.type.subtypeOfImpl<contextSensitive>(b.val);
  }

  bool operator()(const DArrLikePacked& a, const DArrLikeMapN& b) const {
    if (!b.key.couldBe(BInt)) return false;
    for (auto const& v : a.elems) {
      if (!v.subtypeOfImpl<contextSensitive>(b.val)) return false;
    }
    return true;
  }

  bool operator()(const DArrLikeMap& a, const DArrLikeMapN& b) const {
    for (auto const& kv : a.map) {
      if (!from_cell(kv.first).subtypeOfImpl<contextSensitive>(b.key)) {
        return false;
      }
      if (!kv.second.subtypeOfImpl<contextSensitive>(b.val)) return false;
    }
    return
      a.optKey.subtypeOfImpl<contextSensitive>(b.key) &&
      a.optVal.subtypeOfImpl<contextSensitive>(b.val);
  }

  bool operator()(SArray a, const DArrLikeMapN& b) const {
    assert(!a->empty()); // if a is empty, we should have projected b to top
    bool bad = false;
    IterateKV(
      a,
      [&] (TypedValue k, TypedValue v) {
        bad |= !(b.key.couldBe(from_cell(k)) && b.val.couldBe(from_cell(v)));
        return bad;
      }
    );
    return !bad;
  }

  bool operator()(const DArrLikePacked& a, const DArrLikePackedN& b) const {
    for (auto const& t : a.elems) {
      if (!t.subtypeOfImpl<contextSensitive>(b.type)) return false;
    }
    return true;
  }

  bool operator()(SArray a, const DArrLikePackedN& b) const {
    assert(!a->empty()); // if a is empty, we should have projected b to top
    auto p = toDArrLikePackedN(a);
    return p && p->type.subtypeOfImpl<contextSensitive>(b.type);
  }

  bool operator()(const DArrLikePackedN&, const DArrLikePacked&) const {
    // PackedN contains arrays with an arbitrary number of keys, while Packed
    // contains arrays with a fixed number of keys, so there's always arrays in
    // PackedN which aren't in Packed.
    return false;
  }
  bool operator()(const DArrLikePackedN&, SArray) const {
    // PackedN contains arrays with an arbitrary number of keys, while SArray is
    // just a single array.
    return false;
  }
  bool operator()(const DArrLikeMap& /*a*/, const DArrLikePacked& /*b*/) const {
    // Map does not contain any packed arrays.
    return false;
  }
  bool
  operator()(const DArrLikeMap& /*a*/, const DArrLikePackedN& /*b*/) const {
    // Map does not contain any packed arrays.
    return false;
  }
  bool operator()(const DArrLikePacked& /*a*/, const DArrLikeMap& /*b*/) const {
    // Map does not contain any packed arrays.
    return false;
  }
  bool operator()(const DArrLikePackedN&, const DArrLikeMap&) const {
    // Map does not contain any packed arrays.
    return false;
  }
  bool operator()(const DArrLikeMapN&, const DArrLikePackedN&) const {
    // MapN will always contain more arrays than PackedN because packed arrays
    // are a subset of all possible arrays.
    return false;
  }
  bool operator()(const DArrLikeMapN&, const DArrLikePacked&) const {
    // MapN contains arrays with an arbitrary number of keys, while Packed
    // contains arrays with a fixed number of keys, so there's always arrays in
    // MapN which aren't in Packed.
    return false;
  }
  bool operator()(const DArrLikeMapN&, const DArrLikeMap&) const {
    // MapN contains arrays with an arbitrary number of keys, while Map contains
    // arrays with a fixed number of keys, so there's always arrays in MapN
    // which aren't in Map.
    return false;
  }
  bool operator()(const DArrLikeMapN&, SArray) const {
    // MapN contains arrays with an arbitrary number of keys, while SArray is
    // just a single array.
    return false;
  }
};

template<bool contextSensitive>
using DualDispatchEq           = Commute<DualDispatchEqImpl<contextSensitive>>;
using DualDispatchCouldBe      = Commute<DualDispatchCouldBeImpl>;
using DualDispatchUnion        = Commute<DualDispatchUnionImpl>;
using DualDispatchIntersection = Commute<DualDispatchIntersectionImpl>;

//////////////////////////////////////////////////////////////////////
// Helpers for creating literal array-like types

template<typename AInit, bool force_static>
folly::Optional<TypedValue> fromTypeVec(const std::vector<Type> &elems,
                                  ProvTag tag) {
  ARRPROV_USE_RUNTIME_LOCATION();
  AInit ai(elems.size());
  for (auto const& t : elems) {
    auto const v = tv(t);
    if (!v) return folly::none;
    ai.append(tvAsCVarRef(&*v));
  }
  auto var = ai.toVariant();
  if (tag.valid()) {
    assertx(RuntimeOption::EvalArrayProvenance);
    arrprov::setTag<arrprov::Mode::Emplace>(var.asArrRef().get(), tag.get());
  }
  if (force_static) var.setEvalScalar();
  return tvReturn(std::move(var));
}

bool checkTypeVec(const std::vector<Type> &elems, ProvTag /*tag*/) {
  for (auto const& t : elems) {
    if (!is_scalar(t)) return false;
  }
  return true;
}

Variant keyHelper(SString key) {
  return Variant{ key, Variant::PersistentStrInit{} };
}
const Variant& keyHelper(const TypedValue& v) {
  return tvAsCVarRef(&v);
}
template <typename AInit>
void add(AInit& ai, const Variant& key, const Variant& value) {
  ai.setValidKey(key, value);
}
void add(KeysetInit& ai, const Variant& key, const Variant& value) {
  assert(tvSame(*key.asTypedValue(), *value.asTypedValue()));
  ai.add(key);
}

template<typename AInit, bool force_static, typename Key>
folly::Optional<TypedValue> fromTypeMap(const ArrayLikeMap<Key> &elems,
                                  ProvTag tag) {
  ARRPROV_USE_RUNTIME_LOCATION();
  auto val = eval_cell_value([&] () -> TypedValue {
    AInit ai(elems.size());
    for (auto const& elm : elems) {
      auto const v = tv(elm.second);
      if (!v) return make_tv<KindOfUninit>();
      add(ai, keyHelper(elm.first), tvAsCVarRef(&*v));
    }
    auto var = ai.toVariant();
    if (tag.valid()) {
      assertx(RuntimeOption::EvalArrayProvenance);
      assertx(arrprov::arrayWantsTag(var.asCArrRef().get()));
      arrprov::setTag<arrprov::Mode::Emplace>(var.asArrRef().get(), tag.get());
    }
    if (force_static) var.setEvalScalar();
    return tvReturn(std::move(var));
  });
  if (val && val->m_type == KindOfUninit) val.clear();
  return val;
}

template<typename Key>
bool checkTypeMap(const ArrayLikeMap<Key> &elems, ProvTag /*tag*/) {
  for (auto const& elm : elems) {
    if (!is_scalar(elm.second)) return false;
  }
  return true;
}

struct KeysetAppendInit : KeysetInit {
  using KeysetInit::KeysetInit;
  KeysetAppendInit& append(const Variant& v) {
    add(*v.asTypedValue());
    return *this;
  }
};

//////////////////////////////////////////////////////////////////////

}

//////////////////////////////////////////////////////////////////////
// Helpers for managing context types.

Type Type::unctxHelper(Type t, bool& changed) {
  assertx(t.checkInvariants());
  changed = false;
  switch (t.m_dataTag) {
  case DataTag::Obj:
    if (auto const whType = t.m_data.dobj.whType.get()) {
      auto ty = unctxHelper(*whType, changed);
      if (changed) {
        *t.m_data.dobj.whType.mutate() = ty;
      }
    }
    if (t.m_data.dobj.isCtx) {
      t.m_data.dobj.isCtx = false;
      changed = true;
    }
    break;
  case DataTag::Cls:
    if (t.m_data.dcls.isCtx) {
      t.m_data.dcls.isCtx = false;
      changed = true;
    }
    break;
  case DataTag::ArrLikePacked: {
    auto const packed = t.m_data.packed.get();
    HPHP::HHBBC::DArrLikePacked* mutated = nullptr;
    for (size_t i = 0; i < packed->elems.size(); ++i) {
      bool c;
      const auto ty = unctxHelper(packed->elems[i], c);
      if (c) {
        if (!mutated) {
          changed = true;
          mutated = t.m_data.packed.mutate();
        }
        mutated->elems[i] = ty;
      }
    }
    break;
  }
  case DataTag::ArrLikePackedN: {
    auto const packedn = t.m_data.packedn.get();
    auto ty = unctxHelper(packedn->type, changed);
    if (changed) {
      t.m_data.packedn.mutate()->type = ty;
    }
    break;
  }
  case DataTag::ArrLikeMap: {
    auto const map = t.m_data.map.get();
    size_t offset = 0;
    HPHP::HHBBC::DArrLikeMap* mutated = nullptr;
    for (auto it = map->map.begin(); it != map->map.end(); ++it) {
      auto const ty = unctxHelper(it->second, changed);
      if (changed) {
        offset = std::distance(it, map->map.begin());
        mutated = t.m_data.map.mutate();
        break;
      }
    }
    if (mutated) {
      auto it = mutated->map.begin();
      for (std::advance(it, offset);
           it != mutated->map.end();
           ++it) {
        bool c;
        auto ty = unctxHelper(it->second, c);
        if (c) {
          mutated->map.update(it, std::move(ty));
        }
      }
    }
    bool changed2;
    auto ty = unctxHelper(t.m_data.map->optVal, changed2);
    if (changed2) t.m_data.map.mutate()->optVal = std::move(ty);
    changed |= changed2;
    break;
  }
  case DataTag::ArrLikeMapN: {
    auto const mapn = t.m_data.mapn.get();
    auto ty = unctxHelper(mapn->val, changed);
    if (changed) t.m_data.mapn.mutate()->val = std::move(ty);
    break;
  }
  case DataTag::None:
  case DataTag::Int:
  case DataTag::Dbl:
  case DataTag::Str:
  case DataTag::ArrLikeVal:
  case DataTag::Record:
    break;
  }
  return t;
}

//////////////////////////////////////////////////////////////////////

Type::Type(const Type& o) noexcept
  : m_bits(o.m_bits)
  , m_dataTag(o.m_dataTag)
{
  SCOPE_EXIT { assert(checkInvariants()); };
  switch (m_dataTag) {
    case DataTag::None:   return;
    #define DT(tag_name,type,name)              \
      case DataTag::tag_name:                   \
        construct(m_data.name, o.m_data.name);  \
        return;
    DATATAGS
    #undef DT
  }
  not_reached();
}

Type::Type(Type&& o) noexcept
  : m_bits(o.m_bits)
  , m_dataTag(o.m_dataTag)
{
  SCOPE_EXIT { assert(checkInvariants());
               assert(o.checkInvariants()); };
  o.m_dataTag = DataTag::None;
  switch (m_dataTag) {
    case DataTag::None:   return;
    #define DT(tag_name,type,name)                              \
      case DataTag::tag_name:                                   \
        construct(m_data.name, std::move(o.m_data.name));       \
        return;
    DATATAGS
    #undef DT
  }
  not_reached();
}

Type& Type::operator=(const Type& o) noexcept {
  SCOPE_EXIT { assert(checkInvariants()); };
  destroy(*this);
  construct(*this, o);
  return *this;
}

Type& Type::operator=(Type&& o) noexcept {
  SCOPE_EXIT { assert(checkInvariants());
               assert(o.checkInvariants()); };
  destroy(*this);
  construct(*this, std::move(o));
  return *this;
}

Type::~Type() noexcept {
  assert(checkInvariants());

  switch (m_dataTag) {
    case DataTag::None: return;
    #define DT(tag_name,type,name)              \
      case DataTag::tag_name:                   \
        destroy(m_data.name);                   \
        return;
    DATATAGS
    #undef DT
  }
  not_reached();
}

const Type& Type::operator |= (const Type& other) {
  *this = union_of(std::move(*this), other);
  return *this;
}

const Type& Type::operator |= (Type&& other) {
  *this = union_of(std::move(*this), std::move(other));
  return *this;
}

const Type& Type::operator &= (const Type& other) {
  *this = intersection_of(std::move(*this), other);
  return *this;
}

const Type& Type::operator &= (Type&& other) {
  *this = intersection_of(std::move(*this), std::move(other));
  return *this;
}

//////////////////////////////////////////////////////////////////////

template<class Ret, class T, class Function>
struct Type::DDHelperFn {
  template <class Y>
  typename std::enable_if<(!std::is_same<Y,T>::value || !Function::disjoint) &&
                          std::is_same<typename DataTagTrait<Y>::tag,
                                       typename DataTagTrait<T>::tag>::value,
                          Ret>::type
  operator()(const Y& y) const { return f(t, y); }

  template <class Y>
  typename std::enable_if<(std::is_same<Y,T>::value && Function::disjoint) &&
                          std::is_same<typename DataTagTrait<Y>::tag,
                                       typename DataTagTrait<T>::tag>::value,
                          Ret>::type
  operator()(const Y& /*y*/) const { not_reached(); }

  template <class Y>
  typename std::enable_if<!std::is_same<typename DataTagTrait<Y>::tag,
                                        typename DataTagTrait<T>::tag>::value,
                          Ret>::type
  operator()(const Y& /*y*/) const { return f(); }

  Ret operator()() const { return f(); }
  Function f;
  const T& t;
};

template<class Ret, class T, class Function>
Type::DDHelperFn<Ret,T,Function> Type::ddbind(const Function& f,
                                              const T& t) const {
  return { f, t };
}

// Dispatcher for the second argument for dualDispatchDataFn.
template<class Ret, class T, class Function>
Ret Type::dd2nd(const Type& o, DDHelperFn<Ret,T,Function> f) const {
  switch (o.m_dataTag) {
  case DataTag::None:           not_reached();
  case DataTag::Obj:            return f();
  case DataTag::Int:            return f();
  case DataTag::Dbl:            return f();
  case DataTag::Cls:            return f();
  case DataTag::Record:         return f();
  case DataTag::Str:            return f(o.m_data.sval);
  case DataTag::ArrLikeVal:     return f(o.m_data.aval);
  case DataTag::ArrLikePacked:  return f(*o.m_data.packed);
  case DataTag::ArrLikePackedN: return f(*o.m_data.packedn);
  case DataTag::ArrLikeMap:     return f(*o.m_data.map);
  case DataTag::ArrLikeMapN:    return f(*o.m_data.mapn);
  }
  not_reached();
}

/*
 * Dual-dispatch on (this->m_dataTag, o.m_dataTag)
 *
 * See the DualDispatch* classes above
 */
template<class Function>
typename Function::result_type
Type::dualDispatchDataFn(const Type& o, Function f) const {
  using R = typename Function::result_type;
  switch (m_dataTag) {
  case DataTag::None:           not_reached();
  case DataTag::Obj:            return f();
  case DataTag::Int:            return f();
  case DataTag::Dbl:            return f();
  case DataTag::Cls:            return f();
  case DataTag::Record:         return f();
  case DataTag::Str:            return dd2nd(o, ddbind<R>(f, m_data.sval));
  case DataTag::ArrLikeVal:     return dd2nd(o, ddbind<R>(f, m_data.aval));
  case DataTag::ArrLikePacked:  return dd2nd(o, ddbind<R>(f, *m_data.packed));
  case DataTag::ArrLikePackedN: return dd2nd(o, ddbind<R>(f, *m_data.packedn));
  case DataTag::ArrLikeMap:     return dd2nd(o, ddbind<R>(f, *m_data.map));
  case DataTag::ArrLikeMapN:    return dd2nd(o, ddbind<R>(f, *m_data.mapn));
  }
  not_reached();
}

//////////////////////////////////////////////////////////////////////

bool Type::hasData() const {
  return m_dataTag != DataTag::None;
}

template<bool contextSensitive>
bool Type::equivData(const Type& o) const {
  if (m_dataTag != o.m_dataTag) {
    return dualDispatchDataFn(o, DualDispatchEq<contextSensitive>{});
  }

  auto contextSensitiveCheck = [&](auto a, auto b) {
    if (contextSensitive && a.isCtx != b.isCtx) return false;
    return true;
  };

  switch (m_dataTag) {
  case DataTag::None:
    not_reached();
  case DataTag::Str:
    return m_data.sval == o.m_data.sval;
  case DataTag::ArrLikeVal:
    return m_data.aval == o.m_data.aval;
  case DataTag::Int:
    return m_data.ival == o.m_data.ival;
  case DataTag::Dbl:
    // +ve and -ve zero must not compare equal, but (for purposes of
    // Type equivalence), NaNs are equal.
    return m_data.dval == o.m_data.dval ?
      std::signbit(m_data.dval) == std::signbit(o.m_data.dval) :
      (std::isnan(m_data.dval) && std::isnan(o.m_data.dval));
  case DataTag::Obj:
    if (!m_data.dobj.whType != !o.m_data.dobj.whType) return false;
    if (m_data.dobj.whType &&
        !m_data.dobj.whType
          ->equivImpl<contextSensitive>(*o.m_data.dobj.whType)) {
      return false;
    }
    return m_data.dobj.type == o.m_data.dobj.type &&
           m_data.dobj.cls.same(o.m_data.dobj.cls) &&
           contextSensitiveCheck(m_data.dobj, o.m_data.dobj);
  case DataTag::Cls:
    return m_data.dcls.type == o.m_data.dcls.type &&
           m_data.dcls.cls.same(o.m_data.dcls.cls) &&
           contextSensitiveCheck(m_data.dcls, o.m_data.dcls);
  case DataTag::Record:
    return m_data.drec.type == o.m_data.drec.type &&
           m_data.drec.rec.same(o.m_data.drec.rec);
  case DataTag::ArrLikePacked:
    if (m_data.packed->elems.size() != o.m_data.packed->elems.size()) {
      return false;
    }
    if (m_data.packed->provenance != o.m_data.packed->provenance) {
      return false;
    }
    for (auto i = 0; i <  m_data.packed->elems.size(); i++) {
      if (!m_data.packed->elems[i]
             .equivImpl<contextSensitive>(o.m_data.packed->elems[i])) {
        return false;
      }
    }
    return true;
  case DataTag::ArrLikePackedN:
    return m_data.packedn->type
             .equivImpl<contextSensitive>(o.m_data.packedn->type);
  case DataTag::ArrLikeMap: {
    if (m_data.map->map.size() != o.m_data.map->map.size()) {
      return false;
    }
    if (m_data.map->provenance != o.m_data.map->provenance) {
      return false;
    }
    auto it = o.m_data.map->map.begin();
    for (auto const& kv : m_data.map->map) {
      if (!ArrayLikeMapEqual{}(kv.first, it->first)) return false;
      if (!kv.second.equivImpl<contextSensitive>(it->second)) return false;
      ++it;
    }
    return
      m_data.map->optKey.equivImpl<contextSensitive>(o.m_data.map->optKey) &&
      m_data.map->optVal.equivImpl<contextSensitive>(o.m_data.map->optVal);
  }
  case DataTag::ArrLikeMapN:
    return m_data.mapn->key.equivImpl<contextSensitive>(o.m_data.mapn->key) &&
           m_data.mapn->val.equivImpl<contextSensitive>(o.m_data.mapn->val);
  }
  not_reached();
}

template<bool contextSensitive>
bool Type::subtypeData(const Type& o) const {
  if (m_dataTag != o.m_dataTag) {
    return dualDispatchDataFn(o, DualDispatchSubtype<contextSensitive>{});
  }

  auto contextSensitiveCheck = [&](auto self, auto other) {
    if (contextSensitive && !self.isCtx && other.isCtx) return false;
    return true;
  };

  switch (m_dataTag) {
  case DataTag::Obj:
  {
    auto const outer_ok = [&] {
      if (!contextSensitiveCheck(m_data.dobj, o.m_data.dobj)) return false;
      if (m_data.dobj.type == o.m_data.dobj.type &&
          m_data.dobj.cls.same(o.m_data.dobj.cls)) {
        return true;
      }
      if (o.m_data.dobj.type == DObj::Sub) {
        return m_data.dobj.cls.mustBeSubtypeOf(o.m_data.dobj.cls);
      }
      return false;
    }();
    if (!outer_ok) return false;
    if (!o.m_data.dobj.whType) return true;
    if (!m_data.dobj.whType) return false;
    return m_data.dobj.whType
             ->subtypeOfImpl<contextSensitive>(*o.m_data.dobj.whType);
  }
  case DataTag::Cls:
    if (!contextSensitiveCheck(m_data.dcls, o.m_data.dcls)) return false;
    if (m_data.dcls.type == o.m_data.dcls.type &&
        m_data.dcls.cls.same(o.m_data.dcls.cls)) {
      return true;
    }
    if (o.m_data.dcls.type == DCls::Sub) {
      return m_data.dcls.cls.mustBeSubtypeOf(o.m_data.dcls.cls);
    }
    return false;
  case DataTag::Record:
    if (m_data.drec.type == o.m_data.drec.type &&
        m_data.drec.rec.same(o.m_data.drec.rec)) {
      return true;
    }
    if (o.m_data.drec.type == DRecord::Sub) {
      return m_data.drec.rec.mustBeSubtypeOf(o.m_data.drec.rec);
    }
    return false;
  case DataTag::ArrLikeVal:
    return subtypeArrLike(m_data.aval, o.m_data.aval);
  case DataTag::Str:
  case DataTag::Int:
  case DataTag::Dbl:
  case DataTag::None:
    // Context sensitivity should not matter here.
    return equivData<contextSensitive>(o);
  case DataTag::ArrLikePacked:
    return subtypePacked<contextSensitive>(*m_data.packed, *o.m_data.packed);
  case DataTag::ArrLikePackedN:
    return m_data.packedn->type
            .subtypeOfImpl<contextSensitive>(o.m_data.packedn->type);
  case DataTag::ArrLikeMap:
    return subtypeMap<contextSensitive>(*m_data.map, *o.m_data.map);
  case DataTag::ArrLikeMapN:
    return m_data.mapn->key
             .subtypeOfImpl<contextSensitive>(o.m_data.mapn->key) &&
           m_data.mapn->val.subtypeOfImpl<contextSensitive>(o.m_data.mapn->val);
  }
  not_reached();
}

bool Type::couldBeData(const Type& o) const {
  if (m_dataTag != o.m_dataTag) {
    return dualDispatchDataFn(o, DualDispatchCouldBe{});
  }

  switch (m_dataTag) {
  case DataTag::None:
    not_reached();
  case DataTag::Obj:
  {
    auto couldBe = [&] {
      if (m_data.dobj.type == o.m_data.dobj.type &&
          m_data.dobj.cls.same(o.m_data.dobj.cls)) {
        return true;
      }
      if (m_data.dobj.type == DObj::Sub) {
        if (o.m_data.dobj.type == DObj::Sub) {
          return o.m_data.dobj.cls.couldBe(m_data.dobj.cls);
        }
        return o.m_data.dobj.cls.maybeSubtypeOf(m_data.dobj.cls);
      }
      if (o.m_data.dobj.type == DObj::Sub) {
        return m_data.dobj.cls.maybeSubtypeOf(o.m_data.dobj.cls);
      }
      return false;
    }();
    return couldBe && (!o.m_data.dobj.whType ||
                       !m_data.dobj.whType ||
                       m_data.dobj.whType->couldBe(*o.m_data.dobj.whType));
  }
  case DataTag::Cls:
    if (m_data.dcls.type == o.m_data.dcls.type &&
        m_data.dcls.cls.same(o.m_data.dcls.cls)) {
      return true;
    }
    if (m_data.dcls.type == DCls::Sub || o.m_data.dcls.type == DCls::Sub) {
      return m_data.dcls.cls.couldBe(o.m_data.dcls.cls);
    }
    return false;
  case DataTag::Record:
    if (m_data.drec.type == o.m_data.drec.type &&
        m_data.drec.rec.same(o.m_data.drec.rec)) {
      return true;
    }
    if (m_data.drec.type == DRecord::Sub ||
        o.m_data.drec.type == DRecord::Sub) {
      return m_data.drec.rec.couldBe(o.m_data.drec.rec);
    }
    return false;
  case DataTag::ArrLikeVal:
    return couldBeArrLike(m_data.aval, o.m_data.aval);
  case DataTag::Str:
  case DataTag::Int:
  case DataTag::Dbl:
    return equivData<false>(o);
  case DataTag::ArrLikePacked:
    return couldBePacked(*m_data.packed, *o.m_data.packed);
  case DataTag::ArrLikePackedN:
    return m_data.packedn->type.couldBe(o.m_data.packedn->type);
  case DataTag::ArrLikeMap:
    return couldBeMap(*m_data.map, *o.m_data.map);
  case DataTag::ArrLikeMapN:
    return m_data.mapn->key.couldBe(o.m_data.mapn->key) &&
           m_data.mapn->val.couldBe(o.m_data.mapn->val);
  }
  not_reached();
}

template<bool contextSensitive>
bool Type::equivImpl(const Type& o) const {
  // NB: We don't assert checkInvariants() here because this can be called from
  // checkInvariants() and it all takes too long if the type is deeply nested.

  if (m_bits != o.m_bits) return false;
  if (hasData() != o.hasData()) return false;
  //if (m_bits & BRecord) return false;
  if (!hasData()) return true;

  return equivData<contextSensitive>(o);
}

bool Type::equivalentlyRefined(const Type& o) const {
  return equivImpl<true>(o);
}

bool Type::operator==(const Type& o) const {
  return equivImpl<false>(o);
}

size_t Type::hash() const {
  using U1 = std::underlying_type<decltype(m_bits)>::type;
  using U2 = std::underlying_type<decltype(m_dataTag)>::type;
  auto const rawBits = U1{m_bits};
  auto const rawTag  = static_cast<U2>(m_dataTag);

  auto const data =
    [&] () -> uintptr_t {
      switch (m_dataTag) {
        case DataTag::None:
          return 0;
        case DataTag::Obj:
          return (uintptr_t)m_data.dobj.cls.name();
        case DataTag::Cls:
          return (uintptr_t)m_data.dcls.cls.name();
        case DataTag::Record:
          return (uintptr_t)m_data.drec.rec.name();
        case DataTag::Str:
          return (uintptr_t)m_data.sval;
        case DataTag::Int:
          return m_data.ival;
        case DataTag::Dbl:
          return std::hash<double>{}(m_data.dval);
        case DataTag::ArrLikeVal:
          return (uintptr_t)m_data.aval;
        case DataTag::ArrLikePacked:
          return m_data.packed->elems.size();
        case DataTag::ArrLikePackedN:
          return 0;
        case DataTag::ArrLikeMap:
          return m_data.map->map.size();
        case DataTag::ArrLikeMapN:
          return 0;
      }
      not_reached();
    }();

  return folly::hash::hash_combine(rawBits, rawTag, data);
}

bool must_be_counted(const Type& t) {
  return must_be_counted(t, t.m_bits);
}

bool must_be_counted(const Type& t, trep bits) {
  if ((bits & BUnc) == BBottom) return true;
  if (bits & BInitNull) return false;

  switch (t.m_dataTag) {
  case DataTag::None:
  case DataTag::Int:
  case DataTag::Dbl:
  case DataTag::Cls:
  case DataTag::Str:
  case DataTag::ArrLikeVal:
    return false;
  case DataTag::Obj:
    return true;
  case DataTag::ArrLikePackedN:
    if (bits & BSArrLikeE) return false;
    return must_be_counted(t.m_data.packedn->type);
  case DataTag::ArrLikePacked: {
    if (bits & BSArrLikeE) return false;
    auto const packed = t.m_data.packed.get();
    for (size_t i = 0; i < packed->elems.size(); ++i) {
      if (must_be_counted(packed->elems[i])) return true;
    }
    return false;
  }
  case DataTag::ArrLikeMapN:
    if (bits & BSArrLikeE) return false;
    return
      must_be_counted(t.m_data.mapn->key) ||
      must_be_counted(t.m_data.mapn->val);
  case DataTag::ArrLikeMap:
    if (bits & BSArrLikeE) return false;
    for (auto const& p : t.m_data.map->map) {
      if (must_be_counted(p.second)) return true;
    }
    return false;
  default:
    not_reached();
  }
}

Type project_data(Type t, trep bits) {
  auto const restrict_to = [&](trep allowed) {
    assert(t.m_bits & allowed);
    return bits & allowed ? t : loosen_values(t);
  };

  switch (t.m_dataTag) {
  case DataTag::None:
    return t;
  case DataTag::Int:         return restrict_to(BInt);
  case DataTag::Dbl:         return restrict_to(BDbl);
  case DataTag::Obj:         return restrict_to(BObj);
  case DataTag::Record:      return restrict_to(BRecord);
  case DataTag::Cls:         return restrict_to(BCls);
  case DataTag::Str:         return restrict_to(BStr);
  case DataTag::ArrLikePacked:
    return restrict_to(BVecN | BDictN | BKeysetN | BArrN);
  case DataTag::ArrLikeMap:
    return restrict_to(BDictN | BKeysetN | BArrN);
  case DataTag::ArrLikeVal: {
    auto const ad = t.m_data.aval;
    if (ad->empty()) {
      if (bits & (BArrN | BVecN | BDictN | BKeysetN)) {
        return loosen_values(t);
      }
      return restrict_to(BArrE | BDictE | BVecE | BKeysetE);
    }
    return restrict_to(BArrN | BDictN | BVecN | BKeysetN);
  }
  case DataTag::ArrLikePackedN:
    return restrict_to(BVecN | BDictN | BKeysetN | BArrN);
  case DataTag::ArrLikeMapN:
    return restrict_to(BDictN | BKeysetN | BArrN);
  default:
    not_reached();
  }
}

Type remove_counted(Type t) {
  auto const isStatic = [] (const Type& t) {
    return (t.m_bits & BUnc) == t.m_bits;
  };
  auto const isCounted = [] (const Type& t) {
    return (t.m_bits & BUnc) == BBottom;
  };
  auto const strip = [&] {
    t.m_bits &= BUnc;
    assertx(isPredefined(t.m_bits));
    return t;
  };
  auto const nothing = [&] {
    auto ret = t.m_bits & BSArrLikeE;
    if (is_opt(t)) ret |= BInitNull;
    return Type { ret };
  };

  switch (t.m_dataTag) {
    case DataTag::None:
    case DataTag::Int:
    case DataTag::Dbl:
    case DataTag::Cls:
    case DataTag::Record:
    case DataTag::Str:
    case DataTag::ArrLikeVal:
      return strip();
    case DataTag::Obj:
      return nothing();
    case DataTag::ArrLikePackedN: {
      if (isStatic(t.m_data.packedn->type)) return strip();
      if (isCounted(t.m_data.packedn->type)) return nothing();
      auto mutated = t.m_data.packedn.mutate();
      auto ty = remove_counted(std::move(mutated->type));
      if (ty == TBottom) return nothing();
      mutated->type = std::move(ty);
      return strip();
    }
    case DataTag::ArrLikeMapN: {
      auto const keyStatic = isStatic(t.m_data.mapn->key);
      auto const valStatic = isStatic(t.m_data.mapn->val);
      if (keyStatic && valStatic) return strip();
      if (isCounted(t.m_data.mapn->key)) return nothing();
      if (isCounted(t.m_data.mapn->val)) return nothing();

      DArrLikeMapN* mutated = nullptr;
      if (!keyStatic) {
        mutated = t.m_data.mapn.mutate();
        auto ty = remove_counted(std::move(mutated->key));
        if (ty == TBottom) return nothing();
        mutated->key = std::move(ty);
      }
      if (!valStatic) {
        if (!mutated) mutated = t.m_data.mapn.mutate();
        auto ty = remove_counted(std::move(mutated->val));
        if (ty == TBottom) return nothing();
        mutated->val = std::move(ty);
      }
      return strip();
    }
    case DataTag::ArrLikePacked: {
      auto packed = const_cast<DArrLikePacked*>(t.m_data.packed.get());
      auto changed = false;
      for (size_t i = 0; i < packed->elems.size(); ++i) {
        if (isStatic(packed->elems[i])) continue;
        if (isCounted(packed->elems[i])) return nothing();
        if (!changed) {
          packed = t.m_data.packed.mutate();
          changed = true;
        }
        auto ty = remove_counted(std::move(packed->elems[i]));
        if (ty == TBottom) return nothing();
        packed->elems[i] = std::move(ty);
      }
      return strip();
    }
    case DataTag::ArrLikeMap: {
      auto const map = t.m_data.map.get();
      size_t count = 0;
      for (auto it = map->map.begin(); it != map->map.end(); ++it, ++count) {
        if (isStatic(it->second)) continue;
        if (isCounted(it->second)) return nothing();
        break;
      }
      auto const optKeyStatic = isStatic(t.m_data.map->optKey);
      auto optValStatic = isStatic(t.m_data.map->optVal);
      DArrLikeMap* mutated = nullptr;
      if (count < map->map.size()) {
        mutated = t.m_data.map.mutate();
        auto it = mutated->map.begin();
        for (std::advance(it, count); it != mutated->map.end(); ++it) {
          if (isStatic(it->second)) continue;
          if (isCounted(it->second)) return nothing();
          auto ty = remove_counted(it->second);
          if (ty == TBottom) return nothing();
          mutated->map.update(it, std::move(ty));
        }
      }

      if (!optKeyStatic) {
        if (!mutated) mutated = t.m_data.map.mutate();
        if (isCounted(mutated->optKey)) {
          mutated->optKey = TBottom;
          mutated->optVal = TBottom;
          optValStatic = true;
        } else {
          auto ty = remove_counted(std::move(mutated->optKey));
          if (ty == TBottom) {
            mutated->optKey = TBottom;
            mutated->optVal = TBottom;
            optValStatic = true;
          } else {
            mutated->optKey = std::move(ty);
          }
        }
      }

      if (!optValStatic) {
        if (!mutated) mutated = t.m_data.map.mutate();
        if (isCounted(mutated->optVal)) {
          mutated->optKey = TBottom;
          mutated->optVal = TBottom;
        } else {
          auto ty = remove_counted(std::move(mutated->optVal));
          if (ty == TBottom) {
            mutated->optKey = TBottom;
            mutated->optVal = TBottom;
          } else {
            mutated->optVal = std::move(ty);
          }
        }
      }

      return strip();
    }
  }
  not_reached();
}

template<bool contextSensitive>
bool Type::subtypeOfImpl(const Type& o) const {
  // NB: We don't assert checkInvariants() here because this can be called from
  // checkInvariants() and it all takes too long if the type is deeply nested.

  auto const isect = m_bits & o.m_bits;
  if (isect != m_bits) return false;

  // No data is always more general.
  auto const this_projected = project_data(*this, isect);
  auto const o_projected = project_data(o, isect);
  if (!o_projected.hasData())      return true;
  if (!this_projected.hasData()) return !mayHaveData(m_bits);

  // Both have data, so it depends on what the data says.
  return this_projected.subtypeData<contextSensitive>(o_projected);
}

bool Type::moreRefined(const Type& o) const {
  return subtypeOfImpl<true>(o);
}

bool Type::strictlyMoreRefined(const Type& o) const {
  return subtypeOfImpl<true>(o) &&
         !equivImpl<true>(o);
}

bool Type::subtypeOf(const Type& o) const {
  return subtypeOfImpl<false>(o);
}

bool Type::strictSubtypeOf(const Type& o) const {
  assert(checkInvariants());
  assert(o.checkInvariants());
  return *this != o && subtypeOf(o);
}

bool Type::couldBe(const Type& o) const {
  assert(checkInvariants());
  assert(o.checkInvariants());

  auto const isect = m_bits & o.m_bits;
  if (isect == 0) return false;

  // If the intersection contains any of these, it's a valid
  // intersection regardless of any data since they do not use
  // data. This is not just an optimization: if data exists and
  // doesn't match, we don't want to (incorrectly) report no
  // intersection.
  if (isect & (BNull | BBool)) return true;

  // This looks like it could be problematic - eg BCell does not
  // support data, but lots of its subtypes do. It seems like what we
  // need here is !subtypeMayHaveData(isect) (a function we don't
  // actually have). We only care in the case that both inputs have
  // data, however, so all we rely on here is that if A supports data,
  // and B is a subtype of A that does not, then no subtype of B can
  // support data.
  if (!mayHaveData(isect)) return true;

  auto const this_projected = project_data(*this, isect);
  auto const o_projected = project_data(o, isect);

  // If the intersection is static and only one type has data, we need
  // to check if that data does not only contains counted values. If
  // so, there's no actual intersection (because you cannot get a
  // counted value from a static type). This isn't an issue if both
  // have data because couldBeData() will check.
  if (!this_projected.hasData()) {
    return
      ((isect & BUnc) != isect) ||
      !o_projected.hasData() ||
      !must_be_counted(o_projected, isect);
  }
  if (!o_projected.hasData()) {
    return
      ((isect & BUnc) != isect) ||
      !must_be_counted(this_projected, isect);
  }

  // BArrLikeE is similar to the null and bool case above, except we
  // need to check for the special case where both types have data
  // corresponding to an empty array. Only in that case does the data
  // correspond to the BArrLikeE bits and we need to check it (for
  // provenance mismatch purposes).
  if (isect & BArrLikeE) {
    if (m_dataTag != o.m_dataTag) return true;
    if (m_dataTag != DataTag::ArrLikeVal) return true;
    if (m_data.aval->size() != 0 || o.m_data.aval->size() != 0) return true;
    return couldBeArrLike(m_data.aval, o.m_data.aval);
  }
  return this_projected.couldBeData(o_projected);
}

bool Type::checkInvariants() const {
  assert(isPredefined(m_bits));
  assert(!hasData() || mayHaveData(m_bits));

#define check(a) \
  if (m_bits & BC##a) assertx(m_bits & BS##a)
  check(Str);
  check(PArrE);
  check(PArrN);
  check(VArrE);
  check(VArrN);
  check(DArrE);
  check(DArrN);
  check(ArrE);
  check(ArrN);
  check(VecE);
  check(VecN);
  check(DictE);
  check(DictN);
  check(KeysetE);
  check(KeysetN);
#undef check

  // NB: Avoid copying non-trivial types in here to avoid recursive calls to
  // checkInvariants() which can cause exponential time blow-ups.

  DEBUG_ONLY auto const isVArray = subtypeOrNull(BVArr);
  DEBUG_ONLY auto const isDArray = subtypeOrNull(BDArr);
  DEBUG_ONLY auto const isNotDVArray = subtypeOrNull(BPArr);
  DEBUG_ONLY auto const isPHPArray = subtypeOrNull(BArr);
  DEBUG_ONLY auto const isVector = subtypeOrNull(BVec);
  DEBUG_ONLY auto const isKeyset = subtypeOrNull(BKeyset);
  DEBUG_ONLY auto const isDict = subtypeOrNull(BDict);

  DEBUG_ONLY auto const keyBits =
    subtypeOrNull(BSArrLike) ? BUncArrKey : BArrKey;
  DEBUG_ONLY auto const valBits = isKeyset ? BArrKey : BInitCell;

  /*
   * TODO(#3696042): for static arrays, we could enforce that all
   * inner-types are also static (this may would require changes to
   * things like union_of to ensure that we never promote an inner
   * type to a counted type).
   */

  switch (m_dataTag) {
  case DataTag::None:   break;
  case DataTag::Str:    assert(m_data.sval->isStatic()); break;
  case DataTag::Dbl:    break;
  case DataTag::Int:    break;
  case DataTag::Cls:    break;
  case DataTag::Obj:    break;
  case DataTag::Record: break;
  case DataTag::ArrLikeVal:
    assert(m_data.aval->isStatic());
    assert(!m_data.aval->empty() ||
           isVector || isDict || isVArray || isDArray);
    assert(m_bits & (BArr | BVec | BDict | BKeyset));
    if (m_data.aval->empty()) {
      assert(!couldBe(BVecN));
      assert(!couldBe(BDictN));
    }
    // If we have a static array, we'd better be sure of the type.
    assert(!isPHPArray || isVArray || isDArray || isNotDVArray);
    assert(!isPHPArray || m_data.aval->isPHPArrayType());
    assert(!isVArray || m_data.aval->isVArray());
    assert(!isDArray || m_data.aval->isDArray());
    assert(!isNotDVArray || m_data.aval->isNotDVArray());
    assert(!isVector || m_data.aval->isVecArrayType());
    assert(!isKeyset || m_data.aval->isKeysetType());
    assert(!isDict || m_data.aval->isDictType());
    assertx(!RuntimeOption::EvalHackArrDVArrs || m_data.aval->isNotDVArray());
    assertx(!m_data.aval->hasProvenanceData() || RO::EvalArrayProvenance);
    assertx(!m_data.aval->hasProvenanceData() || m_bits & kProvBits);
    assertx(!m_data.aval->hasProvenanceData() ||
            arrprov::getTag(m_data.aval).valid());
    break;
  case DataTag::ArrLikePacked: {
    assert(!m_data.packed->elems.empty());
    assertx(m_data.packed->provenance == ProvTag::Top ||
            RO::EvalArrayProvenance);
    assertx(m_data.packed->provenance == ProvTag::Top ||
            m_bits & kProvBits);
    assert(m_bits & (BVecN | BDictN | BKeysetN | BArrN));
    DEBUG_ONLY auto idx = size_t{0};
    for (DEBUG_ONLY auto const& v : m_data.packed->elems) {
      assert(v.subtypeOf(valBits) && v != TBottom);
      assert(!isKeyset || v == ival(idx++));
    }
    break;
  }
  case DataTag::ArrLikeMap: {
    assert(!isVector);
    assert(!isVArray);
    assert(m_bits & (BDictN | BKeysetN | BArrN));
    assert(!m_data.map->map.empty());
    assertx(m_data.map->provenance == ProvTag::Top ||
            RO::EvalArrayProvenance);
    assertx(m_data.map->provenance == ProvTag::Top ||
            m_bits & kProvBits);
    DEBUG_ONLY auto idx = size_t{0};
    DEBUG_ONLY auto packed = true;
    for (DEBUG_ONLY auto const& kv : m_data.map->map) {
      assert(tvIsPlausible(kv.first));
      assert(isIntType(kv.first.m_type) ||
             kv.first.m_type == KindOfPersistentString);
      assert(kv.second.subtypeOf(valBits) && kv.second != TBottom);
      assert(!isKeyset ||
             loosen_staticness(from_cell(kv.first)) ==
             loosen_staticness(kv.second));
      if (packed) {
        packed = isIntType(kv.first.m_type) && kv.first.m_data.num == idx;
        ++idx;
      }
    }
    // Map shouldn't have packed-like keys. If it does, it should be Packed
    // instead.
    assertx(!packed);
    // Optional elements are either both Bottom or both not
    assertx((m_data.map->optKey == TBottom) == (m_data.map->optVal == TBottom));
    assertx(m_data.map->optKey.subtypeOf(keyBits));
    assertx(m_data.map->optVal.subtypeOf(valBits));
    assertx(!isKeyset || m_data.map->optKey == m_data.map->optVal);
    // If the optional element has a const key, it cannot be the same
    // key in the known keys.
    if (auto const k = tv(m_data.map->optKey)) {
      assertx(m_data.map->map.find(*k) == m_data.map->map.end());
    }
    break;
  }
  case DataTag::ArrLikePackedN:
    assert(m_data.packedn->type.subtypeOf(valBits));
    assert(m_data.packedn->type != TBottom);
    assert(m_bits & (BVecN | BDictN | BKeysetN | BArrN));
    assert(!isKeyset || m_data.packedn->type == TInt);
    break;
  case DataTag::ArrLikeMapN:
    assert(!isVector);
    assert(!isVArray);
    assert(m_bits & (BDictN | BKeysetN | BArrN));
    assert(m_data.mapn->key.subtypeOf(keyBits));
    // MapN shouldn't have a specialized key. If it does, then that implies it
    // only contains arrays of size 1, which means it should be Map instead.
    assert(m_data.mapn->key.m_dataTag == DataTag::None);
    assert(m_data.mapn->val.subtypeOf(valBits));
    assert(m_data.mapn->key != TBottom);
    assert(m_data.mapn->val != TBottom);
    assert(!isKeyset || m_data.mapn->key == m_data.mapn->val);
    break;
  }
  return true;
}

/* Retrieve the provenance tag from a type, if it has one */
ProvTag Type::getProvTag() const {
  if (!RuntimeOption::EvalArrayProvenance) return ProvTag::Top;
  switch (m_dataTag) {
  case DataTag::None:
  case DataTag::Str:
  case DataTag::Int:
  case DataTag::Dbl:
  case DataTag::Obj:
  case DataTag::Cls:
  case DataTag::Record:
  case DataTag::ArrLikePackedN:
  case DataTag::ArrLikeMapN:
    return ProvTag::Top;
  case DataTag::ArrLikeVal:
    return ProvTag::FromSArr(m_data.aval);
  case DataTag::ArrLikePacked:
    return m_data.packed->provenance;
  case DataTag::ArrLikeMap:
    return m_data.map->provenance;
  }
  always_assert(false);
}

//////////////////////////////////////////////////////////////////////

Type wait_handle(const Index& index, Type inner) {
  auto const rwh = index.builtin_class(s_Awaitable.get());
  auto t = subObj(rwh);
  t.m_data.dobj.whType.emplace(std::move(inner));
  return t;
}

bool is_specialized_wait_handle(const Type& t) {
  return
    t.m_dataTag == DataTag::Obj &&
    t.m_data.dobj.whType;
}

Type wait_handle_inner(const Type& t) {
  assert(is_specialized_wait_handle(t));
  return *t.m_data.dobj.whType;
}

Type sval(SString val) {
  assert(val->isStatic());
  auto r        = Type { BSStr };
  r.m_data.sval = val;
  r.m_dataTag   = DataTag::Str;
  return r;
}

Type sval_nonstatic(SString val) {
  assert(val->isStatic());
  auto r        = Type { BStr };
  r.m_data.sval = val;
  r.m_dataTag   = DataTag::Str;
  return r;
}

Type ival(int64_t val) {
  auto r        = Type { BInt };
  r.m_data.ival = val;
  r.m_dataTag   = DataTag::Int;
  return r;
}

Type dval(double val) {
  auto r        = Type { BDbl };
  r.m_data.dval = val;
  r.m_dataTag   = DataTag::Dbl;
  return r;
}

Type aval(SArray val) {
  assert(val->isStatic());
  assert(val->isPHPArrayType());
  assertx(!RuntimeOption::EvalHackArrDVArrs || val->isNotDVArray());

  if (val->empty() && val->isNotDVArray()) return aempty();

  auto r = [&]{
    if (val->empty()) {
      if (val->isDArray()) return Type { BSDArrE };
      if (val->isVArray()) return Type { BSVArrE };
      always_assert(false); // handled above
    }
    if (val->isDArray()) return Type { BSDArrN };
    if (val->isVArray()) return Type { BSVArrN };
    return Type { BSPArrN };
  }();

  r.m_data.aval = val;
  r.m_dataTag = DataTag::ArrLikeVal;
  return r;
}

Type aempty()         { return Type { BSPArrE }; }

Type aempty_varray(ProvTag tag)  {
  assertx(!RuntimeOption::EvalHackArrDVArrs);
  auto r = Type { BSVArrE };
  r.m_data.aval = tag.valid()
    ? arrprov::tagStaticArr(staticEmptyVArray(), tag.get())
    : staticEmptyVArray();
  r.m_dataTag = DataTag::ArrLikeVal;
  return r;
}

Type aempty_darray(ProvTag tag)  {
  assertx(!RuntimeOption::EvalHackArrDVArrs);
  auto r = Type { BSDArrE };
  r.m_data.aval = tag.valid()
    ? arrprov::tagStaticArr(staticEmptyDArray(), tag.get())
    : staticEmptyDArray();
  r.m_dataTag = DataTag::ArrLikeVal;
  return r;
}

Type sempty()         { return sval(staticEmptyString()); }
Type some_aempty()    { return Type { BPArrE }; }

Type some_aempty_darray(ProvTag tag) {
  assertx(!RuntimeOption::EvalHackArrDVArrs);
  auto r = Type { BDArrE };
  r.m_data.aval = tag.valid()
    ? arrprov::tagStaticArr(staticEmptyDArray(), tag.get())
    : staticEmptyDArray();
  r.m_dataTag = DataTag::ArrLikeVal;
  return r;
}

Type vec_val(SArray val) {
  assert(val->isStatic());
  assert(val->isVecArrayType());
  auto const bits = val->empty() ? BSVecE : BSVecN;
  auto r = Type { bits };
  r.m_data.aval = val;
  r.m_dataTag = DataTag::ArrLikeVal;
  return r;
}

Type vec_empty(ProvTag tag) {
  auto r = Type { BSVecE };
  r.m_data.aval = tag.valid()
    ? arrprov::tagStaticArr(staticEmptyVecArray(), tag.get())
    : staticEmptyVecArray();
  r.m_dataTag = DataTag::ArrLikeVal;
  return r;
}

Type some_vec_empty(ProvTag tag) {
  auto r = Type { BVecE };
  r.m_data.aval = tag.valid()
    ? arrprov::tagStaticArr(staticEmptyVecArray(), tag.get())
    : staticEmptyVecArray();
  r.m_dataTag = DataTag::ArrLikeVal;
  return r;
}

Type packedn_impl(trep bits, Type t) {
  auto r = Type { bits };
  construct_inner(r.m_data.packedn, std::move(t));
  r.m_dataTag = DataTag::ArrLikePackedN;
  return r;
}

Type packed_impl(trep bits, std::vector<Type> elems, ProvTag prov) {
  assert(!elems.empty());
  auto r = Type { bits };
  auto const tag = (bits & kProvBits) ? prov : ProvTag::Top;
  construct_inner(r.m_data.packed, std::move(elems), tag);
  r.m_dataTag = DataTag::ArrLikePacked;
  return r;
}

Type vec_n(Type ty) {
  return packedn_impl(BVecN, std::move(ty));
}

Type svec_n(Type ty) {
  return packedn_impl(BSVecN, std::move(ty));
}

Type vec(std::vector<Type> elems, ProvTag tag) {
  return packed_impl(BVecN, std::move(elems), tag);
}

Type svec(std::vector<Type> elems, ProvTag tag) {
  return packed_impl(BSVecN, std::move(elems), tag);
}

Type dict_val(SArray val) {
  assert(val->isStatic());
  assert(val->isDictType());
  auto const bits = val->empty() ? BSDictE : BSDictN;
  auto r = Type { bits };
  r.m_data.aval = val;
  r.m_dataTag   = DataTag::ArrLikeVal;
  return r;
}

Type dict_empty(ProvTag tag) {
  auto r = Type { BSDictE };
  r.m_data.aval = tag.valid()
    ? arrprov::tagStaticArr(staticEmptyDictArray(), tag.get())
    : staticEmptyDictArray();
  r.m_dataTag   = DataTag::ArrLikeVal;
  return r;
}

Type some_dict_empty(ProvTag tag) {
  auto r = Type { BDictE };
  r.m_data.aval = tag.valid()
    ? arrprov::tagStaticArr(staticEmptyDictArray(), tag.get())
    : staticEmptyDictArray();
  r.m_dataTag   = DataTag::ArrLikeVal;
  return r;
}

Type dict_map(MapElems m, ProvTag tag, Type optKey, Type optVal) {
  return map_impl(
    BDictN,
    std::move(m),
    std::move(optKey),
    std::move(optVal),
    tag
  );
}

Type dict_n(Type k, Type v, ProvTag tag) {
  return mapn_impl(BDictN, std::move(k), std::move(v), tag);
}

Type sdict_n(Type k, Type v, ProvTag tag) {
  return mapn_impl(BSDictN, std::move(k), std::move(v), tag);
}

Type keyset_val(SArray val) {
  assert(val->isStatic());
  assert(val->isKeysetType());
  if (val->empty()) return keyset_empty();
  auto r        = Type { BSKeysetN };
  r.m_data.aval = val;
  r.m_dataTag   = DataTag::ArrLikeVal;
  return r;
}

Type keyset_empty()         { return Type { BSKeysetE }; }
Type some_keyset_empty()    { return Type { BKeysetE }; }

Type keyset_n(Type kv) {
  assert(kv.subtypeOf(BArrKey));
  auto v = kv;
  return mapn_impl(BKeysetN, std::move(kv), std::move(v), ProvTag::Top);
}

Type skeyset_n(Type kv) {
  assert(kv.subtypeOf(BUncArrKey));
  auto v = kv;
  return mapn_impl(BSKeysetN, std::move(kv), std::move(v), ProvTag::Top);
}

Type exactRecord(res::Record val) {
  auto r = Type { BRecord };
  construct(r.m_data.drec, DRecord::Exact, val);
  r.m_dataTag = DataTag::Record;
  return r;
}

Type subRecord(res::Record val) {
  auto r = Type { BRecord };
  construct(r.m_data.drec,
            val.couldBeOverriden() ? DRecord::Sub : DRecord::Exact,
            val);
  r.m_dataTag = DataTag::Record;
  return r;
}

Type subObj(res::Class val) {
  auto r = Type { BObj };
  construct(r.m_data.dobj,
            val.couldBeOverriden() ? DObj::Sub : DObj::Exact,
            val);
  r.m_dataTag = DataTag::Obj;
  return r;
}

Type objExact(res::Class val) {
  auto r = Type { BObj };
  construct(r.m_data.dobj, DObj::Exact, val);
  r.m_dataTag = DataTag::Obj;
  return r;
}

Type subCls(res::Class val) {
  auto r        = Type { BCls };
  construct(r.m_data.dcls,
            val.couldBeOverriden() ? DCls::Sub : DCls::Exact,
            val);
  r.m_dataTag = DataTag::Cls;
  return r;
}

Type clsExact(res::Class val) {
  auto r        = Type { BCls };
  construct(r.m_data.dcls, DCls::Exact, val);
  r.m_dataTag   = DataTag::Cls;
  return r;
}

bool is_specialized_array_like(const Type& t) {
  switch (t.m_dataTag) {
  case DataTag::None:
  case DataTag::Str:
  case DataTag::Obj:
  case DataTag::Int:
  case DataTag::Dbl:
  case DataTag::Cls:
  case DataTag::Record:
    return false;
  case DataTag::ArrLikeVal:
  case DataTag::ArrLikePacked:
  case DataTag::ArrLikePackedN:
  case DataTag::ArrLikeMap:
  case DataTag::ArrLikeMapN:
    return true;
  }
  not_reached();
}

bool is_specialized_array(const Type& t) {
  return t.subtypeOrNull(BArr) && is_specialized_array_like(t);
}

bool is_specialized_vec(const Type& t) {
  return t.subtypeOrNull(BVec) && is_specialized_array_like(t);
}

bool is_specialized_dict(const Type& t) {
  return t.subtypeOrNull(BDict) && is_specialized_array_like(t);
}

bool is_specialized_keyset(const Type& t) {
  return t.subtypeOrNull(BKeyset) && is_specialized_array_like(t);
}

Type set_trep(Type& a, trep bits) {
  // If the type and its new bits don't agree on d/varray-ness and the type has
  // a static array, we need to convert the static array into its equivalent
  // Packed or Map type. We cannot have a ArrLikeVal if the type isn't
  // specifically a subtype of TOptParr, TOptVArr, or TOptDArr.
  if (a.m_dataTag == DataTag::ArrLikeVal &&
      ((a.subtypeOrNull(BPArr) && ((bits & (BPArr | BNull)) != bits)) ||
       (a.subtypeOrNull(BVArr) && ((bits & (BVArr | BNull)) != bits)) ||
       (a.subtypeOrNull(BDArr) && ((bits & (BDArr | BNull)) != bits)))) {
    if (a.m_data.aval->empty()) {
      a = loosen_values(a);
    } else {
      if (auto p = toDArrLikePacked(a.m_data.aval)) {
        return packed_impl(bits, std::move(p->elems), p->provenance);
      }
      auto d = toDArrLikeMap(a.m_data.aval);
      return map_impl(
        bits,
        std::move(d->map),
        std::move(d->optKey),
        std::move(d->optVal),
        d->provenance
      );
    }
  }
  a.m_bits = bits;
  return std::move(a);
}

/*
 * Helper function for unioning a specialized array-like type, spec_a, with a
 * same-array-kind type b.
 *
 * opt_e and opt are the corresponding non-specialized nullable-empty,
 * and nullable array-like types (ie OptArrE and OptArr, or OptVecE
 * and OptVec).
 */
Type spec_array_like_union(Type& spec_a,
                           Type& b,
                           trep opt_e,
                           trep opt) {
  // If b isn't the same kind of array-like, we'll have to treat it as
  // a union of two separate types
  if (!b.subtypeOf(opt | BNull)) return TBottom;

  auto const bits = combine_arr_like_bits(spec_a.m_bits, b.m_bits);
  assertx((bits & BNull) == ((spec_a.m_bits | b.m_bits) & BNull));
  if (!is_specialized_array_like(b)) {
    // We can keep a's specialization if b is an empty array-like
    // or a nullable empty array-like.
    if (b.subtypeOf(opt_e | BNull)) return set_trep(spec_a, bits);
    // otherwise drop the specialized bits
    return Type { bits };
  }

  auto const t = Type::unionArrLike(std::move(spec_a), std::move(b));
  assertx((t.m_bits & BNull) == (bits & BNull));
  return t;
}

Type arr_packed(std::vector<Type> elems) {
  return packed_impl(BPArrN, std::move(elems), ProvTag::Top);
}

Type arr_packed_varray(std::vector<Type> elems, ProvTag tag) {
  assertx(!RuntimeOption::EvalHackArrDVArrs);
  return packed_impl(BVArrN, std::move(elems), tag);
}

Type sarr_packed(std::vector<Type> elems) {
  return packed_impl(BSPArrN, std::move(elems), ProvTag::Top);
}

Type arr_packedn(Type t) {
  return packedn_impl(BPArrN, std::move(t));
}

Type sarr_packedn(Type t) {
  return packedn_impl(BSPArrN, std::move(t));
}

Type map_impl(trep bits, MapElems m, Type optKey, Type optVal, ProvTag prov) {
  assert(!m.empty());
  assertx((optKey == TBottom) == (optVal == TBottom));

  // A Map cannot be packed, so if it is, use a different
  // representation.
  auto idx = int64_t{0};
  auto packed = true;
  for (auto const& p : m) {
    if (!isIntType(p.first.m_type) || p.first.m_data.num != idx) {
      packed = false;
      break;
    }
    ++idx;
  }
  if (packed) {
    // The map is actually packed. If there's no optional elements, we
    // can turn it into a Packed.
    if (optKey == TBottom) {
      std::vector<Type> elems;
      for (auto& p : m) elems.emplace_back(p.second);
      return packed_impl(bits, std::move(elems), prov);
    }

    // There are optional elements. We cannot represent optionals in
    // packed representations, so we need to collapse the values into
    // a single type.
    auto vals = std::move(optVal);
    for (auto const& p : m) vals |= p.second;

    // Special case, if the optional elements represent a single key,
    // and that key is next in packed order, we can use PackedN.
    if (auto const k = tv(optKey)) {
      if (isIntType(k->m_type) && k->m_data.num == idx) {
        return packedn_impl(bits, std::move(vals));
      }
    }

    // Not known to be packed including the optional elements, so use
    // MapN, which is most general (it can contain packed and
    // non-packed types).
    return mapn_impl(bits, union_of(TInt, optKey), std::move(vals), prov);
  }

  auto r = Type { bits };
  auto const tag = (bits & kProvBits) ? prov : ProvTag::Top;
  construct_inner(
    r.m_data.map,
    std::move(m),
    std::move(optKey),
    std::move(optVal),
    tag
  );
  r.m_dataTag = DataTag::ArrLikeMap;
  return r;
}

Type arr_map(MapElems m, Type k, Type v) {
  return map_impl(
    BPArrN,
    std::move(m),
    std::move(k),
    std::move(v),
    ProvTag::Top
  );
}

Type arr_map_darray(MapElems m, ProvTag tag) {
  assertx(!RuntimeOption::EvalHackArrDVArrs);
  return map_impl(BDArrN, std::move(m), TBottom, TBottom, tag);
}

Type sarr_map(MapElems m, Type k, Type v) {
  return map_impl(
    BSPArrN,
    std::move(m),
    std::move(k),
    std::move(v),
    ProvTag::Top
  );
}

Type mapn_impl(trep bits, Type k, Type v, ProvTag tag) {
  assert(k.subtypeOf(BArrKey));

  // A MapN cannot have a constant key (because that can actually make it be a
  // subtype of Map sometimes), so if it does, make it a Map instead.
  if (auto val = tv(k)) {
    MapElems m;
    m.emplace_back(*val, std::move(v));
    return map_impl(bits, std::move(m), TBottom, TBottom, tag);
  }

  auto r = Type { bits };
  construct_inner(
    r.m_data.mapn,
    std::move(k),
    std::move(v)
  );
  r.m_dataTag = DataTag::ArrLikeMapN;
  return r;
}

Type mapn_impl_from_map(trep bits, Type k, Type v, ProvTag tag) {
  if (bits & BKeyset && k != v) {
    // When we convert could-be-keyset types from a Map representation to a
    // MapN representation, we need to coerce the staticness of the key and
    // value to match one another.  This necessity arises because DArrLikeMap
    // can decay the value type to be of unknown-staticness, while the key
    // (which is a TypedValue) maintains a persistent DataType.
    //
    // It's sufficient to loosen the key's staticness; it never happens the
    // other way around.  Also note that we don't do any other coercions here;
    // it's up to the caller to ensure `k == v` up-to-staticness.
    k = loosen_staticness(k);
  }
  return mapn_impl(bits, std::move(k), std::move(v), tag);
}

Type arr_mapn(Type k, Type v) {
  return mapn_impl(BPArrN, std::move(k), std::move(v), ProvTag::Top);
}

Type sarr_mapn(Type k, Type v) {
  return mapn_impl(BSPArrN, std::move(k), std::move(v), ProvTag::Top);
}

Type opt(Type t) {
  assert(canBeOptional(t.m_bits));
  auto ret = t;
  ret.m_bits |= BInitNull;
  return ret;
}

Type unopt(Type t) {
  assertx(is_opt(t));
  t.m_bits &= ~BInitNull;
  assertx(!is_opt(t));
  return t;
}

bool is_opt(const Type& t) {
  if (t.m_bits == BInitNull) return false;
  if (!t.couldBe(BInitNull)) return false;
  auto const nonNullBits = t.m_bits & ~BInitNull;
  return isPredefined(nonNullBits) && canBeOptional(nonNullBits);
}

bool is_nullish(const Type& t) {
  if (t.subtypeOf(TNull)) return false;
  if (!t.couldBe(TNull)) return false;
  auto const nonNullBits = t.m_bits & ~BNull;
  return isPredefined(nonNullBits) && canBeOptional(nonNullBits);
}

Type unnullish(Type t) {
  assertx(is_nullish(t));
  t.m_bits &= ~BNull;
  assertx(!is_nullish(t));
  return t;
}

Type return_with_context(Type t, Type context) {
  assertx(t.subtypeOf(BInitCell));
  // We don't assert the context is a TCls or TObj because sometimes we set it
  // to TTop when handling dynamic calls.
  if (((is_specialized_obj(t) && t.m_data.dobj.isCtx) ||
        (is_specialized_cls(t) && t.m_data.dcls.isCtx)) &&
      context.subtypeOfAny(TCls, TObj) && context != TBottom) {
    context = is_specialized_obj(t) ? toobj(context) : objcls(context);
    if (is_specialized_obj(context) && dobj_of(context).type == DObj::Exact &&
        dobj_of(context).cls.couldBeMocked()) {
      context = subObj(dobj_of(context).cls);
    }
    if (is_specialized_cls(context) && dcls_of(context).type == DCls::Exact &&
        dcls_of(context).cls.couldBeMocked()) {
      context = subCls(dcls_of(context).cls);
    }
    bool o = is_opt(t);
    t = intersection_of(unctx(std::move(t)), context);
    // We must preserve optional typing, as this is not included in the
    // context type.
    return (o && canBeOptional(t.m_bits)) ? opt(t) : t;
  }
  return unctx(t);
}

Type setctx(Type t, bool to) {
  if (is_specialized_obj(t)) {
    t.m_data.dobj.isCtx = to;
  }
  if (is_specialized_cls(t)) {
    t.m_data.dcls.isCtx = to;
  }
  return t;
}

Type unctx(Type t) {
  bool c;
  return Type::unctxHelper(t, c);
}

bool equivalently_refined(const Type& a, const Type& b) {
  return a.equivalentlyRefined(b);
}

bool is_specialized_obj(const Type& t) {
  return t.m_dataTag == DataTag::Obj;
}

bool is_specialized_record(const Type& t) {
  return t.m_dataTag == DataTag::Record;
}

bool is_specialized_cls(const Type& t) {
  return t.m_dataTag == DataTag::Cls;
}

bool is_specialized_string(const Type& t) {
  return t.m_dataTag == DataTag::Str;
}

Type toobj(const Type& t) {
  if (t.subtypeOf(BCls) && is_specialized_cls(t)) {
    auto const d = dcls_of(t);
    return setctx(d.type == DCls::Exact ? objExact(d.cls) : subObj(d.cls),
                  d.isCtx);
  } else if (t.subtypeOf(BObj)) {
    return t;
  }
  always_assert(t == TCls);
  return TObj;
}

Type objcls(const Type& t) {
  if (t.subtypeOf(BObj)) {
    if (is_specialized_obj(t)) {
      auto const d = dobj_of(t);
      return setctx(d.type == DObj::Exact ? clsExact(d.cls) : subCls(d.cls),
                    d.isCtx);
    } else {
      return TCls;
    }
  }
  // We can sometimes be given TTop or Class types.
  return t;
}

//////////////////////////////////////////////////////////////////////

folly::Optional<int64_t> arr_size(const Type& t) {
  switch (t.m_dataTag) {
    case DataTag::ArrLikeVal:
      return t.m_data.aval->size();

    case DataTag::ArrLikeMap:
      if (t.m_data.map->hasOptElements()) return folly::none;
      return t.m_data.map->map.size();

    case DataTag::ArrLikePacked:
      return t.m_data.packed->elems.size();

    case DataTag::None:
    case DataTag::Int:
    case DataTag::Dbl:
    case DataTag::Str:
    case DataTag::ArrLikePackedN:
    case DataTag::ArrLikeMapN:
    case DataTag::Obj:
    case DataTag::Cls:
    case DataTag::Record:
      return folly::none;
  }
  not_reached();
}

Type::ArrayCat categorize_array(const Type& t) {
  auto hasInts = false;
  auto hasStrs = false;
  auto isPacked = true;
  // Even if all the values are constants, we can't produce a constant array
  // unless the d/varray-ness is definitely known.
  auto val = t.subtypeOfAny(TPArr, TVArr, TDArr, TVec, TDict, TKeyset);
  size_t idx = 0;
  auto checkKey = [&] (const TypedValue& key) {
    if (isStringType(key.m_type)) {
      hasStrs = true;
      isPacked = false;
      return hasInts;
    } else {
      hasInts = true;
      if (key.m_data.num != idx++) isPacked = false;
      return hasStrs && !isPacked;
    }
  };

  switch (t.m_dataTag) {
    case DataTag::ArrLikeVal:
      IterateKV(t.m_data.aval,
                [&] (TypedValue k, TypedValue) {
                  return checkKey(k);
                });
      break;

    case DataTag::ArrLikeMap:
      if (t.m_data.map->hasOptElements()) return {};
      for (auto const& elem : t.m_data.map->map) {
        if (checkKey(elem.first) && !val) break;
        val = val && tv(elem.second);
      }
      break;

    case DataTag::ArrLikePacked:
      for (auto const& elem : t.m_data.packed->elems) {
        hasInts = true;
        val = val && tv(elem);
        if (!val) break;
      }
      break;

    case DataTag::None:
    case DataTag::Int:
    case DataTag::Dbl:
    case DataTag::Str:
    case DataTag::ArrLikePackedN:
    case DataTag::ArrLikeMapN:
    case DataTag::Obj:
    case DataTag::Cls:
    case DataTag::Record:
      return {};
  }

  auto cat =
    hasInts ? (isPacked ? Type::ArrayCat::Packed : Type::ArrayCat::Mixed) :
    hasStrs ? Type::ArrayCat::Struct : Type::ArrayCat::Empty;

  return { cat , val };
}

CompactVector<LSString> get_string_keys(const Type& t) {
  CompactVector<LSString> strs;

  switch (t.m_dataTag) {
    case DataTag::ArrLikeVal:
      IterateKV(t.m_data.aval,
                [&] (TypedValue k, TypedValue) {
                  assert(isStringType(k.m_type));
                  strs.push_back(k.m_data.pstr);
                });
      break;

    case DataTag::ArrLikeMap:
      assertx(!t.m_data.map->hasOptElements());
      for (auto const& elem : t.m_data.map->map) {
        assert(isStringType(elem.first.m_type));
        strs.push_back(elem.first.m_data.pstr);
      }
      break;

    case DataTag::ArrLikePacked:
    case DataTag::None:
    case DataTag::Int:
    case DataTag::Dbl:
    case DataTag::Str:
    case DataTag::ArrLikePackedN:
    case DataTag::ArrLikeMapN:
    case DataTag::Obj:
    case DataTag::Cls:
    case DataTag::Record:
      always_assert(false);
  }

  return strs;
}

template<typename R, bool force_static>
struct tvHelper {
  template<DataType dt,typename... Args>
  static R make(Args&&... args) {
    return make_tv<dt>(std::forward<Args>(args)...);
  }
  template<typename... Args>
  static R makePersistentArray(Args&&... args) {
    return make_persistent_array_like_tv(std::forward<Args>(args)...);
  }

  template<typename Init, typename... Args>
  static R fromMap(Args&&... args) {
    return fromTypeMap<Init, force_static>(std::forward<Args>(args)...);
  }
  template<typename Init, typename... Args>
  static R fromVec(Args&&... args) {
    return fromTypeVec<Init, force_static>(std::forward<Args>(args)...);
  }
};

template<bool ignored>
struct tvHelper<bool, ignored> {
  template <DataType dt, typename... Args>
  static bool make(Args&&... /*args*/) {
    return true;
  }
  template <typename... Args>
  static bool makePersistentArray(Args&&... /*args*/) {
    return true;
  }
  template<typename Init, typename... Args>
  static bool fromMap(Args&&... args) {
    return checkTypeMap(std::forward<Args>(args)...);
  }
  template<typename Init, typename... Args>
  static bool fromVec(Args&&... args) {
    return checkTypeVec(std::forward<Args>(args)...);
  }
};

template<typename R, bool force_static>
R tvImpl(const Type& t) {
  assert(t.checkInvariants());
  using H = tvHelper<R, force_static>;

  switch (t.m_bits) {
  case BUninit:      return H::template make<KindOfUninit>();
  case BInitNull:    return H::template make<KindOfNull>();
  case BTrue:        return H::template make<KindOfBoolean>(true);
  case BFalse:       return H::template make<KindOfBoolean>(false);
  case BPArrE:
  case BSPArrE:
    return H::makePersistentArray(staticEmptyArray());
  case BVArrE:
  case BSVArrE:
    assertx(!RuntimeOption::EvalHackArrDVArrs);
    if (t.m_dataTag == DataTag::ArrLikeVal) {
      return H::makePersistentArray(const_cast<ArrayData*>(t.m_data.aval));
    }
    return H::makePersistentArray(staticEmptyVArray());
  case BDArrE:
  case BSDArrE:
    assertx(!RuntimeOption::EvalHackArrDVArrs);
    if (t.m_dataTag == DataTag::ArrLikeVal) {
      return H::makePersistentArray(const_cast<ArrayData*>(t.m_data.aval));
    }
    return H::makePersistentArray(staticEmptyDArray());
  case BVecE:
  case BSVecE:
    if (t.m_dataTag == DataTag::ArrLikeVal) {
      return H::template make<KindOfPersistentVec>(t.m_data.aval);
    }
    return H::template make<KindOfPersistentVec>(staticEmptyVecArray());
  case BDictE:
  case BSDictE:
    if (t.m_dataTag == DataTag::ArrLikeVal) {
      return H::template make<KindOfPersistentDict>(t.m_data.aval);
    }
    return H::template make<KindOfPersistentDict>(staticEmptyDictArray());
  case BKeysetE:
  case BSKeysetE:
    return H::template make<KindOfPersistentKeyset>(staticEmptyKeysetArray());

  case BRecord:
    break;

  case BCStr:
  case BCArrE:
  case BCArrN:
  case BCArr:
  case BCPArrE:
  case BCPArrN:
  case BCPArr:
  case BCVArrE:
  case BCVArrN:
  case BCVArr:
  case BCDArrE:
  case BCDArrN:
  case BCDArr:
  case BCVecE:
  case BCVecN:
  case BCVec:
  case BCDictE:
  case BCDictN:
  case BCDict:
  case BCKeysetE:
  case BCKeysetN:
  case BCKeyset:
    // We don't produce these types.
    always_assert(false);

  default:
    if (t.couldBe(TNull)) {
      break;
    }
    switch (t.m_dataTag) {
    case DataTag::Int:
      return H::template make<KindOfInt64>(t.m_data.ival);
    case DataTag::Dbl:
      return H::template make<KindOfDouble>(t.m_data.dval);
    case DataTag::Str:
      return H::template make<KindOfPersistentString>(t.m_data.sval);
    case DataTag::ArrLikeVal:
      if (t.subtypeOf(BArrN)) {
        return H::makePersistentArray(
          const_cast<ArrayData*>(t.m_data.aval)
        );
      }
      if (t.subtypeOf(BVecN)) {
        return H::template make<KindOfPersistentVec>(
          const_cast<ArrayData*>(t.m_data.aval)
        );
      }
      if (t.subtypeOf(BDictN)) {
        return H::template make<KindOfPersistentDict>(
          const_cast<ArrayData*>(t.m_data.aval)
        );
      }
      if (t.subtypeOf(BKeysetN)) {
        return H::template make<KindOfPersistentKeyset>(
          const_cast<ArrayData*>(t.m_data.aval)
        );
      }
      break;
    case DataTag::ArrLikeMap:
      if (t.m_data.map->hasOptElements()) break;
      if (t.subtypeOf(BDictN)) {
        return H::template fromMap<DictInit>(t.m_data.map->map,
                                             t.m_data.map->provenance);
      } else if (t.subtypeOf(BKeysetN)) {
        return H::template fromMap<KeysetInit>(t.m_data.map->map,
                                               ProvTag::Top);
      } else if (t.subtypeOf(BPArrN)) {
        return H::template fromMap<MixedArrayInit>(t.m_data.map->map,
                                                   ProvTag::Top);
      } else if (t.subtypeOf(BDArrN)) {
        assertx(!RuntimeOption::EvalHackArrDVArrs);
        return H::template fromMap<DArrayInit>(t.m_data.map->map,
                                               t.m_data.map->provenance);
      }
      break;
    case DataTag::ArrLikePacked:
      if (t.subtypeOf(BVecN)) {
        return H::template fromVec<VecArrayInit>(t.m_data.packed->elems,
                                                 t.m_data.packed->provenance);
      } else if (t.subtypeOf(BDictN)) {
        return H::template fromVec<DictInit>(t.m_data.packed->elems,
                                             t.m_data.packed->provenance);
      } else if (t.subtypeOf(BKeysetN)) {
        return H::template fromVec<KeysetAppendInit>(t.m_data.packed->elems,
                                                     ProvTag::Top);
      } else if (t.subtypeOf(BPArrN)) {
        return H::template fromVec<PackedArrayInit>(t.m_data.packed->elems,
                                                    ProvTag::Top);
      } else if (t.subtypeOf(BVArrN)) {
        assertx(!RuntimeOption::EvalHackArrDVArrs);
        return H::template fromVec<VArrayInit>(t.m_data.packed->elems,
                                               t.m_data.packed->provenance);
      } else if (t.subtypeOf(BDArrN)) {
        assertx(!RuntimeOption::EvalHackArrDVArrs);
        return H::template fromVec<DArrayInit>(t.m_data.packed->elems,
                                               t.m_data.packed->provenance);
      }
      break;
    case DataTag::ArrLikePackedN:
    case DataTag::ArrLikeMapN:
    case DataTag::Obj:
    case DataTag::Cls:
    case DataTag::Record:
    case DataTag::None:
      break;
    }
  }

  return R{};
}

folly::Optional<TypedValue> tv(const Type& t) {
  return tvImpl<folly::Optional<TypedValue>, true>(t);
}

folly::Optional<TypedValue> tvNonStatic(const Type& t) {
  return tvImpl<folly::Optional<TypedValue>, false>(t);
}

bool is_scalar(const Type& t) {
  return tvImpl<bool, true>(t);
}

Type scalarize(Type t) {
  assertx(is_scalar(t));

  switch (t.m_dataTag) {
    case DataTag::None:
      assertx(t.subtypeOfAny(TNull, TTrue, TFalse,
                             TArrE, TVecE, TDictE, TKeysetE));
      t.m_bits &= BNull | BBool | BSArrE | BSVecE | BSDictE | BSKeysetE;
    case DataTag::Int:
    case DataTag::Dbl:
      return t;
    case DataTag::ArrLikeVal:
      t.m_bits &= BSArrN | BSVecN | BSDictN | BSKeysetN |
                  BSArrE | BSVecE | BSDictE;
      return t;
    case DataTag::Str:
      t.m_bits &= BSStr;
      return t;
    case DataTag::ArrLikeMap:
    case DataTag::ArrLikePacked:
      return from_cell(*tv(t));
    case DataTag::ArrLikePackedN:
    case DataTag::ArrLikeMapN:
    case DataTag::Obj:
    case DataTag::Cls:
    case DataTag::Record:
      break;
  }
  not_reached();
}

folly::Optional<size_t> array_size(const Type& t) {
  if (!t.subtypeOf(BArrLike)) return folly::none;
  switch (t.m_dataTag) {
    case DataTag::None:
      if (t.subtypeOf(BArrLikeE)) return 0;
      // fall through
    case DataTag::Int:
    case DataTag::Dbl:
    case DataTag::Str:
    case DataTag::ArrLikePackedN:
    case DataTag::ArrLikeMapN:
    case DataTag::Obj:
    case DataTag::Cls:
    case DataTag::Record:
      return folly::none;
    case DataTag::ArrLikeVal:
      return t.m_data.aval->size();
    case DataTag::ArrLikeMap:
      if (t.m_data.map->hasOptElements()) return folly::none;
      return t.m_data.map->map.size();
    case DataTag::ArrLikePacked:
      return t.m_data.packed->elems.size();
  }
  not_reached();
}

Type type_of_istype(IsTypeOp op) {
  using RO = RuntimeOption;
  switch (op) {
  case IsTypeOp::Null:   return TNull;
  case IsTypeOp::Bool:   return TBool;
  case IsTypeOp::Int:    return TInt;
  case IsTypeOp::Dbl:    return TDbl;
  case IsTypeOp::Str:    return TStrLike;
  case IsTypeOp::Res:    return TRes;
  case IsTypeOp::PHPArr:
  case IsTypeOp::Arr:
    return !RO::EvalHackArrDVArrs && RO::EvalIsCompatibleClsMethType
      ? TPArrLike : TArr;
  case IsTypeOp::Vec:
    return RO::EvalHackArrDVArrs && RO::EvalIsCompatibleClsMethType
      ? TVecLike : TVec;
  case IsTypeOp::Dict:   return TDict;
  case IsTypeOp::Keyset: return TKeyset;
  case IsTypeOp::Obj:    return TObj;
  case IsTypeOp::VArray:
    assertx(!RO::EvalHackArrDVArrs);
    return RO::EvalIsCompatibleClsMethType ? TVArrLike : TVArr;
  case IsTypeOp::DArray:
    assertx(!RO::EvalHackArrDVArrs);
    return TDArr;
  case IsTypeOp::ClsMeth: return TClsMeth;
  case IsTypeOp::Func: return TFunc;
  case IsTypeOp::ArrLike:
  case IsTypeOp::Scalar: always_assert(0);
  }
  not_reached();
}

folly::Optional<IsTypeOp> type_to_istypeop(const Type& t) {
  if (t.subtypeOf(BNull))   return IsTypeOp::Null;
  if (t.subtypeOf(BBool))   return IsTypeOp::Bool;
  if (t.subtypeOf(BInt))    return IsTypeOp::Int;
  if (t.subtypeOf(BDbl))    return IsTypeOp::Dbl;
  if (t.subtypeOf(BStr))    return IsTypeOp::Str;
  if (t.subtypeOf(BArr))    return IsTypeOp::Arr;
  if (t.subtypeOf(BVec))    return IsTypeOp::Vec;
  if (t.subtypeOf(BDict))   return IsTypeOp::Dict;
  if (t.subtypeOf(BRes))    return IsTypeOp::Res;
  if (t.subtypeOf(BKeyset)) return IsTypeOp::Keyset;
  if (t.subtypeOf(BObj))    return IsTypeOp::Obj;
  if (t.subtypeOf(BVArr)) {
    assertx(!RuntimeOption::EvalHackArrDVArrs);
    return IsTypeOp::VArray;
  }
  if (t.subtypeOf(BDArr)) {
    assertx(!RuntimeOption::EvalHackArrDVArrs);
    return IsTypeOp::DArray;
  }
  if (t.subtypeOf(BClsMeth)) return IsTypeOp::ClsMeth;
  if (t.subtypeOf(BFunc)) return IsTypeOp::Func;
  return folly::none;
}

folly::Optional<Type> type_of_type_structure(const Index& index,
                                             Context ctx,
                                             SArray ts) {
  auto const is_nullable = is_ts_nullable(ts);
  switch (get_ts_kind(ts)) {
    case TypeStructure::Kind::T_int:
      return is_nullable ? TOptInt : TInt;
    case TypeStructure::Kind::T_bool:
      return is_nullable ? TOptBool : TBool;
    case TypeStructure::Kind::T_float:
      return is_nullable ? TOptDbl : TDbl;
    case TypeStructure::Kind::T_string:
      return is_nullable ? TOptStrLike : TStrLike;
    case TypeStructure::Kind::T_resource:
      return is_nullable ? TOptRes : TRes;
    case TypeStructure::Kind::T_num:
      return is_nullable ? TOptNum : TNum;
    case TypeStructure::Kind::T_arraykey:
      return is_nullable ? TOptArrKey : TArrKey;
    case TypeStructure::Kind::T_dict:
      return is_nullable ? TOptDict : TDict;
    case TypeStructure::Kind::T_vec:
      if (RuntimeOption::EvalHackArrDVArrs) {
        return is_nullable ? TOptVecLike : TVecLike;
      }
      return is_nullable ? TOptVec : TVec;
    case TypeStructure::Kind::T_keyset:
      return is_nullable ? TOptKeyset : TKeyset;
    case TypeStructure::Kind::T_void:
    case TypeStructure::Kind::T_null:
      return TNull;
    case TypeStructure::Kind::T_tuple: {
      auto const tsElems = get_ts_elem_types(ts);
      std::vector<Type> v;
      for (auto i = 0; i < tsElems->size(); i++) {
        auto t = type_of_type_structure(
          index, ctx, tsElems->getValue(i).getArrayData());
        if (!t) return folly::none;
        v.emplace_back(std::move(t.value()));
      }
      if (v.empty()) return folly::none;
      auto const arrT = arr_packed_varray(v, ProvTag::Top);
      return is_nullable ? union_of(std::move(arrT), TNull) : arrT;
    }
    case TypeStructure::Kind::T_shape: {
      // Taking a very conservative approach to shapes where we dont do any
      // conversions if the shape contains unknown or optional fields
      if (does_ts_shape_allow_unknown_fields(ts)) return folly::none;
      auto map = MapElems{};
      auto const fields = get_ts_fields(ts);
      for (auto i = 0; i < fields->size(); i++) {
        auto key = fields->getKey(i).getStringData();
        auto const wrapper = fields->getValue(i).getArrayData();

        // Shapes can be defined using class constants, these keys must be
        // resolved, and to side-step the issue of shapes potentially being
        // packed arrays if the keys are consecutive integers beginning with 0,
        // we allow only keys that resolve to strings here.
        if (wrapper->exists(s_is_cls_cns)) {
          std::string cls, cns;
          auto const matched = folly::split("::", key->data(), cls, cns);
          always_assert(matched);

          auto const rcls = index.resolve_class(ctx, makeStaticString(cls));
          if (!rcls || index.lookup_class_init_might_raise(ctx, *rcls)) {
            return folly::none;
          }
          auto const tcns = index.lookup_class_constant(
            ctx, *rcls, makeStaticString(cns), false);

          auto const vcns = tv(tcns);
          if (!vcns || !isStringType(type(*vcns))) return folly::none;
          key = val(*vcns).pstr;
        }

        // Optional fields are hard to represent as a type
        if (is_optional_ts_shape_field(wrapper)) return folly::none;
        auto t = type_of_type_structure(index, ctx, get_ts_value(wrapper));
        if (!t) return folly::none;
        map.emplace_back(
          make_tv<KindOfPersistentString>(key), std::move(t.value()));
      }
      if (map.empty()) return folly::none;
      auto const arrT = arr_map_darray(map);
      return is_nullable ? union_of(std::move(arrT), TNull) : arrT;
    }
    case TypeStructure::Kind::T_vec_or_dict:
      // Ideally, we would return this union; but thats not an allowed type, so
      // we end up with TInitCell as the result, which makes hhbbc think that
      // the condition is always true.
      //
      // return is_nullable ?
      //   union_of(TOptVec, TOptDict) : union_of(TVec, TDict);
      return folly::none;
    case TypeStructure::Kind::T_arraylike:
      // Similar to the above, we can't (yet) do this.
      //
      // return is_nullable
      //  ? union_of(union_of(union_of(TOptArr, TOptVec), TOptDict), TOptKeyset)
      //  : union_of(union_of(union_of(TArr, TVec), TDict), TKeyset);
      return folly::none;

    case TypeStructure::Kind::T_nothing:
    case TypeStructure::Kind::T_noreturn:
    case TypeStructure::Kind::T_mixed:
    case TypeStructure::Kind::T_dynamic:
    case TypeStructure::Kind::T_nonnull:
    case TypeStructure::Kind::T_class:
    case TypeStructure::Kind::T_interface:
    case TypeStructure::Kind::T_unresolved:
    case TypeStructure::Kind::T_typeaccess:
    case TypeStructure::Kind::T_array:
    case TypeStructure::Kind::T_darray:
    case TypeStructure::Kind::T_varray:
    case TypeStructure::Kind::T_varray_or_darray:
    case TypeStructure::Kind::T_xhp:
    case TypeStructure::Kind::T_enum:
    case TypeStructure::Kind::T_fun:
    case TypeStructure::Kind::T_typevar:
    case TypeStructure::Kind::T_trait:
    case TypeStructure::Kind::T_reifiedtype:
      return folly::none;
  }

  not_reached();
}

DObj dobj_of(const Type& t) {
  assert(t.checkInvariants());
  assert(is_specialized_obj(t));
  return t.m_data.dobj;
}

DRecord drec_of(const Type& t) {
  assert(t.checkInvariants());
  assert(is_specialized_record(t));
  return t.m_data.drec;
}

DCls dcls_of(Type t) {
  assert(t.checkInvariants());
  assert(is_specialized_cls(t));
  return t.m_data.dcls;
}

SString sval_of(const Type& t) {
  assert(t.checkInvariants());
  assert(is_specialized_string(t));
  return t.m_data.sval;
}

Type from_cell(TypedValue cell) {
  assert(tvIsPlausible(cell));

  switch (cell.m_type) {
  case KindOfUninit:   return TUninit;
  case KindOfNull:     return TInitNull;
  case KindOfBoolean:  return cell.m_data.num ? TTrue : TFalse;
  case KindOfInt64:    return ival(cell.m_data.num);
  case KindOfDouble:   return dval(cell.m_data.dbl);

  case KindOfPersistentString:
  case KindOfString:
    always_assert(cell.m_data.pstr->isStatic());
    return sval(cell.m_data.pstr);

  case KindOfPersistentVec:
  case KindOfVec:
    always_assert(cell.m_data.parr->isStatic());
    always_assert(cell.m_data.parr->isVecArrayType());
    return vec_val(cell.m_data.parr);

  case KindOfPersistentDict:
  case KindOfDict:
    always_assert(cell.m_data.parr->isStatic());
    always_assert(cell.m_data.parr->isDictType());
    return dict_val(cell.m_data.parr);

  case KindOfPersistentKeyset:
  case KindOfKeyset:
    always_assert(cell.m_data.parr->isStatic());
    always_assert(cell.m_data.parr->isKeysetType());
    return keyset_val(cell.m_data.parr);

  case KindOfPersistentDArray:
  case KindOfDArray:
  case KindOfPersistentVArray:
  case KindOfVArray:
  case KindOfPersistentArray:
  case KindOfArray:
    always_assert(cell.m_data.parr->isStatic());
    always_assert(cell.m_data.parr->isPHPArrayType());
    return aval(cell.m_data.parr);

  case KindOfObject:
  case KindOfResource:
  case KindOfFunc:
  case KindOfClass:
  case KindOfClsMeth:
  case KindOfRecord:
    break;
  }
  always_assert(
      0 && "reference counted/class/func/clsmeth/record type in from_cell");
}

Type from_DataType(DataType dt) {
  switch (dt) {
  case KindOfUninit:   return TUninit;
  case KindOfNull:     return TInitNull;
  case KindOfBoolean:  return TBool;
  case KindOfInt64:    return TInt;
  case KindOfDouble:   return TDbl;
  case KindOfPersistentString:
  case KindOfString:   return TStr;
  case KindOfPersistentVec:
  case KindOfVec:      return TVec;
  case KindOfPersistentDict:
  case KindOfDict:     return TDict;
  case KindOfPersistentKeyset:
  case KindOfKeyset:   return TKeyset;
  case KindOfRecord:   return TRecord;
  case KindOfPersistentDArray:
  case KindOfDArray:   return TDArr;
  case KindOfPersistentVArray:
  case KindOfVArray:   return TVArr;
  case KindOfPersistentArray:
  case KindOfArray:    return TArr;
  case KindOfObject:   return TObj;
  case KindOfResource: return TRes;
  case KindOfFunc:     return TFunc;
  case KindOfClass:    return TCls;
  case KindOfClsMeth:  return TClsMeth;
  }
  always_assert(0 && "dt in from_DataType didn't satisfy preconditions");
}

Type from_hni_constraint(SString s) {
  if (!s) return TCell;

  auto p   = s->data();
  auto ret = TBottom;

  if (*p == '?') {
    ret |= TInitNull;
    ++p;
  }

  if (!strcasecmp(p, "HH\\null"))     return union_of(ret, TInitNull);
  if (!strcasecmp(p, "HH\\resource")) return union_of(ret, TRes);
  if (!strcasecmp(p, "HH\\bool"))     return union_of(ret, TBool);
  if (!strcasecmp(p, "HH\\int"))      return union_of(ret, TInt);
  if (!strcasecmp(p, "HH\\float"))    return union_of(ret, TDbl);
  if (!strcasecmp(p, "HH\\num"))      return union_of(ret, TNum);
  if (!strcasecmp(p, "HH\\string"))   return union_of(ret, TStr);
  if (!strcasecmp(p, "HH\\arraykey")) return union_of(ret, TArrKey);
  if (!strcasecmp(p, "HH\\dict"))     return union_of(ret, TDict);
  if (!strcasecmp(p, "HH\\vec"))      return union_of(ret, TVec);
  if (!strcasecmp(p, "HH\\keyset"))   return union_of(ret, TKeyset);
  if (!strcasecmp(p, "HH\\varray")) {
    return union_of(ret, RuntimeOption::EvalHackArrDVArrs ? TVec : TArr);
  }
  if (!strcasecmp(p, "HH\\darray")) {
    return union_of(ret, RuntimeOption::EvalHackArrDVArrs ? TDict : TArr);
  }
  if (!strcasecmp(p, "HH\\varray_or_darray")) {
    if (RuntimeOption::EvalHackArrDVArrs) {
      return union_of(ret, union_of(TVec, TDict));
    }
    return union_of(ret, TArr);
  }
  if (!strcasecmp(p, "HH\\vec_or_dict")) {
    return union_of(ret, union_of(TVec, TDict));
  }
  if (!strcasecmp(p, "HH\\arraylike")) {
    return union_of(ret,
                    union_of(TArr, union_of(TVec, union_of(TDict, TKeyset))));
  }
  if (!strcasecmp(p, "array"))        return union_of(ret, TArr);
  if (!strcasecmp(p, "HH\\mixed"))    return TInitCell;
  if (!strcasecmp(p, "HH\\nonnull"))  return TInitCell;

  // It might be an object, or we might want to support type aliases in HNI at
  // some point.  For now just be conservative.
  return TCell;
}

Type intersection_of(Type a, Type b) {
  auto isect = a.m_bits & b.m_bits;
  if (!mayHaveData(isect)) return Type { isect };

  auto fix = [&] (Type& t) {
    t.m_bits = isect;
    return std::move(t);
  };

  auto aProjected = project_data(a, isect);
  auto bProjected = project_data(b, isect);

  auto const isStatic = [] (trep b) {
    return ((b & BUnc) == b);
  };

  // The intersection is non-empty. If either type has data (but not
  // both), the intersection will contain that data. If the
  // intersection is static, we need to remove any counted types from
  // the data first (if there's no types left afterwards, the
  // intersection doesn't actually exist).
  if (!bProjected.hasData()) {
    if (isStatic(isect) &&
        !isStatic(aProjected.m_bits) &&
        aProjected.hasData()) {
      aProjected = remove_counted(std::move(aProjected));
      if (aProjected == TBottom) return TBottom;
      isect &= aProjected.m_bits;
    }
    return fix(aProjected);
  }
  if (!aProjected.hasData()) {
    if (isStatic(isect) &&
        !isStatic(bProjected.m_bits) &&
        bProjected.hasData()) {
      bProjected = remove_counted(std::move(bProjected));
      if (bProjected == TBottom) return TBottom;
      isect &= bProjected.m_bits;
    }
    return fix(bProjected);
  }

  // Otherwise we have data for both types, which we need to
  // intersect.
  if (aProjected.subtypeData<true>(bProjected)) return fix(aProjected);
  if (bProjected.subtypeData<true>(aProjected)) return fix(bProjected);

  /* from this point on, the projection doesn't matter since neither a or b
   * isn't supported by a type bit in the intersection */
  assert(aProjected.hasData() && aProjected == a);
  assert(bProjected.hasData() && bProjected == b);

  auto t = [&] {
    if (a.m_dataTag == b.m_dataTag) {
      switch (a.m_dataTag) {
        case DataTag::None:
          not_reached();
        case DataTag::Obj:
        {
          auto fixWh = [&] (Type& t) {
            if (!a.m_data.dobj.whType) {
              t.m_data.dobj.whType = b.m_data.dobj.whType;
            } else if (!b.m_data.dobj.whType) {
              t.m_data.dobj.whType = a.m_data.dobj.whType;
            } else {
              auto whType = intersection_of(*a.m_data.dobj.whType,
                                            *b.m_data.dobj.whType);
              if (whType == TBottom) return TBottom;
              *t.m_data.dobj.whType.mutate() = whType;
            }
            t = setctx(t, a.m_data.dobj.isCtx || b.m_data.dobj.isCtx);
            return fix(t);
          };
          if (a.m_data.dobj.type == b.m_data.dobj.type &&
              a.m_data.dobj.cls.same(b.m_data.dobj.cls)) {
            return fixWh(a);
          }
          if (b.m_data.dobj.type == DObj::Sub &&
              a.m_data.dobj.cls.mustBeSubtypeOf(b.m_data.dobj.cls)) {
            return fixWh(a);
          }
          if (a.m_data.dobj.type == DObj::Sub &&
              b.m_data.dobj.cls.mustBeSubtypeOf(a.m_data.dobj.cls)) {
            return fixWh(b);
          }
          if (a.m_data.dobj.type == DObj::Sub &&
              b.m_data.dobj.type == DObj::Sub) {
            if (a.m_data.dobj.cls.couldBeInterface()) {
              if (!b.m_data.dobj.cls.couldBeInterface()) {
                return fixWh(b);
              } else {
                return Type { isect };
              }
            } else if (b.m_data.dobj.cls.couldBeInterface()) {
              return fixWh(a);
            }
          }
          return TBottom;
        }
        case DataTag::Cls:
          // We need to handle cases where one is tagged as the context and the
          // other isn't.
          if (a.subtypeData<false>(b)) {
            auto t = setctx(a);
            return fix(t);
          }
          if (b.subtypeData<false>(a)) {
            auto t = setctx(b);
            return fix(t);
          }
          return TBottom;
        case DataTag::Record:
          if (a.subtypeData<false>(b)) {
            return fix(a);
          }
          if (b.subtypeData<false>(a)) {
            return fix(b);
          }
          return TBottom;
        case DataTag::Str:
        case DataTag::ArrLikeVal:
        case DataTag::Int:
        case DataTag::Dbl:
          // Neither is a subtype of the other, so the intersection is empty
          return TBottom;
        case DataTag::ArrLikePacked:
        case DataTag::ArrLikePackedN:
        case DataTag::ArrLikeMap:
        case DataTag::ArrLikeMapN:
          // will be handled by dual dispatch.
          break;
      }
    }
    return a.dualDispatchDataFn(b, DualDispatchIntersection{ isect });
  }();

  if (t != TBottom) return t;
  auto const bits =
    isect & ~(BInt|BDbl|BSStr|BArrN|BVecN|BDictN|BKeysetN|BObj|BRecord);
  return Type { bits };
}

Type Type::unionArrLike(Type a, Type b) {
  auto const newBits = combine_arr_like_bits(a.m_bits, b.m_bits);
  /* the call to project_data here is because adding bits to the trep
   * can cause us to lose data also, e.g. if a ArrN bit is added to
   * what was previously known to be an empty array */
  auto a_projected = project_data(a, newBits);
  auto b_projected = project_data(b, newBits);
  if (!b_projected.hasData()) {
    return set_trep(a_projected, newBits);
  }
  if (!a_projected.hasData()) {
    return set_trep(b_projected, newBits);
  }
  /* at this point we know the projection removed nothing */
  assert(a.hasData() && a_projected.hasData());
  assert(b.hasData() && b_projected.hasData());
  if (a.subtypeData<true>(b)) {
    return set_trep(b, newBits);
  }
  if (b.subtypeData<true>(a)) {
    return set_trep(a, newBits);
  }
  return a.dualDispatchDataFn(b, DualDispatchUnion{ newBits });
}

Type union_of(Type a, Type b) {
  auto const nullbits = (a.m_bits | b.m_bits) & BNull;

  auto nullify = [&] (Type& dst) {
    dst.m_bits |= nullbits;
    assertx(isPredefined(dst.m_bits));
    return dst;
  };

  if (is_nullish(a)) {
    if (unnullish(a).subtypeOfImpl<true>(b)) {
      return nullify(b);
    }
  } else if (a.subtypeOfImpl<true>(b)) {
    return b;
  }

  if (is_nullish(b)) {
    if (unnullish(b).subtypeOfImpl<true>(a)) {
      return nullify(a);
    }
  } else if (b.subtypeOfImpl<true>(a)) {
    return a;
  }

  /*
   * We need to check this before specialized objects, including the case where
   * one of them was TInitNull, because otherwise we'll go down the
   * is_specialized_obj paths and lose the wait handle information.
   */
  if (is_specialized_wait_handle(a)) {
    if (is_specialized_wait_handle(b)) {
      *a.m_data.dobj.whType.mutate() |= *b.m_data.dobj.whType;
      return nullify(a);
    }
    if (b.subtypeOf(TNull)) {
      return nullify(a);
    }
  }
  if (is_specialized_wait_handle(b)) {
    if (a.subtypeOf(TNull)) {
      return nullify(b);
    }
  }

  // When both types are strict subtypes of nullish TObj or both
  // are strict subtypes of TCls we look for a common ancestor if one
  // exists.
  if (is_specialized_obj(a) && is_specialized_obj(b)) {
    auto t = a.m_data.dobj.cls.commonAncestor(dobj_of(b).cls);
    // We need not to distinguish between Obj<=T and Obj=T, and always
    // return an Obj<=Ancestor, because that is the single type that
    // includes both children.
    auto const isCtx = a.m_data.dobj.isCtx && b.m_data.dobj.isCtx;
    auto ret = t ? subObj(*t) : TObj;
    return setctx(nullify(ret), isCtx);
  }
  if (is_specialized_record(a) && is_specialized_record(b)) {
    auto t = a.m_data.drec.rec.commonAncestor(drec_of(b).rec);
    auto ret = t ? subRecord(*t) : TRecord;
    return nullify(ret);
  }
  if (a.strictSubtypeOf(TCls) && b.strictSubtypeOf(TCls)) {
    auto t = a.m_data.dcls.cls.commonAncestor(dcls_of(b).cls);
    // Similar to above, this must always return an Obj<=Ancestor.
    auto const isCtx = a.m_data.dcls.isCtx && b.m_data.dcls.isCtx;
    return setctx(t ? subCls(*t) : TCls, isCtx);
  }

  if (is_specialized_array(a)) {
    auto t = spec_array_like_union(a, b, BArrE, BArr);
    if (t != TBottom) return t;
  } else if (is_specialized_array(b)) {
    auto t = spec_array_like_union(b, a, BArrE, BArr);
    if (t != TBottom) return t;
  }

  if (is_specialized_vec(a)) {
    auto t = spec_array_like_union(a, b, BVecE, BVec);
    if (t != TBottom) return t;
  } else if (is_specialized_vec(b)) {
    auto t = spec_array_like_union(b, a, BVecE, BVec);
    if (t != TBottom) return t;
  }

  if (is_specialized_dict(a)) {
    auto t = spec_array_like_union(a, b, BDictE, BDict);
    if (t != TBottom) return t;
  } else if (is_specialized_dict(b)) {
    auto t = spec_array_like_union(b, a, BDictE, BDict);
    if (t != TBottom) return t;
  }

  if (is_specialized_keyset(a)) {
    auto t = spec_array_like_union(a, b, BKeysetE, BKeyset);
    if (t != TBottom) return t;
  } else if (is_specialized_keyset(b)) {
    auto t = spec_array_like_union(b, a, BKeysetE, BKeyset);
    if (t != TBottom) return t;
  }

  /*
   * Merging option types tries to preserve subtype information where it's
   * possible.  E.g. if you union InitNull and Obj<=Foo, we want OptObj<=Foo to
   * be the result.
   */
  if (isPredefined(a.m_bits | b.m_bits)) {
    if (a.subtypeOf(BNull)) {
      b.m_bits |= a.m_bits;
      return b;
    }
    if (b.subtypeOf(BNull)) {
      a.m_bits |= b.m_bits;
      return a;
    }
  }

#define X(y) if (a.subtypeOf(B ## y) && b.subtypeOf(B ## y)) return T ## y;
#define Y(y)                                                            \
  X(y)                                                                  \
  if (a.subtypeOf(B ## y | BInitNull) &&                                \
      b.subtypeOf(B ## y | BInitNull)) return TOpt ## y;                \
  if (a.subtypeOf(B ## y | BUninit) &&                                  \
      b.subtypeOf(B ## y | BUninit)) return Type{B ## y | BUninit};     \
  if (a.subtypeOf(B ## y | BNull) &&                                    \
      b.subtypeOf(B ## y | BNull)) return Type{B ## y | BNull};

  // non-optional types
  X(Null)

  // optional types
  Y(Bool)
  Y(Int)
  Y(Dbl)
  Y(Num)
  Y(SStr)
  Y(Str)
  Y(Obj)
  Y(Cls)
  Y(Record)

  Y(SPArr)
  Y(PArrE)
  Y(PArrN)
  Y(PArr)

  Y(SVArr)
  Y(VArrE)
  Y(VArrN)
  Y(VArr)

  Y(SDArr)
  Y(DArrE)
  Y(DArrN)
  Y(DArr)

  Y(SArrE)
  Y(SArrN)
  Y(SArr)
  Y(ArrE)
  Y(ArrN)
  Y(Arr)

  Y(SVec)
  Y(VecE)
  Y(VecN)
  Y(Vec)

  Y(SDict)
  Y(DictE)
  Y(DictN)
  Y(Dict)

  Y(SKeyset)
  Y(KeysetE)
  Y(KeysetN)
  Y(Keyset)

  Y(UncArrKey)
  Y(ArrKey)

  Y(FuncOrCls)

  Y(UncStrLike)
  Y(StrLike)

  Y(VArrLikeSA)
  Y(VArrLike)

  Y(VecLikeSA)
  Y(VecLike)

  Y(PArrLikeSA)
  Y(PArrLike)

  // non-optional types that contain other types above (and hence
  // must come after them).
  X(InitPrim)
  X(Prim)
  X(InitUnc)
  X(Unc)
  X(InitCell)
  X(Cell)

#undef Y
#undef X

  return TTop;
}

Type promote_emptyish(Type a, Type b) {
  if (is_opt(a)) a = unopt(a);
  if (a.subtypeOf(sempty())) {
    return b;
  }
  auto t = trep(a.m_bits & ~(BNull | BFalse));
  if (!isPredefined(t)) {
    if (trep(t & BInitPrim) == t) {
      t = BInitPrim;
    } else if (trep(t & BInitUnc) == t) {
      t = BInitUnc;
    } else {
      t = BInitCell;
    }
    return union_of(Type { t }, b);
  }
  a.m_bits = t;
  return union_of(a, b);
}

bool could_have_magic_bool_conversion(const Type& t) {
  if (!t.couldBe(BObj)) return false;

  if (t.strictSubtypeOf(TObj) ||
      (is_opt(t) && unopt(t).strictSubtypeOf(TObj))) {
    return dobj_of(t).cls.couldHaveMagicBool();
  }
  return true;
}

Emptiness emptiness(const Type& t) {
  auto const empty_mask = BNull | BFalse | BArrE | BVecE | BDictE | BKeysetE;
  if ((t.m_bits & empty_mask) == t.m_bits) return Emptiness::Empty;
  auto const non_empty_mask = BTrue | BArrN | BVecN | BDictN | BKeysetN;
  if ((t.m_bits & non_empty_mask) == t.m_bits) return Emptiness::NonEmpty;
  if (t.strictSubtypeOf(TObj)) {
    if (!could_have_magic_bool_conversion(t)) {
      return Emptiness::NonEmpty;
    }
  }

  if (is_opt(t)) {
    // Something like ?Int=0 is always empty, but ?Int=1 may or may not be.
    if (auto v = tv(unopt(t))) {
      return tvToBool(*v) ? Emptiness::Maybe : Emptiness::Empty;
    }
  } else if (auto v = tv(t)) {
    return tvToBool(*v) ? Emptiness::NonEmpty : Emptiness::Empty;
  }

  return Emptiness::Maybe;
}

void widen_type_impl(Type& t, uint32_t depth) {
  // Right now to guarantee termination we need to just limit the nesting depth
  // of the type to a fixed degree.
  auto const checkDepth = [&] {
    if (depth >= kTypeWidenMaxDepth) {
      t = Type { t.m_bits };
      return true;
    }
    return false;
  };

  switch (t.m_dataTag) {
    case DataTag::None:
    case DataTag::Str:
    case DataTag::Int:
    case DataTag::Dbl:
    case DataTag::Cls:
    case DataTag::Record:
    case DataTag::ArrLikeVal:
      return;

    case DataTag::Obj:
      if (t.m_data.dobj.whType) {
        widen_type_impl(*t.m_data.dobj.whType.mutate(), depth + 1);
      }
      return;

    case DataTag::ArrLikePacked: {
      if (checkDepth()) return;
      auto& packed = *t.m_data.packed.mutate();
      for (auto& e : packed.elems) {
        widen_type_impl(e, depth + 1);
      }
      return;
    }

    case DataTag::ArrLikePackedN: {
      if (checkDepth()) return;
      auto& packed = *t.m_data.packedn.mutate();
      widen_type_impl(packed.type, depth + 1);
      return;
    }

    case DataTag::ArrLikeMap: {
      if (checkDepth()) return;
      auto& map = *t.m_data.map.mutate();
      for (auto it = map.map.begin(); it != map.map.end(); it++) {
        auto temp = it->second;
        widen_type_impl(temp, depth + 1);
        map.map.update(it, std::move(temp));
      }
      if (map.hasOptElements()) {
        // Key must be at least ArrKey, which doesn't need widening.
        widen_type_impl(map.optVal, depth + 1);
      }
      return;
    }

    case DataTag::ArrLikeMapN: {
      if (checkDepth()) return;
      auto& map = *t.m_data.mapn.mutate();
      // Key must be at least ArrKey, which doesn't need widening.
      widen_type_impl(map.val, depth + 1);
      return;
    }
  }

  not_reached();
}

Type widen_type(Type t) {
  widen_type_impl(t, 0);
  return t;
}

Type widening_union(const Type& a, const Type& b) {
  if (a.subtypeOf(b)) return b;
  if (b.subtypeOf(a)) return a;
  return widen_type(union_of(a, b));
}

Type stack_flav(Type a) {
  if (a.subtypeOf(BUninit))   return TUninit;
  if (a.subtypeOf(BInitCell)) return TInitCell;
  always_assert(0 && "stack_flav passed invalid type");
}

Type loosen_staticness(Type t) {
  auto bits = t.m_bits;
  if (TInitUnc.subtypeOf(bits)) return union_of(t, TInitCell);

  auto const check = [&] (trep a) {
    if (bits & a) bits |= a;
  };
  check(BStr);
  check(BPArrE);
  check(BPArrN);
  check(BVArrE);
  check(BVArrN);
  check(BDArrE);
  check(BDArrN);
  check(BVecE);
  check(BVecN);
  check(BDictE);
  check(BDictN);
  check(BKeysetE);
  check(BKeysetN);

  assertx(isPredefined(bits));
  t.m_bits = bits;

  switch (t.m_dataTag) {
    case DataTag::None:
    case DataTag::Str:
    case DataTag::Int:
    case DataTag::Dbl:
    case DataTag::Cls:
    case DataTag::Record:
    case DataTag::ArrLikeVal:
      break;

    case DataTag::Obj:
      if (t.m_data.dobj.whType) {
        auto whType = t.m_data.dobj.whType.mutate();
        *whType = loosen_staticness(std::move(*whType));
      }
      break;

    case DataTag::ArrLikePacked: {
      auto& packed = *t.m_data.packed.mutate();
      for (auto& e : packed.elems) {
        e = loosen_staticness(std::move(e));
      }
      break;
    }

    case DataTag::ArrLikePackedN: {
      auto& packed = *t.m_data.packedn.mutate();
      packed.type = loosen_staticness(std::move(packed.type));
      break;
    }

    case DataTag::ArrLikeMap: {
      auto& map = *t.m_data.map.mutate();
      for (auto it = map.map.begin(); it != map.map.end(); it++) {
        map.map.update(it, loosen_staticness(it->second));
      }
      map.optKey = loosen_staticness(std::move(map.optKey));
      map.optVal = loosen_staticness(std::move(map.optVal));
      break;
    }

    case DataTag::ArrLikeMapN: {
      auto& map = *t.m_data.mapn.mutate();
      map.key = loosen_staticness(std::move(map.key));
      map.val = loosen_staticness(std::move(map.val));
      break;
    }
  }

  return t;
}

Type loosen_dvarrayness(Type t) {
  auto const check = [&] (trep a) {
    if (t.m_bits & a) t.m_bits |= a;
  };
  if (t.couldBe(BArr) && t.m_dataTag == DataTag::ArrLikeVal) {
    // We need to drop any static array from the type because TArr unions cannot
    // have one. Turn it into the equivalent Packed or Map data.
    if (t.m_data.aval->empty()) {
      t = loosen_values(t);
    } else if (auto p = toDArrLikePacked(t.m_data.aval)) {
      t = packed_impl(t.m_bits, std::move(p->elems), p->provenance);
    } else {
      auto d = toDArrLikeMap(t.m_data.aval);
      t = map_impl(
        t.m_bits,
        std::move(d->map),
        std::move(d->optKey),
        std::move(d->optVal),
        d->provenance
      );
    }
  }
  check(BSArrE);
  check(BCArrE);
  check(BSArrN);
  check(BCArrN);
  return t;
}

Type loosen_provenance(Type t) {
  if (!RuntimeOption::EvalArrayProvenance) return t;
  if (t.couldBe(kProvBits)) {
    switch (t.m_dataTag) {
    case DataTag::None:
    case DataTag::Str:
    case DataTag::Int:
    case DataTag::Dbl:
    case DataTag::Cls:
    case DataTag::Record:
      break;

    case DataTag::ArrLikeVal:
      if (arrprov::getTag(t.m_data.aval)) {
        auto ad = t.m_data.aval->copy();
        ArrayData::GetScalarArray(&ad);
        t.m_data.aval = ad;
      }
      break;

    case DataTag::Obj:
      if (t.m_data.dobj.whType) {
        auto whType = t.m_data.dobj.whType.mutate();
        *whType = loosen_provenance(std::move(*whType));
      }
      break;

    case DataTag::ArrLikePacked: {
      auto& packed = *t.m_data.packed.mutate();
      packed.provenance = ProvTag::Top;
      for (auto& e : packed.elems) {
        e = loosen_provenance(std::move(e));
      }
      break;
    }

    case DataTag::ArrLikePackedN: {
      auto& packed = *t.m_data.packedn.mutate();
      packed.type = loosen_provenance(std::move(packed.type));
      break;
    }

    case DataTag::ArrLikeMap: {
      auto& map = *t.m_data.map.mutate();
      map.provenance = ProvTag::Top;
      for (auto it = map.map.begin(); it != map.map.end(); it++) {
        map.map.update(it, loosen_provenance(it->second));
      }
      map.optKey = loosen_provenance(std::move(map.optKey));
      map.optVal = loosen_provenance(std::move(map.optVal));
      break;
    }

    case DataTag::ArrLikeMapN: {
      auto& map = *t.m_data.mapn.mutate();
      map.key = loosen_provenance(std::move(map.key));
      map.val = loosen_provenance(std::move(map.val));
      break;
    }
    }
  }
  return t;
}

Type loosen_arrays(Type a) {
  if (a.couldBe(BArr))     a |= TArr;
  if (a.couldBe(BVec))     a |= TVec;
  if (a.couldBe(BDict))    a |= TDict;
  if (a.couldBe(BKeyset))  a |= TKeyset;
  if (a.couldBe(BClsMeth)) a |= RO::EvalHackArrDVArrs ? TVecLike : TPArrLike;
  return a;
}

Type loosen_values(Type a) {
  auto t = [&]{
    switch (a.m_dataTag) {
    case DataTag::Str:
    case DataTag::Int:
    case DataTag::Dbl:
    case DataTag::ArrLikeVal:
    case DataTag::ArrLikePacked:
    case DataTag::ArrLikePackedN:
    case DataTag::ArrLikeMap:
    case DataTag::ArrLikeMapN:
      return Type { a.m_bits };
    case DataTag::None:
    case DataTag::Obj:
    case DataTag::Cls:
    case DataTag::Record:
      return a;
    }
    not_reached();
  }();
  if (t.couldBe(BFalse) || t.couldBe(BTrue)) t |= TBool;
  return t;
}

Type loosen_emptiness(Type t) {
  auto const check = [&] (trep a, trep b) {
    if (t.m_bits & a) t.m_bits |= b;
  };
  check(BSPArr,   BSPArr);
  check(BCPArr,   BPArr);
  check(BSVArr,   BSVArr);
  check(BCVArr,   BVArr);
  check(BSDArr,   BSDArr);
  check(BCDArr,   BDArr);
  check(BSVec,    BSVec);
  check(BCVec,    BVec);
  check(BSDict,   BSDict);
  check(BCDict,   BDict);
  check(BSKeyset, BSKeyset);
  check(BCKeyset, BKeyset);
  return project_data(t, t.m_bits);
}

Type loosen_likeness(Type t) {
  if (RuntimeOption::EvalIsCompatibleClsMethType && t.couldBe(BClsMeth)) {
    if (!RuntimeOption::EvalHackArrDVArrs) {
      // Ideally we would loosen to TVArrLike, however, varray and darray need
      // to behave the same in most instances.
      t = union_of(std::move(t), TPArrLike);
    } else {
      t = union_of(std::move(t), TVecLike);
    }
  }

  if (t.couldBe(BFunc | BCls)) t = union_of(std::move(t), TUncStrLike);

  switch (t.m_dataTag) {
  case DataTag::None:
  case DataTag::Str:
  case DataTag::Int:
  case DataTag::Dbl:
  case DataTag::Cls:
  case DataTag::Record:
    break;

  case DataTag::ArrLikeVal:
    // Static arrays cannot currently contain function or class pointers.
    break;

  case DataTag::Obj:
    if (t.m_data.dobj.whType) {
      auto whType = t.m_data.dobj.whType.mutate();
      *whType = loosen_likeness(std::move(*whType));
    }
    break;

  case DataTag::ArrLikePacked: {
    auto& packed = *t.m_data.packed.mutate();
    for (auto& e : packed.elems) {
      e = loosen_likeness(std::move(e));
    }
    break;
  }

  case DataTag::ArrLikePackedN: {
    auto& packed = *t.m_data.packedn.mutate();
    packed.type = loosen_likeness(std::move(packed.type));
    break;
  }

  case DataTag::ArrLikeMap: {
    auto& map = *t.m_data.map.mutate();
    for (auto it = map.map.begin(); it != map.map.end(); it++) {
      map.map.update(it, loosen_likeness(it->second));
    }
    map.optKey = loosen_likeness(std::move(map.optKey));
    map.optVal = loosen_likeness(std::move(map.optVal));
    break;
  }

  case DataTag::ArrLikeMapN: {
    auto& map = *t.m_data.mapn.mutate();
    map.key = loosen_likeness(std::move(map.key));
    map.val = loosen_likeness(std::move(map.val));
    break;
  }
  }
  return t;
}

Type loosen_all(Type t) {
  return loosen_dvarrayness(
    loosen_staticness(
      loosen_emptiness(
        loosen_values(
          loosen_likeness(std::move(t))
        )
      )
    )
  );
}

Type add_nonemptiness(Type t) {
  auto const check = [&] (trep a, trep b) {
    if (t.m_bits & a) t.m_bits |= b;
  };
  check(BSPArrE,   BSPArrN);
  check(BCPArrE,   BPArrN);
  check(BSVArrE,   BSVArrN);
  check(BCVArrE,   BVArrN);
  check(BSDArrE,   BSDArrN);
  check(BCDArrE,   BDArrN);
  check(BSVecE,    BSVecN);
  check(BCVecE,    BVecN);
  check(BSDictE,   BSDictN);
  check(BCDictE,   BDictN);
  check(BSKeysetE, BSKeysetN);
  check(BCKeysetE, BKeysetN);
  return project_data(t, t.m_bits);
}

Type remove_uninit(Type t) {
  assert(t.subtypeOf(BCell));
  if (!t.couldBe(BUninit))  return t;
  if (isPredefined(t.m_bits & ~BUninit)) {
    t.m_bits &= ~BUninit;
    return t;
  }
  return TInitCell;
}

Type to_cell(Type t) {
  if (!t.subtypeOf(BCell)) return TInitCell;
  if (!(t.m_bits & BUninit)) return t;
  auto bits = (t.m_bits & ~BUninit) | BInitNull;
  assertx(isPredefined(bits));
  t.m_bits = bits;
  return t;
}

Type assert_emptiness(Type t) {
  if (t.subtypeOfAny(TTrue, TArrN, TVecN, TDictN, TKeysetN)) {
    return TBottom;
  }
  if (!could_have_magic_bool_conversion(t) && t.subtypeOrNull(BObj)) {
    return TNull;
  }

  auto remove = [&] (trep m, trep e) {
    if ((t.m_bits & (m | BNull)) == t.m_bits) {
      auto bits = t.m_bits & (e | BNull);
      if (t.hasData() && !mayHaveData(bits)) {
        t = Type { bits };
      } else {
        t = project_data(t, bits);
        t.m_bits = bits;
      }
      return true;
    }
    return false;
  };

  if (remove(BArr, BArrE) || remove(BVec, BVecE) ||
      remove(BDict, BDictE) || remove(BKeyset, BKeysetE)) {
    return t;
  }

  if (t.subtypeOf(BInt))     return ival(0);
  if (t.subtypeOf(BBool))    return TFalse;
  if (t.subtypeOf(BDbl))     return dval(0);
  if (t.subtypeOf(BSStr))    return sempty();

  auto add_nullish = [&] (Type in) {
    in.m_bits |= t.m_bits & BNull;
    return in;
  };

  if (t.subtypeOrNull(BInt))  return add_nullish(ival(0));
  if (t.subtypeOrNull(BBool)) return add_nullish(TFalse);
  if (t.subtypeOrNull(BDbl))  return add_nullish(dval(0));
  if (t.subtypeOrNull(BSStr)) return add_nullish(sempty());

  return t;
}

Type assert_nonemptiness(Type t) {
  t = remove_uninit(std::move(t));
  if (is_opt(t)) t = unopt(std::move(t));
  if (t.subtypeOf(BNull | BFalse | BArrE | BVecE | BDictE | BKeysetE)) {
    return TBottom;
  }
  if (auto const v = tv(t)) return tvToBool(*v) ? t : TBottom;
  if (t.subtypeOf(BBool)) return TTrue;

  auto remove = [&] (trep m, trep e) {
    if ((t.m_bits & (m | BNull)) == t.m_bits) {
      t.m_bits &= (e | BNull);
      return true;
    }
    return false;
  };

  if (remove(BArr, BArrN) || remove(BVec, BVecN) ||
      remove(BDict, BDictN) || remove(BKeyset, BKeysetN)) {
    return t;
  }

  return t;
}

//////////////////////////////////////////////////////////////////////

folly::Optional<ArrKey> maybe_class_func_key(const Type& keyTy, bool strict) {
  if (keyTy.subtypeOf(TNull)) return {};

  auto ret = ArrKey{};

  if (keyTy.subtypeOf(BOptCls | BOptFunc)) {
    ret.mayThrow = true;
    if (keyTy.subtypeOf(BCls | BFunc)) {
      ret.type = TStr;
      if (keyTy.strictSubtypeOf(TCls)) {
        ret.s = dcls_of(keyTy).cls.name();
      }
      return ret;
    }
    ret.type = TUncArrKey;
    return ret;
  } else if (keyTy.couldBe(BOptCls | BOptFunc)) {
    ret.mayThrow = true;
    if (strict) ret.type = keyTy.couldBe(BCStr) ? TArrKey : TUncArrKey;
    else        ret.type = TInitCell;
    return ret;
  }

  return {};
}

/*
 * For known strings that are strictly integers, we'll set both the known
 * integer and string keys, so generally the int case should be checked first
 * below.
 *
 * For keys that could be strings, we have to assume they could be
 * strictly-integer strings. After disection, the effective type we can assume
 * for the array key is in `type'. If the key might coerce to an integer, TInt
 * will be unioned into `type'. So, if `type' is TStr, its safe to assume it
 * will not coerce.
 *
 * `mayThrow' will be set if the key coercion could possibly throw.
 *
 * If the key might be strange (array or object), `type' will be unchanged so it
 * can be detected later on.
 */

ArrKey disect_array_key(const Type& keyTy) {
  auto ret = ArrKey{};

  if (auto const r = maybe_class_func_key(keyTy, false)) return *r;

  if (keyTy.subtypeOf(BOptInt)) {
    if (keyTy.subtypeOf(BInt)) {
      if (keyTy.strictSubtypeOf(TInt)) {
        ret.i = keyTy.m_data.ival;
        ret.type = ival(*ret.i);
        return ret;
      }
      ret.type = keyTy;
      return ret;
    }
    // The key could be an integer or a null, which means it might become the
    // empty string. Either way, its an uncounted value.
    ret.type = TUncArrKey;
    ret.mayThrow = RuntimeOption::EvalHackArrCompatNotices;
    return ret;
  }

  if (keyTy.subtypeOf(BOptStr)) {
    if (keyTy.subtypeOf(BStr)) {
      if (keyTy.strictSubtypeOf(TStr) && keyTy.m_dataTag == DataTag::Str) {
        ret.s = keyTy.m_data.sval;
      } else {
        ret.mayThrow = RuntimeOption::EvalHackArrCompatNotices;
      }
      ret.type = keyTy;
      return ret;
    }

    // Since the key is optional, we cannot include the value itself, as
    // explained below.
    ret.type = keyTy.subtypeOf(BOptSStr) ? TSStr : TStr;
    ret.mayThrow = RuntimeOption::EvalHackArrCompatNotices;
    return ret;
  }

  if (keyTy.subtypeOf(BOptArrKey)) {
    // The key is an integer, string, or null. The effective type is int or
    // string because null will become the empty string.
    ret.type = is_opt(keyTy) ? unopt(keyTy) : keyTy;
    return ret;
  }

  if (keyTy.strictSubtypeOf(TDbl)) {
    ret.i = double_to_int64(keyTy.m_data.dval);
    ret.type = ival(*ret.i);
    ret.mayThrow = RuntimeOption::EvalHackArrCompatNotices;
    return ret;
  }
  if (keyTy.subtypeOf(BNum)) {
    ret.type = TInt;
    ret.mayThrow = RuntimeOption::EvalHackArrCompatNotices;
    return ret;
  }
  if (keyTy.subtypeOf(BNull)) {
    ret.s = staticEmptyString();
    ret.type = sempty();
    ret.mayThrow = RuntimeOption::EvalHackArrCompatNotices;
    return ret;
  }
  if (keyTy.subtypeOf(BRes)) {
    ret.type = TInt;
    ret.mayThrow = RuntimeOption::EvalHackArrCompatNotices;
    return ret;
  }
  if (keyTy.subtypeOf(BTrue)) {
    ret.i = 1;
    ret.type = ival(1);
    ret.mayThrow = RuntimeOption::EvalHackArrCompatNotices;
    return ret;
  }
  if (keyTy.subtypeOf(BFalse)) {
    ret.i = 0;
    ret.type = ival(0);
    ret.mayThrow = RuntimeOption::EvalHackArrCompatNotices;
    return ret;
  }
  if (keyTy.subtypeOf(BBool)) {
    ret.type = TInt;
    ret.mayThrow = RuntimeOption::EvalHackArrCompatNotices;
    return ret;
  }
  if (keyTy.subtypeOf(BPrim)) {
    ret.type = TUncArrKey;
    ret.mayThrow = RuntimeOption::EvalHackArrCompatNotices;
    return ret;
  }

  // The key could be something strange like an array or an object. This can
  // raise warnings, so always assume it may throw. Keep the type as-is so that
  // we can detect this case at the point of the set.

  if (!keyTy.subtypeOf(BInitCell)) {
    ret.type = TInitCell;
    ret.mayThrow = true;
    return ret;
  }

  ret.type = keyTy;
  ret.mayThrow = true;
  return ret;
}

/*
 * Extract map[key] when map is known to have DataTag::ArrLikeVal.
 * Returns a std::pair where the first is the Type of the result, and
 * the second is a flag; if the flag is true, the element definitely
 * existed in the array.
 */
std::pair<Type,bool> arr_val_elem(const Type& aval, const ArrKey& key) {
  assert(aval.m_dataTag == DataTag::ArrLikeVal);
  auto ad = aval.m_data.aval;
  if (key.i) {
    if (auto const r = ad->rval(*key.i)) {
      return { from_cell(r.tv()), true };
    }
    return { TBottom, false };
  } else if (key.s) {
    if (auto const r = ad->rval(*key.s)) {
      return { from_cell(r.tv()), true };
    }
    return { TBottom, false };
  }

  auto const couldBeInt = key.type.couldBe(BInt);
  auto const couldBeStr = key.type.couldBe(BStr);
  auto ty = TBottom;
  IterateKV(ad, [&] (TypedValue k, TypedValue v) {
      if (isStringType(k.m_type) ? couldBeStr : couldBeInt) {
        ty |= from_cell(v);
        return TInitCell.subtypeOf(ty);
      }
      return false;
    });
  return { ty, false };
}

/*
 * Extract map[key] when map is known to have DataTag::ArrLikeMap
 * returns a std::pair where the first is the Type of the result, and
 * the second is a flag; if the flag is true, the element definitely
 * existed in the array.
 */
std::pair<Type,bool> arr_map_elem(const Type& map, const ArrKey& key) {
  assert(map.m_dataTag == DataTag::ArrLikeMap);
  if (auto const k = key.tv()) {
    auto r = map.m_data.map->map.find(*k);
    if (r != map.m_data.map->map.end()) return { r->second, true };
    // If the key could match an optional element, use its associated
    // type (but we cannot assert it definitely exists).
    if (key.type.couldBe(map.m_data.map->optKey)) {
      return { map.m_data.map->optVal, false };
    }
    return { TBottom, false };
  }
  auto couldBeInt = key.type.couldBe(BInt);
  auto couldBeStr = key.type.couldBe(BStr);
  auto ty = TBottom;
  for (auto const& kv : map.m_data.map->map) {
    if (isStringType(kv.first.m_type) ? couldBeStr : couldBeInt) {
      ty |= kv.second;
      if (TInitCell.subtypeOf(ty)) break;
    }
  }

  // Include any optional elements if there's a possible match
  if (key.type.couldBe(map.m_data.map->optKey)) ty |= map.m_data.map->optVal;
  return { ty, false };
}

/*
 * Extract pack[key] when pack is known to have DataTag::ArrLikePacked
 * returns a std::pair where the first is the Type of the result, and
 * the second is a flag; if the flag is true, the element definitely
 * existed in the array.
 */
std::pair<Type,bool> arr_packed_elem(const Type& pack, const ArrKey& key) {
  assert(pack.m_dataTag == DataTag::ArrLikePacked);
  if (key.i) {
    if (*key.i >= 0 && *key.i < pack.m_data.packed->elems.size()) {
      return { pack.m_data.packed->elems[*key.i], true };
    }
    return { TBottom, false };
  } else if (!key.type.couldBe(BInt)) {
    return { TBottom, false };
  }
  return { packed_values(*pack.m_data.packed), false };
}

/*
 * Extract pack[key] when pack is known to have DataTag::ArrLikePackedN
 */
std::pair<Type,bool> arr_packedn_elem(const Type& pack, const ArrKey& key) {
  assert(pack.m_dataTag == DataTag::ArrLikePackedN);
  if (key.s || !key.type.couldBe(BInt) || (key.i && *key.i < 0)) {
    return { TBottom, false };
  }
  return { pack.m_data.packedn->type, false };
}

/*
 * Apply the effects of pack[key] = val, when pack is known to have
 * DataTag::ArrLikePackedN.
 *
 * Returns true iff the key is known to be in range.
 */
bool arr_packedn_set(Type& pack,
                     const ArrKey& key,
                     const Type& val,
                     bool maybeEmpty) {
  assert(pack.m_dataTag == DataTag::ArrLikePackedN);
  assert(key.type.subtypeOf(BArrKey));

  auto const isPhpArray = pack.subtypeOrNull(BArr);
  auto const isVecArray = pack.subtypeOrNull(BVec);
  auto& ty = pack.m_data.packedn.mutate()->type;
  ty |= val;

  if (key.i) {
    // If the key is known to be in range - its still a packedn
    if (isPhpArray) {
      if (!*key.i) return true;
      if (!maybeEmpty && *key.i == 1) return true;
    } else if (!maybeEmpty && !*key.i) {
      return true;
    }
    pack.m_bits = (*key.i < 0)
      ? promote_varray(pack.m_bits)
      : maybe_promote_varray(pack.m_bits);
  } else {
    pack.m_bits = key.type.subtypeOf(BStr)
      ? promote_varray(pack.m_bits)
      : maybe_promote_varray(pack.m_bits);
  }

  if (!isVecArray) {
    pack = mapn_impl(
      pack.m_bits,
      union_of(TInt, key.type),
      std::move(ty),
      ProvTag::Top
    );
  }
  return false;
}

/*
 * Return the appropriate provenance tag to apply given the current type of the
 * array-like base and the source location.
 */
ProvTag arr_like_update_prov_tag(const Type& base, ProvTag loc) {
  assert(base.subtypeOrNull(BArrLike));
  if (!RuntimeOption::EvalArrayProvenance) return ProvTag::Top;
  if (!base.couldBe(kProvBits)) return ProvTag::Top;
  // If we don't know whether or not the array is empty, we also don't know if
  // we'll preserve the provenance tag that may be associated with the
  // ArrLikeN bit(s) of arr.
  //
  // OTOH, if we know the empty-ness of the array, it's safe to reason about
  // the provenance of the resulting array.
  if (base.couldBe(BArrLikeN) && base.couldBe(BArrLikeE)) return ProvTag::Top;

  auto const tag = base.getProvTag();

  if (tag == ProvTag::Top) return tag; // don't narrow Top
  if (tag == ProvTag::NoTag) return loc; // override unknown prov
  assertx(tag.valid());
  return tag;
}

/*
 * Apply the effects of map[key] = val, when map is known to have
 * DataTag::ArrLikeMap.
 *
 * Returns true iff we hit a specific element (this matters for
 * hack-array types, since they may throw for other cases).
 */
bool arr_map_set(Type& map,
                 const ArrKey& key,
                 const Type& val,
                 ProvTag src) {
  assert(map.m_dataTag == DataTag::ArrLikeMap);
  assert(key.type.subtypeOf(BArrKey));
  assert(!map.subtypeOf(BVArr));

  auto const tag = arr_like_update_prov_tag(map, src);

  auto mutated = map.m_data.map.mutate();
  mutated->provenance = tag;

  if (auto const k = key.tv()) {
    // The new key is known
    auto const it = mutated->map.find(*k);
    // It matches an existing key. Set its associated type to be the
    // new value type.
    if (it != mutated->map.end()) {
      mutated->map.update(it, val);
      return true;
    }

    if (mutated->hasOptElements()) {
      // The Map has optional elements. If the optional element
      // represents a single key, and that key is the same as the new
      // key, we can turn the optional element into a known key. An
      // optional element representing a single key means "this array
      // might end with this key, or might not". Since we're setting
      // this key, the new array definitely ends with that key.
      if (auto const optK = tv(mutated->optKey)) {
        if (tvSame(*optK, *k)) {
          mutated->map.emplace_back(*k, union_of(val, mutated->optVal));
          mutated->optKey = TBottom;
          mutated->optVal = TBottom;
          return true;
        }
      }
      // Otherwise just add the new key and value to the optional
      // elements. We've lost the complete key structure of the array.
      mutated->optKey |= key.type;
      mutated->optVal |= val;
    } else {
      // There's no optional elements and we know this key doesn't
      // exist in the Map. Add it as the next known key.
      mutated->map.emplace_back(*k, val);
    }
    return true;
  }

  // The new key isn't known. The key either matches an existing key,
  // or its a new one. For an existing key which can possibly match,
  // union in the new value type. Then add the new key and value to
  // the optional elements since it might not match. We don't have to
  // do this for Keysets because the value is guaranteed to be the
  // same as the key.
  if (!map.subtypeOf(BKeyset)) {
    for (auto it = mutated->map.begin(); it != mutated->map.end(); ++it) {
      if (key.type.couldBe(from_cell(it->first))) {
        mutated->map.update(it, union_of(it->second, val));
      }
    }
  }
  mutated->optKey |= key.type;
  mutated->optVal |= val;

  return true;
}

/*
 * Apply the effects of pack[key] = val, when pack is known to have
 * DataTag::ArrLikePacked.
 *
 * Returns true iff the key is known to be in range.
 */
bool arr_packed_set(Type& pack,
                    const ArrKey& key,
                    const Type& val,
                    ProvTag src) {
  assert(pack.m_dataTag == DataTag::ArrLikePacked);
  assert(key.type.subtypeOf(BArrKey));
  auto const tag = arr_like_update_prov_tag(pack, src);

  auto const isVecArray = pack.subtypeOrNull(BVec);
  if (key.i) {
    if (*key.i >= 0) {
      if (*key.i < pack.m_data.packed->elems.size()) {
        auto& current = pack.m_data.packed.mutate()->elems[*key.i];
        pack.m_data.packed.mutate()->provenance = tag;
        // if the element was a ref, its still a ref after assigning to it
        if (current.subtypeOf(BInitCell)) {
          current = val;
        }
        return true;
      }
      if (!isVecArray && *key.i == pack.m_data.packed->elems.size()) {
        pack.m_data.packed.mutate()->elems.push_back(val);
        pack.m_data.packed.mutate()->provenance = tag;
        return true;
      }
    }
    if (isVecArray) {
      pack = TBottom;
      return false;
    }
    pack.m_bits = promote_varray(pack.m_bits);
  } else {
    pack.m_bits = key.type.subtypeOf(BStr)
      ? promote_varray(pack.m_bits)
      : maybe_promote_varray(pack.m_bits);
  }

  if (!isVecArray) {
    if (auto const v = key.tv()) {
      MapElems elems;
      auto idx = int64_t{0};
      for (auto const& t : pack.m_data.packed->elems) {
        elems.emplace_back(make_tv<KindOfInt64>(idx++), t);
      }
      elems.emplace_back(*v, val);
      pack = map_impl(pack.m_bits, std::move(elems), TBottom, TBottom, tag);
      return true;
    }

    auto ty = union_of(packed_values(*pack.m_data.packed), val);
    pack = mapn_impl(pack.m_bits, union_of(TInt, key.type), std::move(ty), tag);
    return false;
  }

  auto ty = union_of(packed_values(*pack.m_data.packed), val);
  pack = packedn_impl(pack.m_bits, std::move(ty));

  return false;
}

bool arr_mapn_set(Type& map,
                  const ArrKey& key,
                  const Type& val) {
  assert(map.m_dataTag == DataTag::ArrLikeMapN);
  assert(key.type.subtypeOf(BArrKey));
  assert(!map.subtypeOf(BVArr));
  auto& mapn = *map.m_data.mapn.mutate();
  mapn.val |= val;
  mapn.key |= key.type;
  assert(map.checkInvariants());
  return true;
}

Type arr_map_newelem(Type& map, const Type& val, ProvTag src) {
  assert(map.m_dataTag == DataTag::ArrLikeMap);
  auto const tag = arr_like_update_prov_tag(map, src);

  // If the Map has optional elements, we can't known what the new key
  // is (but it won't modify known keys).
  if (map.m_data.map->hasOptElements()) {
    auto mutated = map.m_data.map.mutate();
    mutated->optKey |= TInt;
    mutated->optVal |= val;
    return TInt;
  }

  int64_t lastK = -1;
  for (auto const& kv : map.m_data.map->map) {
    if (kv.first.m_type == KindOfInt64 &&
        kv.first.m_data.num > lastK) {
      lastK = kv.first.m_data.num;
    }
  }

  if (lastK == std::numeric_limits<int64_t>::max()) {
    return TInt;
  }
  map.m_data.map.mutate()->map.emplace_back(make_tv<KindOfInt64>(lastK + 1),
                                            val);
  map.m_data.map.mutate()->provenance = tag;
  return ival(lastK + 1);
}

std::pair<Type, ThrowMode> array_like_elem(const Type& arr,
                                           const ArrKey& key,
                                           const Type& defaultTy) {
  const bool maybeEmpty = arr.couldBe(BArrLikeE);
  const bool mustBeStatic = arr.subtypeOrNull(BSArrLike);

  if (!arr.couldBe(BArrLikeN)) {
    assert(maybeEmpty);
    return {
      defaultTy,
      key.mayThrow ? ThrowMode::MaybeBadKey : ThrowMode::MissingElement
    };
  }
  auto pair = [&]() -> std::pair<Type, bool> {
    switch (arr.m_dataTag) {
    case DataTag::Str:
    case DataTag::Obj:
    case DataTag::Int:
    case DataTag::Dbl:
    case DataTag::Cls:
    case DataTag::Record:
      not_reached();

    case DataTag::None: {
      auto const val = [&]{
        if (arr.subtypeOrNull(BVArr) && !key.type.couldBe(BInt)) return TBottom;
        if (arr.subtypeOrNull(BKeyset)) {
          return mustBeStatic ? TUncArrKey : TArrKey;
        }
        return mustBeStatic ? TInitUnc : TInitCell;
      }();
      return { val, false };
    }

    case DataTag::ArrLikeVal:
      return arr_val_elem(arr, key);

    case DataTag::ArrLikePacked:
      return arr_packed_elem(arr, key);

    case DataTag::ArrLikePackedN:
      return arr_packedn_elem(arr, key);

    case DataTag::ArrLikeMap:
      return arr_map_elem(arr, key);

    case DataTag::ArrLikeMapN:
      return { arr.m_data.mapn->val, false };
    }
    not_reached();
  }();

  auto const isBottom = pair.first.subtypeOf(BBottom);
  std::pair<Type, ThrowMode> ret = {
    std::move(pair.first),
    key.mayThrow ? ThrowMode::MaybeBadKey :
    pair.second ? ThrowMode::None :
    isBottom ? ThrowMode::MissingElement :
    ThrowMode::MaybeMissingElement
  };
  if (!pair.second) ret.first |= defaultTy;

  if (!ret.first.subtypeOf(BInitCell)) {
    ret.first = TInitCell;
  }

  if (maybeEmpty) {
    ret.first |= defaultTy;
    if (ret.second == ThrowMode::None) {
      ret.second = ThrowMode::MaybeMissingElement;
    }
  }

  return ret;
}

std::pair<Type,ThrowMode>
array_elem(const Type& arr, const Type& undisectedKey, const Type& defaultTy) {
  assert(arr.subtypeOrNull(BArr));
  auto const key = disect_array_key(undisectedKey);
  return array_like_elem(arr, key, defaultTy);
}

/*
 * Note: for now we're merging counted arrays into whatever type it used to
 * have in the following set functions, and returning arr_*'s in some cases,
 * because it might become counted.
 *
 * To be able to assume it is actually counted it if used to be
 * static, we need to add code checking for keys that are one of the
 * "illegal offset type" of keys; in addition, even if we know *now*
 * that its counted, after further optimizations we might be able to
 * fully determine its contents, and replace it with a static array.
 *
 * A similar issue applies if you want to take out emptiness when a set occurs.
 * If the key could be an illegal key type, the array may remain empty.
 */

std::pair<Type,ThrowMode> array_like_set(Type arr,
                                         const ArrKey& key,
                                         const Type& valIn,
                                         ProvTag src) {
  const bool maybeEmpty = arr.couldBe(BArrLikeE);
  const bool isVector   = arr.couldBe(BVec);
  DEBUG_ONLY const bool isVArray   = arr.subtypeOrNull(BVArr);
  const bool validKey   = key.type.subtypeOf(isVector ? BInt : BArrKey);

  trep bits = combine_dv_arr_like_bits(arr.m_bits, BArrLikeN);
  if (validKey) bits &= ~BArrLikeE;

  auto const throwMode = validKey && !key.mayThrow ?
    ThrowMode::None : ThrowMode::BadOperation;
  auto const& val    = valIn;
  // We don't want to store types more general than TArrKey into specialized
  // array type keys. If the key was strange (array or object), it will be more
  // general than TArrKey (this is needed so we can set validKey above), so
  // force it to TArrKey.
  auto const& fixedKey = validKey
    ? key
    : []{ ArrKey key; key.type = TArrKey; key.mayThrow = true; return key; }();

  if (!arr.couldBe(BArrLikeN)) {
    auto const tag = arr_like_update_prov_tag(arr, src);
    assert(maybeEmpty);
    if (isVector) return { TBottom, ThrowMode::BadOperation };
    if (fixedKey.i) {
      if (!*fixedKey.i) {
        return { packed_impl(bits, { val }, tag),
                 throwMode };
      }
      bits = promote_varray(bits);
    } else {
      bits = fixedKey.type.subtypeOf(BStr)
        ? promote_varray(bits)
        : maybe_promote_varray(bits);
    }
    if (auto const k = fixedKey.tv()) {
      MapElems m;
      m.emplace_back(*k, val);
      return { map_impl(bits, std::move(m), TBottom, TBottom, tag), throwMode };
    }
    return { mapn_impl_from_map(bits, fixedKey.type, val, tag), throwMode };
  }

  auto emptyHelper = [&] (const Type& inKey,
                          const Type& inVal) -> std::pair<Type,ThrowMode> {
    bits = fixedKey.type.subtypeOf(BStr)
      ? promote_varray(bits)
      : maybe_promote_varray(bits);
    return {
      mapn_impl_from_map(
        bits,
        union_of(inKey, fixedKey.type),
        union_of(inVal, val),
        arr_like_update_prov_tag(arr, src)
      ),
      throwMode
    };
  };

  arr.m_bits = bits;

  switch (arr.m_dataTag) {
  case DataTag::Str:
  case DataTag::Obj:
  case DataTag::Int:
  case DataTag::Dbl:
  case DataTag::Cls:
  case DataTag::Record:
    not_reached();

  case DataTag::None:
    arr.m_bits = fixedKey.type.subtypeOf(BStr)
      ? promote_varray(arr.m_bits)
      : maybe_promote_varray(arr.m_bits);
    return { std::move(arr), ThrowMode::BadOperation };

  case DataTag::ArrLikeVal:
    if (maybeEmpty && !isVector) {
      auto kv = val_key_values(arr.m_data.aval);
      return emptyHelper(kv.first, kv.second);
    } else {
      if (auto d = toDArrLikePacked(arr.m_data.aval)) {
        return array_like_set(
          packed_impl(bits, std::move(d->elems), d->provenance),
          key, valIn, src
        );
      }
      assert(!isVector);
      assert(!isVArray);
      // We know its not packed, so this should always succeed
      auto d = toDArrLikeMap(arr.m_data.aval);
      return array_like_set(
        map_impl(
          bits,
          std::move(d->map),
          std::move(d->optKey),
          std::move(d->optVal),
          d->provenance
        ),
        key, valIn, src
      );
    }

  case DataTag::ArrLikePacked:
    // Setting element zero of a maybe empty, 1 element packed array
    // turns it into a 1 element packed array.
    if (maybeEmpty && !isVector &&
        (!fixedKey.i ||
         *fixedKey.i ||
         arr.m_data.packed->elems.size() != 1)) {
      return emptyHelper(TInt, packed_values(*arr.m_data.packed));
    } else {
      auto const inRange = arr_packed_set(arr, fixedKey, val, src);
      return { std::move(arr), inRange ? throwMode : ThrowMode::BadOperation };
    }

  case DataTag::ArrLikePackedN:
    if (maybeEmpty && !isVector) {
      return emptyHelper(TInt, arr.m_data.packedn->type);
    } else {
      auto const inRange = arr_packedn_set(arr, fixedKey, val, false);
      return { std::move(arr), inRange ? throwMode : ThrowMode::BadOperation };
    }

  case DataTag::ArrLikeMap:
    assert(!isVector);
    assert(!isVArray);
    if (maybeEmpty) {
      auto mkv = map_key_values(*arr.m_data.map);
      return emptyHelper(std::move(mkv.first), std::move(mkv.second));
    } else {
      auto const inRange = arr_map_set(arr, fixedKey, val, src);
      return { std::move(arr), inRange ? throwMode : ThrowMode::BadOperation };
    }

  case DataTag::ArrLikeMapN:
    assert(!isVector);
    assert(!isVArray);
    if (maybeEmpty) {
      return emptyHelper(arr.m_data.mapn->key, arr.m_data.mapn->val);
    } else {
      auto const inRange = arr_mapn_set(arr, fixedKey, val);
      return { std::move(arr), inRange ? throwMode : ThrowMode::BadOperation };
    }
  }

  not_reached();
}

std::pair<Type, ThrowMode> array_set(Type arr,
                                     const Type& undisectedKey,
                                     const Type& val,
                                     ProvTag src) {
  assert(arr.subtypeOf(BArr));

  auto const key = disect_array_key(undisectedKey);
  assert(key.type != TBottom);
  return array_like_set(std::move(arr), key, val, src);
}

std::pair<Type,Type> array_like_newelem(Type arr,
                                        const Type& val,
                                        ProvTag src) {

  if (arr.couldBe(BKeyset)) {
    auto const key = disect_strict_key(val);
    if (key.type == TBottom) return { TBottom, TInitCell };
    return { array_like_set(std::move(arr), key, key.type, src).first, val };
  }

  const bool maybeEmpty = arr.couldBe(BArrLikeE);
  const bool isVector = arr.couldBe(BVec);
  const bool isVArray = arr.subtypeOrNull(BVArr);

  trep bits = combine_dv_arr_like_bits(arr.m_bits, BArrLikeN);
  bits &= ~BArrLikeE;

  if (!arr.couldBe(BArrLikeN)) {
    assert(maybeEmpty);
    return { packed_impl(bits, { val }, arr_like_update_prov_tag(arr, src)),
             ival(0) };
  }


  auto emptyHelper = [&] (const Type& inKey,
                          const Type& inVal) -> std::pair<Type,Type> {
    if (isVector || isVArray) {
      assert(inKey.subtypeOf(BInt));
      return { packedn_impl(bits, union_of(inVal, val)), TInt };
    }

    return {
      mapn_impl_from_map(
        bits,
        union_of(inKey, TInt),
        union_of(inVal, val),
        arr_like_update_prov_tag(arr, src)
      ),
      TInt
    };
  };

  switch (arr.m_dataTag) {
  case DataTag::Str:
  case DataTag::Obj:
  case DataTag::Int:
  case DataTag::Dbl:
  case DataTag::Cls:
  case DataTag::Record:
    not_reached();

  case DataTag::None:
    arr.m_bits = bits;
    return { std::move(arr), TInt };

  case DataTag::ArrLikeVal:
    if (maybeEmpty) {
      auto kv = val_key_values(arr.m_data.aval);
      return emptyHelper(kv.first, kv.second);
    } else {
      if (auto d = toDArrLikePacked(arr.m_data.aval)) {
        return array_like_newelem(
          packed_impl(bits, std::move(d->elems), d->provenance), val, src);
      }
      assert(!isVector);
      // We know its not packed, so this should always succeed.
      auto d = toDArrLikeMap(arr.m_data.aval);
      return array_like_newelem(
        map_impl(
          bits,
          std::move(d->map),
          std::move(d->optKey),
          std::move(d->optVal),
          d->provenance
        ),
        val, src
      );
    }

  case DataTag::ArrLikePacked:
    if (maybeEmpty) {
      return emptyHelper(TInt, packed_values(*arr.m_data.packed));
    } else {
      arr.m_bits = bits;
      auto len = arr.m_data.packed->elems.size();
      arr.m_data.packed.mutate()->elems.push_back(val);
      arr.m_data.packed.mutate()->provenance =
        arr_like_update_prov_tag(arr, src);
      return { std::move(arr), ival(len) };
    }

  case DataTag::ArrLikePackedN:
    if (maybeEmpty) {
      return emptyHelper(TInt, arr.m_data.packedn->type);
    } else {
      arr.m_bits = bits;
      auto packedn = arr.m_data.packedn.mutate();
      packedn->type |= val;
      return { std::move(arr), TInt };
    }

  case DataTag::ArrLikeMap:
    assert(!isVector);
    assert(!isVArray);
    if (maybeEmpty) {
      auto mkv = map_key_values(*arr.m_data.map);
      return emptyHelper(mkv.first, mkv.second);
    } else {
      arr.m_bits = bits;
      auto const idx = arr_map_newelem(arr, val, src);
      return { std::move(arr), idx };
    }

  case DataTag::ArrLikeMapN:
    assert(!isVector);
    assert(!isVArray);
    if (maybeEmpty) {
      return emptyHelper(arr.m_data.mapn->key, arr.m_data.mapn->val);
    }
    return {
      mapn_impl_from_map(
        bits,
        union_of(arr.m_data.mapn->key, TInt),
        union_of(arr.m_data.mapn->val, val),
        arr_like_update_prov_tag(arr, src)
      ),
      TInt
    };
  }

  not_reached();
}

std::pair<Type,Type> array_newelem(Type arr, const Type& val, ProvTag src) {
  assert(arr.subtypeOf(BArr));

  return array_like_newelem(std::move(arr), val, src);
}

IterTypes iter_types(const Type& iterable) {
  // Only array types and objects can be iterated. Everything else raises a
  // warning and jumps out of the loop.
  if (!iterable.couldBeAny(TArr, TVec, TDict, TKeyset, TObj, TClsMeth)) {
    return { TBottom, TBottom, IterTypes::Count::Empty, true, true };
  }

  // Optional types are okay here because a null will not set any locals (but it
  // might throw).
  if (!iterable.subtypeOfAny(TOptArr, TOptVec, TOptDict, TOptKeyset)) {
    return {
      TInitCell,
      TInitCell,
      IterTypes::Count::Any,
      true,
      iterable.couldBe(BObj)
    };
  }

  auto const mayThrow = is_opt(iterable);

  if (iterable.subtypeOfAny(TOptArrE, TOptVecE, TOptDictE, TOptKeysetE)) {
    return { TBottom, TBottom, IterTypes::Count::Empty, mayThrow, false };
  }

  // If we get a null, it will be as if we have any empty array, so consider
  // that possibly "empty".
  auto const maybeEmpty =
    mayThrow ||
    !iterable.subtypeOfAny(TOptArrN, TOptVecN, TOptDictN, TOptKeysetN);

   auto const count = [&](folly::Optional<size_t> size){
    if (size) {
      assert(*size > 0);
      if (*size == 1) {
        return maybeEmpty
          ? IterTypes::Count::ZeroOrOne
          : IterTypes::Count::Single;
      }
    }
    return maybeEmpty ? IterTypes::Count::Any : IterTypes::Count::NonEmpty;
  };

  if (!is_specialized_array_like(iterable)) {
    auto kv = [&]() -> std::pair<Type, Type> {
      if (iterable.subtypeOrNull(BSVec))    return { TInt, TInitUnc };
      if (iterable.subtypeOrNull(BSDict))   return { TUncArrKey, TInitUnc };
      if (iterable.subtypeOrNull(BSKeyset)) return { TUncArrKey, TUncArrKey };
      if (iterable.subtypeOrNull(BSVArr))   return { TInt, TInitUnc };
      if (iterable.subtypeOrNull(BSArr))    return { TUncArrKey, TInitUnc };

      if (iterable.subtypeOrNull(BVec))     return { TInt, TInitCell };
      if (iterable.subtypeOrNull(BDict))    return { TArrKey, TInitCell };
      if (iterable.subtypeOrNull(BKeyset))  return { TArrKey, TArrKey };
      if (iterable.subtypeOrNull(BVArr))    return { TInt, TInitCell };
      if (iterable.subtypeOrNull(BArr))     return { TArrKey, TInitCell };

      always_assert(false);
    }();

    return {
      std::move(kv.first),
      std::move(kv.second),
      count(folly::none),
      mayThrow,
      false
    };
  }

  switch (iterable.m_dataTag) {
  case DataTag::None:
  case DataTag::Str:
  case DataTag::Obj:
  case DataTag::Int:
  case DataTag::Dbl:
  case DataTag::Cls:
  case DataTag::Record:
    always_assert(0);
  case DataTag::ArrLikeVal: {
    auto kv = val_key_values(iterable.m_data.aval);
    return {
      std::move(kv.first),
      std::move(kv.second),
      count(iterable.m_data.aval->size()),
      mayThrow,
      false
    };
  }
  case DataTag::ArrLikePacked:
    return {
      TInt,
      packed_values(*iterable.m_data.packed),
      count(iterable.m_data.packed->elems.size()),
      mayThrow,
      false
    };
  case DataTag::ArrLikePackedN:
    return {
      TInt,
      iterable.m_data.packedn->type,
      count(folly::none),
      mayThrow,
      false
    };
  case DataTag::ArrLikeMap: {
    auto kv = map_key_values(*iterable.m_data.map);
    return {
      std::move(kv.first),
      std::move(kv.second),
      iterable.m_data.map->hasOptElements()
        ? count(folly::none)
        : count(iterable.m_data.map->map.size()),
      mayThrow,
      false
    };
  }
  case DataTag::ArrLikeMapN:
    return {
      iterable.m_data.mapn->key,
      iterable.m_data.mapn->val,
      count(folly::none),
      mayThrow,
      false
    };
  }

  not_reached();
}

bool could_contain_objects(const Type& t) {
  if (t.couldBe(BObj)) return true;

  auto const couldBeArrWithDestructors =
    t.m_bits & (BCArrN | BCVecN | BCDictN);

  if (!couldBeArrWithDestructors) return false;

  switch (t.m_dataTag) {
  case DataTag::None:
  case DataTag::Str:
  case DataTag::Obj:
  case DataTag::Int:
  case DataTag::Dbl:
  case DataTag::Cls:
  case DataTag::Record:
    return true;
  case DataTag::ArrLikeVal: return false;
  case DataTag::ArrLikePacked:
    for (auto const& e : t.m_data.packed->elems) {
      if (could_contain_objects(e)) return true;
    }
    return false;
  case DataTag::ArrLikePackedN:
    return could_contain_objects(t.m_data.packedn->type);
  case DataTag::ArrLikeMap:
    for (auto const& kv : t.m_data.map->map) {
      if (could_contain_objects(kv.second)) return true;
    }
    if (could_contain_objects(t.m_data.map->optVal)) return true;
    return false;
  case DataTag::ArrLikeMapN:
    return could_contain_objects(t.m_data.mapn->val);
  }

  not_reached();
}

bool could_copy_on_write(const Type& t) {
  return t.m_bits & (BCStr | BCArrN | BCVecN | BCDictN | BCKeysetN);
}

bool is_type_might_raise(const Type& testTy, const Type& valTy) {
  auto const hackarr = RO::EvalHackArrDVArrs;
  auto const BHackArr = BVec | BDict | BKeyset;

  // Explanation for the array-like type test behaviors:
  //
  //  1. If arrprov is on, we may log if an array-like with provenance fails or
  //     passes a type test, because those arrays may have to be marked.
  //
  //  2. The ...IsArrayNotices flag is used to migrate is_array calls to
  //     is_any_array calls, which differ for Hack arrays, so we must log.
  //
  //  3. The ...IsVecDictNotices flag tracks those type tests that would change
  //     behavior with the Hack array migration - i.e. those tests which have
  //     varrays in is_vec, darrays in is_dict, or vice versa.

  auto const mayLogProv = RO::EvalArrayProvenance && valTy.couldBe(kProvBits);

  if (is_opt(testTy)) return is_type_might_raise(unopt(testTy), valTy);
  if (testTy == TStrLike) {
    return valTy.couldBe(BFunc | BCls);
  } else if (testTy == TArr || testTy == TPArrLike) {
    return mayLogProv ||
           (RO::EvalIsVecNotices && !hackarr && valTy.couldBe(BClsMeth)) ||
           (RO::EvalHackArrCompatIsArrayNotices && valTy.couldBe(BHackArr));
  } else if (testTy == TVArr || testTy == TVArrLike) {
    return mayLogProv ||
           (RO::EvalIsVecNotices && valTy.couldBe(BClsMeth)) ||
           (RO::EvalHackArrCompatIsVecDictNotices && valTy.couldBe(BVec));
  } else if (testTy == TDArr) {
    return mayLogProv ||
           (RO::EvalHackArrCompatIsVecDictNotices && valTy.couldBe(BDict));
  } else if (testTy == TVec || testTy == TVecLike) {
    return mayLogProv ||
           (RO::EvalIsVecNotices && hackarr && valTy.couldBe(BClsMeth)) ||
           (RO::EvalHackArrCompatIsVecDictNotices && valTy.couldBe(BVec));
  } else if (testTy == TDict) {
    return mayLogProv ||
           (RO::EvalHackArrCompatIsVecDictNotices && valTy.couldBe(BDArr));
  }
  return false;
}

bool is_type_might_raise(IsTypeOp testOp, const Type& valTy) {
  switch (testOp) {
    case IsTypeOp::ArrLike:
      return RuntimeOption::EvalIsVecNotices && valTy.couldBe(BClsMeth);
    case IsTypeOp::Scalar:
      return false;
    default:
      return is_type_might_raise(type_of_istype(testOp), valTy);
  }
}

bool inner_types_might_raise(const Type& t1, const Type& t2) {
  assertx(t1.subtypeOf(BArrLike));
  assertx(t2.subtypeOf(BArrLike));

  // If either is an empty array, there are no inner elements to warn about.
  if (!t1.couldBe(BArrLikeN) || !t2.couldBe(BArrLikeN)) return false;

  auto const checkOne = [&] (const Type& t, folly::Optional<size_t>& sz) {
    switch (t.m_dataTag) {
      case DataTag::None:
        return true;

      case DataTag::Str:
      case DataTag::Obj:
      case DataTag::Int:
      case DataTag::Dbl:
      case DataTag::Cls:
      case DataTag::Record:
        not_reached();

      case DataTag::ArrLikeVal:
        sz = t.m_data.aval->size();
        return true;
      case DataTag::ArrLikePacked:
        sz = t.m_data.packed->elems.size();
        return true;
      case DataTag::ArrLikePackedN:
        return t.m_data.packedn->type.couldBe(BArrLike | BObj | BClsMeth);
      case DataTag::ArrLikeMap:
        if (!t.m_data.map->hasOptElements()) sz = t.m_data.map->map.size();
        return true;
      case DataTag::ArrLikeMapN:
        return t.m_data.mapn->val.couldBe(BArrLike | BObj | BClsMeth);
    }
    not_reached();
  };

  folly::Optional<size_t> sz1;
  if (!checkOne(t1, sz1)) return false;
  folly::Optional<size_t> sz2;
  if (!checkOne(t2, sz2)) return false;

  // if the arrays have different sizes, we don't even check their contents
  if (sz1 && sz2 && *sz1 != *sz2) return false;
  size_t numToCheck = 1;
  if (sz1 && *sz1 > numToCheck) numToCheck = *sz1;
  if (sz2 && *sz2 > numToCheck) numToCheck = *sz2;

  union ArrPos {
    ArrPos() : pos{} {}
    size_t pos;
    MapElems::iterator it;
  } p1, p2;

  folly::Optional<Type> vals1;
  folly::Optional<Type> vals2;

  for (size_t i = 0; i < numToCheck; i++) {
    auto const nextType = [&] (const Type& t,
                               ArrPos& p,
                               folly::Optional<Type>& vals) {
      switch (t.m_dataTag) {
        case DataTag::None:
          return TInitCell;

        case DataTag::Str:
        case DataTag::Obj:
        case DataTag::Int:
        case DataTag::Dbl:
        case DataTag::Cls:
        case DataTag::Record:
          not_reached();

        case DataTag::ArrLikeVal:
          if (!i) {
            p.pos = t.m_data.aval->iter_begin();
          } else {
            p.pos = t.m_data.aval->iter_advance(p.pos);
          }
          return from_cell(t.m_data.aval->nvGetVal(p.pos));
        case DataTag::ArrLikePacked:
          return t.m_data.packed->elems[i];
        case DataTag::ArrLikePackedN:
          return t.m_data.packedn->type;
        case DataTag::ArrLikeMap:
          if (t.m_data.map->hasOptElements()) {
            if (!vals) vals = map_key_values(*t.m_data.map).second;
            return *vals;
          }

          if (!i) {
            p.it = t.m_data.map->map.begin();
          } else {
           ++p.it;
          }
          return p.it->second;
        case DataTag::ArrLikeMapN:
          return t.m_data.mapn->val;
      }
      not_reached();
    };
    if (compare_might_raise(nextType(t1, p1, vals1), nextType(t2, p2, vals2))) {
      return true;
    }
  }
  return false;
}

bool compare_might_raise(const Type& t1, const Type& t2) {
  if (!RuntimeOption::EvalHackArrCompatNotices &&
      !RuntimeOption::EvalHackArrCompatDVCmpNotices &&
      !RuntimeOption::EvalEmitClsMethPointers) {
    return false;
  }

  auto checkOne = [&] (const trep bits) -> folly::Optional<bool> {
    if (t1.subtypeOf(bits) && t2.subtypeOf(bits)) {
      return inner_types_might_raise(t1, t2);
    }
    if (t1.couldBe(bits) && t2.couldBe(BArrLike)) return true;
    if (t2.couldBe(bits) && t1.couldBe(BArrLike)) return true;
    return folly::none;
  };

  if (RuntimeOption::EvalHackArrCompatDVCmpNotices) {
    if (auto const f = checkOne(BPArr)) return *f;
    if (auto const f = checkOne(BDArr)) return *f;
    if (auto const f = checkOne(BVArr)) return *f;
  } else {
    if (auto const f = checkOne(BArr)) return *f;
  }
  if (auto const f = checkOne(BDict)) return *f;
  if (auto const f = checkOne(BVec)) return *f;
  if (auto const f = checkOne(BKeyset)) return *f;

  if (RuntimeOption::EvalHackArrCompatDVCmpNotices ||
      RuntimeOption::EvalEmitClsMethPointers) {
    if (!RuntimeOption::EvalHackArrDVArrs) {
      if (t1.couldBe(TClsMeth) && t2.couldBe(TArr))     return true;
      if (t1.couldBe(TArr)     && t2.couldBe(TClsMeth)) return true;
    } else if (RuntimeOption::EvalEmitClsMethPointers) {
      if (t1.couldBe(TClsMeth) && t2.couldBe(TVec))     return true;
      if (t1.couldBe(TVec)     && t2.couldBe(TClsMeth)) return true;
    }
  }

  return t1.couldBe(BObj) && t2.couldBe(BObj);
}

//////////////////////////////////////////////////////////////////////

ArrKey disect_vec_key(const Type& keyTy) {
  auto ret = ArrKey{};

  if (!keyTy.couldBe(BInt)) {
    ret.type = TBottom;
    ret.mayThrow = true;
    return ret;
  }

  // If the key is null, we'll throw, so we can assume its not for the effective
  // type (and mark it as potentially throwing). We check for this explicitly
  // here rather than falling through so we can take advantage of something like
  // ?Int=123.
  if (keyTy.subtypeOf(BOptInt)) {
    if (keyTy.m_dataTag == DataTag::Int) {
      ret.i = keyTy.m_data.ival;
      ret.type = ival(*ret.i);
    } else {
      ret.type = TInt;
    }
    ret.mayThrow = !keyTy.subtypeOf(BInt);
    return ret;
  }

  // Something else. Same reasoning as above. We can assume its an TInt because
  // it will throw otherwise.
  ret.type = TInt;
  ret.mayThrow = true;
  return ret;
}

std::pair<Type, ThrowMode>
vec_elem(const Type& vec, const Type& undisectedKey, const Type& defaultTy) {
  auto const key = disect_vec_key(undisectedKey);
  if (key.type == TBottom) return {TBottom, ThrowMode::BadOperation};
  return array_like_elem(vec, key, defaultTy);
}

std::pair<Type, ThrowMode>
vec_set(Type vec, const Type& undisectedKey, const Type& val, ProvTag src) {
  if (!val.couldBe(BInitCell)) return {TBottom, ThrowMode::BadOperation};

  auto const key = disect_vec_key(undisectedKey);
  if (key.type == TBottom) return {TBottom, ThrowMode::BadOperation};

  return array_like_set(std::move(vec), key, val, src);
}

std::pair<Type,Type> vec_newelem(Type vec, const Type& val, ProvTag src) {
  return array_like_newelem(std::move(vec),
                            val.subtypeOf(BInitCell) ? val : TInitCell,
                            src);
}

//////////////////////////////////////////////////////////////////////

ArrKey disect_strict_key(const Type& keyTy) {
  auto ret = ArrKey{};

  if (auto const r = maybe_class_func_key(keyTy, true)) return *r;

  if (!keyTy.couldBe(BArrKey)) {
    ret.type = TBottom;
    ret.mayThrow = true;
    return ret;
  }

  // If the key is null, we'll throw, so we can assume its not for the effective
  // type (but mark it as potentially throwing).
  if (keyTy.subtypeOf(BOptArrKey)) {
    if (keyTy.m_dataTag == DataTag::Int) {
      ret.i = keyTy.m_data.ival;
    } else if (keyTy.m_dataTag == DataTag::Str) {
      ret.s = keyTy.m_data.sval;
    }
    ret.type = is_opt(keyTy) ? unopt(keyTy) : keyTy;
    ret.mayThrow = !keyTy.subtypeOf(BArrKey);
    return ret;
  }

  // Something else. Same reasoning as above. We can assume its an TArrKey
  // because it will throw otherwise.
  ret.type = TArrKey;
  ret.mayThrow = true;
  return ret;
}

std::pair<Type, ThrowMode>
dict_elem(const Type& dict, const Type& undisectedKey, const Type& defaultTy) {
  auto const key = disect_strict_key(undisectedKey);
  if (key.type == TBottom) return {TBottom, ThrowMode::BadOperation};
  return array_like_elem(dict, key, defaultTy);
}

std::pair<Type, ThrowMode>
dict_set(Type dict, const Type& undisectedKey, const Type& val, ProvTag src) {
  if (!val.couldBe(BInitCell)) return {TBottom, ThrowMode::BadOperation};

  auto const key = disect_strict_key(undisectedKey);
  if (key.type == TBottom) return {TBottom, ThrowMode::BadOperation};

  return array_like_set(std::move(dict), key, val, src);
}

std::pair<Type,Type> dict_newelem(Type dict, const Type& val, ProvTag src) {
  return array_like_newelem(std::move(dict),
                            val.subtypeOf(BInitCell) ? val : TInitCell,
                            src);
}

//////////////////////////////////////////////////////////////////////

std::pair<Type, ThrowMode>
keyset_elem(const Type& keyset,
            const Type& undisectedKey,
            const Type& defaultTy) {
  auto const key = disect_strict_key(undisectedKey);
  if (key.type == TBottom) return {TBottom, ThrowMode::BadOperation};
  return array_like_elem(keyset, key, defaultTy);
}

std::pair<Type, ThrowMode>
keyset_set(Type /*keyset*/, const Type&, const Type&) {
  // The set operation on keysets is not allowed.
  return {TBottom, ThrowMode::BadOperation};
}

std::pair<Type,Type> keyset_newelem(Type keyset, const Type& val) {
  return array_like_newelem(std::move(keyset), val, ProvTag::Top);
}

//////////////////////////////////////////////////////////////////////

RepoAuthType make_repo_type_arr(ArrayTypeTable::Builder& arrTable,
                                const Type& t) {
  auto const emptiness = (TArrE.couldBe(t) || TVecE.couldBe(t) ||
                          TDictE.couldBe(t) || TKeysetE.couldBe(t))
    ? RepoAuthType::Array::Empty::Maybe
    : RepoAuthType::Array::Empty::No;

  auto const arr = [&]() -> const RepoAuthType::Array* {
    switch (t.m_dataTag) {
    case DataTag::None:
    case DataTag::Str:
    case DataTag::Obj:
    case DataTag::Int:
    case DataTag::Dbl:
    case DataTag::Cls:
    case DataTag::Record:
    case DataTag::ArrLikeVal:
    case DataTag::ArrLikeMap:
    case DataTag::ArrLikeMapN:
      return nullptr;
    case DataTag::ArrLikePackedN:
      return arrTable.packedn(
        emptiness,
        make_repo_type(arrTable, t.m_data.packedn->type)
      );
    case DataTag::ArrLikePacked:
      {
        std::vector<RepoAuthType> repoTypes;
        std::transform(
          begin(t.m_data.packed->elems), end(t.m_data.packed->elems),
          std::back_inserter(repoTypes),
          [&] (const Type& t2) { return make_repo_type(arrTable, t2); }
        );
        return arrTable.packed(emptiness, repoTypes);
      }
      return nullptr;
    }
    not_reached();
  }();

  auto const tag = [&]() -> RepoAuthType::Tag {
    if (t.subtypeOf(BSVArr))    return RepoAuthType::Tag::SVArr;
    if (t.subtypeOf(BVArr))     return RepoAuthType::Tag::VArr;
    if (t.subtypeOf(BOptSVArr)) return RepoAuthType::Tag::OptSVArr;
    if (t.subtypeOf(BOptVArr))  return RepoAuthType::Tag::OptVArr;
    if (t.subtypeOf(BSDArr))    return RepoAuthType::Tag::SDArr;
    if (t.subtypeOf(BDArr))     return RepoAuthType::Tag::DArr;
    if (t.subtypeOf(BOptSDArr)) return RepoAuthType::Tag::OptSDArr;
    if (t.subtypeOf(BOptDArr))  return RepoAuthType::Tag::OptDArr;
    if (t.subtypeOf(BSArr))     return RepoAuthType::Tag::SArr;
    if (t.subtypeOf(BArr))      return RepoAuthType::Tag::Arr;
    if (t.subtypeOf(BOptSArr))  return RepoAuthType::Tag::OptSArr;
    if (t.subtypeOf(BOptArr))   return RepoAuthType::Tag::OptArr;

    if (t.subtypeOf(BSVec))     return RepoAuthType::Tag::SVec;
    if (t.subtypeOf(BVec))      return RepoAuthType::Tag::Vec;
    if (t.subtypeOf(BOptSVec))  return RepoAuthType::Tag::OptSVec;
    if (t.subtypeOf(BOptVec))   return RepoAuthType::Tag::OptVec;

    if (t.subtypeOf(BSDict))    return RepoAuthType::Tag::SDict;
    if (t.subtypeOf(BDict))     return RepoAuthType::Tag::Dict;
    if (t.subtypeOf(BOptSDict)) return RepoAuthType::Tag::OptSDict;
    if (t.subtypeOf(BOptDict))  return RepoAuthType::Tag::OptDict;

    if (t.subtypeOf(BSKeyset))    return RepoAuthType::Tag::SKeyset;
    if (t.subtypeOf(BKeyset))     return RepoAuthType::Tag::Keyset;
    if (t.subtypeOf(BOptSKeyset)) return RepoAuthType::Tag::OptSKeyset;
    if (t.subtypeOf(BOptKeyset))  return RepoAuthType::Tag::OptKeyset;

    not_reached();
  }();

  return RepoAuthType { tag, arr };
}

RepoAuthType make_repo_type(ArrayTypeTable::Builder& arrTable, const Type& t) {
  assert(!t.subtypeOf(BBottom));
  using T = RepoAuthType::Tag;

  if (is_specialized_obj(t) && t.subtypeOf(TOptObj)) {
    auto const dobj = dobj_of(t);
    auto const tag =
      is_opt(t)
        ? (dobj.type == DObj::Exact ? T::OptExactObj : T::OptSubObj)
        : (dobj.type == DObj::Exact ? T::ExactObj    : T::SubObj);
    return RepoAuthType { tag, dobj.cls.name() };
  }

  if (is_specialized_cls(t) && t.subtypeOf(TOptCls)) {
    auto const dcls = dcls_of(t);
    auto const tag =
      is_opt(t)
        ? (dcls.type == DCls::Exact ? T::OptExactCls : T::OptSubCls)
        : (dcls.type == DCls::Exact ? T::ExactCls : T::SubCls);
    return RepoAuthType { tag, dcls.cls.name() };
  }

  if (is_specialized_record(t) && t.subtypeOf(TOptRecord)) {
    auto const drec = drec_of(t);
    auto const tag =
      is_opt(t)
        ? (drec.type == DRecord::Exact ? T::OptExactRecord : T::OptSubRecord)
        : (drec.type == DRecord::Exact ? T::ExactRecord : T::SubRecord);
    return RepoAuthType { tag, drec.rec.name() };
  }

  if ((is_specialized_array(t) && t.subtypeOf(TOptArr)) ||
      (is_specialized_vec(t) && t.subtypeOf(TOptVec))) {
    return make_repo_type_arr(arrTable, t);
  }

#define X(x) if (t.subtypeOf(B##x)) return RepoAuthType{T::x};
  X(Uninit)
  X(InitNull)
  X(Null)
  X(Int)
  X(OptInt)
  X(Dbl)
  X(OptDbl)
  X(Res)
  X(OptRes)
  X(Bool)
  X(OptBool)
  X(SStr)
  X(OptSStr)
  X(Str)
  X(OptStr)
  X(SVArr)
  X(OptSVArr)
  X(VArr)
  X(OptVArr)
  X(SDArr)
  X(OptSDArr)
  X(DArr)
  X(OptDArr)
  X(SArr)
  X(OptSArr)
  X(Arr)
  X(OptArr)
  X(SVec)
  X(OptSVec)
  X(Vec)
  X(OptVec)
  X(SDict)
  X(OptSDict)
  X(Dict)
  X(OptDict)
  X(SKeyset)
  X(OptSKeyset)
  X(Keyset)
  X(OptKeyset)
  X(Obj)
  X(OptObj)
  X(Func)
  X(OptFunc)
  X(Cls)
  X(OptCls)
  X(ClsMeth)
  X(OptClsMeth)
  X(UncArrKey)
  X(ArrKey)
  X(OptUncArrKey)
  X(OptArrKey)
  X(UncStrLike)
  X(StrLike)
  X(OptUncStrLike)
  X(OptStrLike)
  X(VArrLike)
  X(VecLike)
  X(OptVArrLike)
  X(OptVecLike)
  X(PArrLike)
  X(OptPArrLike)
  X(InitUnc)
  X(Unc)
  X(InitCell)
  X(Cell)
#undef X
  not_reached();
}

//////////////////////////////////////////////////////////////////////

Type adjust_type_for_prop(const Index& index,
                          const php::Class& propCls,
                          const TypeConstraint* tc,
                          const Type& ty) {
  auto ret = loosen_likeness(ty);
  // If the type-hint might not be enforced, we must be conservative.
  if (!tc || index.prop_tc_maybe_unenforced(propCls, *tc)) return ret;
  auto const ctx = Context { nullptr, nullptr, &propCls };
  // Otherwise lookup what we know about the constraint.
  auto tcType = unctx(
    loosen_dvarrayness(remove_uninit(index.lookup_constraint(ctx, *tc, ret)))
  );
  // For the same reason as property/return type enforcement, we have to be
  // pessimistic with interfaces to ensure that types in the index always
  // shrink.
  if (is_specialized_obj(tcType) &&
      dobj_of(tcType).cls.couldBeInterfaceOrTrait()) {
    tcType = is_opt(tcType) ? TOptObj : TObj;
  }
  // The adjusted type is the intersection of the constraint and the type (which
  // might not exist).
  return intersection_of(tcType, std::move(ret));
}

//////////////////////////////////////////////////////////////////////

}}
