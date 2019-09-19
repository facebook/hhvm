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
  case BRef:     case BFunc:
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

    /* Under some circumstances, we store an array in
     * B[Opt][S]{Vec,Dict}E to effectively track their
     * provenance tag */
  case BCVecE:     case BSVecE:     case BVecE:
  case BOptCVecE:  case BOptSVecE:  case BOptVecE:
  case BCDictE:    case BSDictE:    case BDictE:
  case BOptCDictE: case BOptSDictE: case BOptDictE:
    return true;

  case BBottom:
  case BUninit:
  case BInitNull:
  case BFalse:
  case BTrue:
  case BCStr:
  case BSArrE:
  case BCArrE:
  case BSKeysetE:
  case BCKeysetE:
  case BSPArrE:
  case BCPArrE:
  case BPArrE:
  case BSVArrE:
  case BCVArrE:
  case BVArrE:
  case BSDArrE:
  case BCDArrE:
  case BDArrE:
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
  case BOptSVArrE:
  case BOptCVArrE:
  case BOptVArrE:
  case BOptSDArrE:
  case BOptCDArrE:
  case BOptDArrE:
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
  case BInitGen:
  case BGen:
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
  case BUncStrLike:
  case BStrLike:
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

  case BRef:
    return false;

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
  case BOptUncStrLike:
  case BOptStrLike:
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
  case BInitGen:
  case BCell:
  case BGen:
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
  if (p1 && p2) {
    return *p1 == *p2;
  } else {
    return !p2;
  }
}

/*
 * Compute the union of two provenance tags
 */
ProvTag unionProvTag(ProvTag p1, ProvTag p2) {
  if (p1 && p2) {
    return *p1 == *p2 ? p1 : folly::none;
  } else {
    return folly::none;
  }
}

/*
 * Compute the intersection of two provenance tags
 *
 * XXX: if both p1 and p2 are specific tags and they differ,
 * the result should be bottom, but we widen to 'top' since it prevents
 * provenance data from producing bottoms and is still correct since
 * intersections are allowed to be wider than either input type.
 */
ProvTag intersectProvTag(ProvTag p1, ProvTag p2) {
  if (p1 && p2) {
    return (*p1 == *p2) ? p1 : folly::none;
  } else {
    return p1 ? p1 : p2;
  }
}

/*
 * Determine if p1 and p2 could possibly be in a subtype relationship (i.e.
 * their intersection is non-empty). For provenance tags, this only isn't the
 * case if both tags are known and they do not match
 */
bool couldBeProvTag(ProvTag p1, ProvTag p2) {
  return !p1 || !p2 || (*p1 == *p2);
}

ProvTag provTagFromArray(SArray arr) {
  if (!RuntimeOption::EvalArrayProvenance) return folly::none;
  return arrprov::getTag(arr);
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

  return DArrLikePacked { std::move(elems), provTagFromArray(ar) };
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
  return DArrLikeMap { std::move(map), provTagFromArray(ar) };
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
  if (a.map.size() != b.map.size()) return false;
  if (!subtypeProvTag(a.provenance, b.provenance)) return false;
  auto aIt = begin(a.map);
  auto bIt = begin(b.map);
  for (; aIt != end(a.map); ++aIt, ++bIt) {
    if (!cellSame(aIt->first, bIt->first)) return false;
    if (!aIt->second.subtypeOfImpl<contextSensitive>(bIt->second)) return false;
  }
  return true;
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
  if (a.map.size() != b.map.size()) return false;
  if (!couldBeProvTag(a.provenance, b.provenance)) return false;
  auto aIt = begin(a.map);
  auto bIt = begin(b.map);
  for (; aIt != end(a.map); ++aIt, ++bIt) {
    if (!cellSame(aIt->first, bIt->first)) return false;
    if (!aIt->second.couldBe(bIt->second)) return false;
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
  if (!visit(arrprov::getTag(arr1), arrprov::getTag(arr2))) return false;
  bool ret = true;
  IterateKV(
    arr1,
    [&](Cell k, TypedValue tv1) {
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
  if (!arr1->equal(arr2, true)) return false;
  return visitDifferingProvTagsInSimilarArrays(arr1, arr2, subtypeProvTag);
}

bool couldBeArrLike(SArray arr1, SArray arr2) {
  if (arr1 == arr2) return true;
  if (arr1->size() != arr2->size()) return false;
  if (!arr1->equal(arr2, true)) return false;
  return visitDifferingProvTagsInSimilarArrays(arr1, arr2, couldBeProvTag);
}

//////////////////////////////////////////////////////////////////////

std::pair<Type,Type> val_key_values(SArray a) {
  auto ret = std::make_pair(TBottom, TBottom);
  for (ArrayIter iter(a); iter; ++iter) {
    ret.first |= from_cell(*iter.first().asTypedValue());
    ret.second |= from_cell(*iter.secondRval());
  }
  return ret;
}

std::pair<Type,Type> map_key_values(const DArrLikeMap& a) {
  auto ret = std::make_pair(TBottom, TBottom);
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
    if (RuntimeOption::EvalArrayProvenance &&
        a.provenance != arrprov::getTag(b)) return false;
    auto const p = toDArrLikePacked(b);
    if (!p) return false;
    for (auto i = 0; i < a.elems.size(); i++) {
      if (!a.elems[i].equivImpl<contextSensitive>(p->elems[i])) return false;
    }
    return true;
  }

  bool operator()(const DArrLikeMap& a, SArray b) const {
    if (a.map.size() != b->size()) return false;
    if (RuntimeOption::EvalArrayProvenance &&
        a.provenance != arrprov::getTag(b)) return false;
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
    if (RuntimeOption::EvalArrayProvenance &&
        !couldBeProvTag(a.provenance, arrprov::getTag(b))) return false;
    auto const p = toDArrLikePacked(b);
    return p && couldBePacked(a, *p);
  }

  bool operator()(const DArrLikeMap& a, SArray b) const {
    if (a.map.size() != b->size()) return false;
    if (RuntimeOption::EvalArrayProvenance &&
        !couldBeProvTag(a.provenance, arrprov::getTag(b))) return false;
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
      [&] (Cell k, TypedValue v) {
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

  template <typename F>
  Type intersect_map(MapElems map, F next, ProvTag tag) const {
    for (auto it = map.begin(); it != map.end(); it++) {
      auto other = next();
      if (it->first.m_type == KindOfInt64 ?
          !other.first.couldBe(BInt) : !other.first.couldBe(BStr)) {
        return TBottom;
      }
      auto val = intersection_of(it->second, other.second);
      if (val == TBottom) return TBottom;
      map.update(it, std::move(val));
    }
    return map_impl(bits, std::move(map), tag);
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
    return mapn_impl(bits, k, v, folly::none);
  }
  Type operator()(const DArrLikeMapN& a, const DArrLikeMap& b) const {
    return intersect_map(b.map,
                         [&] { return std::make_pair(a.key, a.val); },
                         b.provenance);
  }

  Type operator()(const DArrLikeMap& a, const DArrLikeMap& b) const {
    if (a.map.size() != b.map.size()) return TBottom;
    auto it = b.map.begin();
    return intersect_map(a.map, [&] {
      auto ret = std::make_pair(from_cell(it->first), it->second);
      ++it;
      return ret;
    }, intersectProvTag(a.provenance, b.provenance));
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
    auto to_map = [&] {
      auto mkva = map_key_values(a);
      auto mkvb = map_key_values(b);

      return mapn_impl_from_map(
        bits,
        union_of(std::move(mkva.first), std::move(mkvb.first)),
        union_of(std::move(mkva.second), std::move(mkvb.second)),
        unionProvTag(a.provenance, b.provenance)
      );
    };

    /*
     * With the current meaning of structs, if the keys are different, we can't
     * do anything better than going to a map type.  The reason for this is
     * that our struct types currently are implying the presence of all the
     * keys in the struct (it might be worth adding some more types for struct
     * subtyping to handle this better.)
     */
    if (a.map.size() != b.map.size()) return to_map();

    auto retStruct = MapElems{};
    auto aIt = begin(a.map);
    auto bIt = begin(b.map);
    for (; aIt != end(a.map); ++aIt, ++bIt) {
      if (!cellSame(aIt->first, bIt->first)) return to_map();
      retStruct.emplace_back(aIt->first, union_of(aIt->second, bIt->second));
    }
    return map_impl(
      bits,
      std::move(retStruct),
      unionProvTag(a.provenance, b.provenance)
    );
  }

  Type operator()(SArray a, SArray b) const {
    assert(a != b); // Should've been handled earlier in union_of.
    if (a->empty() && b->empty()) {
      auto const aTag = arrprov::getTag(a);
      auto const bTag = arrprov::getTag(b);
      assert(aTag != bTag ||
             a->kind() != b->kind());
      if (a->kind() != b->kind() ||
          a->dvArray() != b->dvArray()) return Type { bits };
      if (!a->isVecArray() && !a->isDict()) return Type { bits };

      auto ad = a->copy();
      if (auto const tag = unionProvTag(aTag, bTag)) {
        arrprov::setTag(ad, *tag);
      }
      ArrayData::GetScalarArray(&ad);

      auto r = a->isVecArray()
        ? vec_empty()
        : dict_empty();
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
      folly::none
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
      folly::none
    );
  }

  Type operator()(const DArrLikePackedN& a, const DArrLikeMap& b) const {
    auto mkv = map_key_values(b);
    return mapn_impl_from_map(
      bits,
      union_of(TInt, std::move(mkv.first)),
      union_of(a.type, std::move(mkv.second)),
      folly::none
    );
  }

  Type operator()(const DArrLikePackedN& a, const DArrLikeMapN& b) const {
    return mapn_impl(
      bits,
      union_of(TInt, b.key),
      union_of(a.type, b.val),
      folly::none
    );
  }

  Type operator()(const DArrLikeMap& a, const DArrLikeMapN& b) const {
    auto mkv = map_key_values(a);
    return mapn_impl_from_map(
      bits,
      union_of(std::move(mkv.first), b.key),
      union_of(std::move(mkv.second), b.val),
      folly::none
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
    if (a.map.size() != b->size()) return false;
    auto const m = toDArrLikeMap(b);
    return m && subtypeMap<contextSensitive>(a, *m);
  }

  bool operator()(SArray a, const DArrLikeMap& b) const {
    if (a->size() != b.map.size()) return false;
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
    return true;
  }

  bool operator()(SArray a, const DArrLikeMapN& b) const {
    assert(!a->empty()); // if a is empty, we should have projected b to top
    bool bad = false;
    IterateKV(
      a,
      [&] (Cell k, TypedValue v) {
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
folly::Optional<Cell> fromTypeVec(const std::vector<Type> &elems,
                                  ProvTag tag) {
  AInit ai(elems.size());
  for (auto const& t : elems) {
    auto const v = tv(t);
    if (!v) return folly::none;
    ai.append(tvAsCVarRef(&*v));
  }
  auto var = ai.toVariant();
  if (tag) {
    assertx(RuntimeOption::EvalArrayProvenance);
    arrprov::setTag(var.asArrRef().get(), *tag);
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
const Variant& keyHelper(const Cell& v) {
  return tvAsCVarRef(&v);
}
template <typename AInit>
void add(AInit& ai, const Variant& key, const Variant& value) {
  ai.setValidKey(key, value);
}
void add(KeysetInit& ai, const Variant& key, const Variant& value) {
  assert(cellSame(*key.asTypedValue(), *value.asTypedValue()));
  ai.add(key);
}

template<typename AInit, bool force_static, typename Key>
folly::Optional<Cell> fromTypeMap(const ArrayLikeMap<Key> &elems,
                                  ProvTag tag) {
  auto val = eval_cell_value([&] () -> Cell {
    AInit ai(elems.size());
    for (auto const& elm : elems) {
      auto const v = tv(elm.second);
      if (!v) return make_tv<KindOfUninit>();
      add(ai, keyHelper(elm.first), tvAsCVarRef(&*v));
    }
    auto var = ai.toVariant();
    if (tag) {
      assertx(RuntimeOption::EvalArrayProvenance);
      assertx(arrprov::arrayWantsTag(var.asCArrRef().get()));
      arrprov::setTag(var.asArrRef().get(), *tag);
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
        auto const ty = unctxHelper(it->second, c);
        if (c) {
          mutated->map.update(it, ty);
        }
      }
    }
    break;
  }
  case DataTag::ArrLikeMapN: {
    auto const mapn = t.m_data.mapn.get();
    auto ty = unctxHelper(mapn->val, changed);
    if (changed) {
      t.m_data.mapn.mutate()->val = ty;
    }
    break;
  }
  case DataTag::None:
  case DataTag::Int:
  case DataTag::Dbl:
  case DataTag::Str:
  case DataTag::RefInner:
  case DataTag::ArrLikeVal:
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
  case DataTag::RefInner:       return f();
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
  case DataTag::RefInner:       return f();
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
  case DataTag::RefInner:
    return m_data.inner->equivImpl<contextSensitive>(*o.m_data.inner);
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
    return true;
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
  case DataTag::ArrLikeVal:
    return subtypeArrLike(m_data.aval, o.m_data.aval);
  case DataTag::Str:
  case DataTag::Int:
  case DataTag::Dbl:
  case DataTag::None:
    // Context sensitivity should not matter here.
    return equivData<contextSensitive>(o);
  case DataTag::RefInner:
    return m_data.inner->subtypeOfImpl<contextSensitive>(*o.m_data.inner);
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
  case DataTag::RefInner:
    return m_data.inner->couldBe(*o.m_data.inner);
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
        case DataTag::RefInner:
          return 0;
        case DataTag::Str:
          return (uintptr_t)m_data.sval;
        case DataTag::Int:
          return m_data.ival;
        case DataTag::Dbl:
          return m_data.dval;
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
  case DataTag::Cls:         return restrict_to(BCls);
  case DataTag::Str:         return restrict_to(BStr);
  case DataTag::RefInner:    return restrict_to(BRef);
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
    } else{
      return restrict_to(BArrN | BDictN | BVecN | BKeysetN);
    }
    not_reached();
  }
  case DataTag::ArrLikePackedN:
    return restrict_to(BVecN | BDictN | BKeysetN | BArrN);
  case DataTag::ArrLikeMapN:
    return restrict_to(BDictN | BKeysetN | BArrN);
  default:
    not_reached();
  }
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
  // just an optimization; if the intersection contains one of these,
  // we're done because they don't support data.
  if (isect & (BNull | BBool)) return true;

  auto const this_projected = project_data(*this, isect);
  auto const o_projected = project_data(o, isect);
  // hasData is actually cheaper than mayHaveData, so do those checks first
  if (!this_projected.hasData() || !o_projected.hasData()) return true;
  // This looks like it could be problematic - eg BCell does not
  // support data, but lots of its subtypes do. It seems like what we
  // need here is !subtypeMayHaveData(isect) (a function we don't
  // actually have). We know however that both inputs have data, so
  // all we rely on here is that if A supports data, and B is a
  // subtype of A that does not (eg TOptArr and TOptArrE), then no
  // subtype of B can support data.
  if (!mayHaveData(isect)) return true;
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
  DEBUG_ONLY auto const valBits = isPHPArray ?
    BInitGen : isKeyset ? BArrKey : BInitCell;

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
  case DataTag::RefInner:
    assert(!m_data.inner->couldBe(BRef));
    break;
  case DataTag::Cls:    break;
  case DataTag::Obj:    break;
  case DataTag::ArrLikeVal:
    assert(m_data.aval->isStatic());
    assert(!m_data.aval->empty() || isVector || isDict);
    assert(m_bits & (BArr | BVec | BDict | BKeyset));
    if (m_data.aval->empty()) {
      assert(!couldBe(BVecN));
      assert(!couldBe(BDictN));
    }
    // If we have a static array, we'd better be sure of the type.
    assert(!isPHPArray || isVArray || isDArray || isNotDVArray);
    assert(!isPHPArray || m_data.aval->isPHPArray());
    assert(!isVArray || m_data.aval->isVArray());
    assert(!isDArray || m_data.aval->isDArray());
    assert(!isNotDVArray || m_data.aval->isNotDVArray());
    assert(!isVector || m_data.aval->isVecArray());
    assert(!isKeyset || m_data.aval->isKeyset());
    assert(!isDict || m_data.aval->isDict());
    assertx(!RuntimeOption::EvalHackArrDVArrs || m_data.aval->isNotDVArray());
    assertx(!m_data.aval->hasProvenanceData() ||
            RuntimeOption::EvalArrayProvenance);
    assertx(!m_data.aval->hasProvenanceData() || isVector || isDict);
    assertx(!m_data.aval->hasProvenanceData() ||
            arrprov::getTag(m_data.aval)->filename());
    break;
  case DataTag::ArrLikePacked: {
    assert(!m_data.packed->elems.empty());
    assertx(!m_data.packed->provenance || RuntimeOption::EvalArrayProvenance);
    assertx(!m_data.packed->provenance || couldBe(BVec) || couldBe(BDict));
    assertx(!m_data.packed->provenance ||
            m_data.packed->provenance->filename());
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
    assertx(!m_data.map->provenance || RuntimeOption::EvalArrayProvenance);
    assertx(!m_data.map->provenance || couldBe(BDict));
    assertx(!m_data.map->provenance || m_data.map->provenance->filename());
    DEBUG_ONLY auto idx = size_t{0};
    DEBUG_ONLY auto packed = true;
    for (DEBUG_ONLY auto const& kv : m_data.map->map) {
      assert(cellIsPlausible(kv.first));
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
    assert(!packed);
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
  assert(val->isPHPArray());
  assertx(!RuntimeOption::EvalHackArrDVArrs || val->isNotDVArray());
  if (val->empty()) {
    if (val->isDArray()) return aempty_darray();
    if (val->isVArray()) return aempty_varray();
    return aempty();
  }
  auto r = [&]{
    if (val->isDArray()) return Type { BSDArrN };
    if (val->isVArray()) return Type { BSVArrN };
    return Type { BSPArrN };
  }();
  r.m_data.aval = val;
  r.m_dataTag   = DataTag::ArrLikeVal;
  return r;
}

Type aempty()         { return Type { BSPArrE }; }
Type aempty_varray()  {
  assertx(!RuntimeOption::EvalHackArrDVArrs);
  return Type { BSVArrE };
}
Type aempty_darray()  {
  assertx(!RuntimeOption::EvalHackArrDVArrs);
  return Type { BSDArrE };
}
Type sempty()         { return sval(staticEmptyString()); }
Type some_aempty()    { return Type { BPArrE }; }
Type some_aempty_darray() {
  assertx(!RuntimeOption::EvalHackArrDVArrs);
  return Type { BDArrE };
}

Type vec_val(SArray val) {
  assert(val->isStatic());
  assert(val->isVecArray());
  auto const bits = val->empty() ? BSVecE : BSVecN;
  auto r = Type { bits };
  r.m_data.aval = val;
  r.m_dataTag   = DataTag::ArrLikeVal;
  return r;
}

Type vec_empty() {
  auto r = Type { BSVecE };
  r.m_data.aval = staticEmptyVecArray();
  r.m_dataTag   = DataTag::ArrLikeVal;
  return r;
}

Type some_vec_empty() {
  auto r = Type { BVecE };
  r.m_data.aval = staticEmptyVecArray();
  r.m_dataTag   = DataTag::ArrLikeVal;
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
  auto const tag = (bits & (BVec | BDict)) ? prov : folly::none;
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
  assert(val->isDict());
  auto const bits = val->empty() ? BSDictE : BSDictN;
  auto r = Type { bits };
  r.m_data.aval = val;
  r.m_dataTag   = DataTag::ArrLikeVal;
  return r;
}

Type dict_empty() {
  auto r = Type { BSDictE };
  r.m_data.aval = staticEmptyDictArray();
  r.m_dataTag   = DataTag::ArrLikeVal;
  return r;
}

Type some_dict_empty() {
  auto r = Type { BDictE };
  r.m_data.aval = staticEmptyDictArray();
  r.m_dataTag   = DataTag::ArrLikeVal;
  return r;
}

Type dict_map(MapElems m, ProvTag tag) {
  return map_impl(BDictN, std::move(m), tag);
}

Type dict_n(Type k, Type v, ProvTag tag) {
  return mapn_impl(BDictN, std::move(k), std::move(v), tag);
}

Type sdict_n(Type k, Type v, ProvTag tag) {
  return mapn_impl(BSDictN, std::move(k), std::move(v), tag);
}

Type keyset_val(SArray val) {
  assert(val->isStatic());
  assert(val->isKeyset());
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
  return mapn_impl(BKeysetN, std::move(kv), std::move(v), folly::none);
}

Type skeyset_n(Type kv) {
  assert(kv.subtypeOf(BUncArrKey));
  auto v = kv;
  return mapn_impl(BSKeysetN, std::move(kv), std::move(v), folly::none);
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

Type ref_to(Type t) {
  assert(t.subtypeOf(BInitCell));
  auto r = Type{BRef};
  construct_inner(r.m_data.inner, std::move(t));
  r.m_dataTag = DataTag::RefInner;
  return r;
}

bool is_ref_with_inner(const Type& t) {
  return t.m_dataTag == DataTag::RefInner;
}

bool is_specialized_array_like(const Type& t) {
  switch (t.m_dataTag) {
  case DataTag::None:
  case DataTag::Str:
  case DataTag::Obj:
  case DataTag::Int:
  case DataTag::Dbl:
  case DataTag::Cls:
  case DataTag::RefInner:
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
    if (auto p = toDArrLikePacked(a.m_data.aval)) {
      return packed_impl(bits, std::move(p->elems), p->provenance);
    }
    auto d = toDArrLikeMap(a.m_data.aval);
    return map_impl(bits, std::move(d->map), d->provenance);
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
  return packed_impl(BPArrN, std::move(elems), folly::none);
}

Type arr_packed_varray(std::vector<Type> elems) {
  assertx(!RuntimeOption::EvalHackArrDVArrs);
  return packed_impl(BVArrN, std::move(elems), folly::none);
}

Type sarr_packed(std::vector<Type> elems) {
  return packed_impl(BSPArrN, std::move(elems), folly::none);
}

Type arr_packedn(Type t) {
  return packedn_impl(BPArrN, std::move(t));
}

Type sarr_packedn(Type t) {
  return packedn_impl(BSPArrN, std::move(t));
}

Type map_impl(trep bits, MapElems m, ProvTag prov) {
  assert(!m.empty());

  // A Map cannot be packed, so if it is, return a Packed instead.
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
    std::vector<Type> elems;
    for (auto& p : m) elems.emplace_back(std::move(p.second));
    return packed_impl(bits, std::move(elems), prov);
  }

  auto r = Type { bits };
  auto const tag = (bits & (BVec | BDict)) ? prov : folly::none;
  construct_inner(r.m_data.map, std::move(m), tag);
  r.m_dataTag = DataTag::ArrLikeMap;
  return r;
}

Type arr_map(MapElems m) {
  return map_impl(BPArrN, std::move(m), folly::none);
}

Type arr_map_darray(MapElems m) {
  assertx(!RuntimeOption::EvalHackArrDVArrs);
  return map_impl(BDArrN, std::move(m), folly::none);
}

Type sarr_map(MapElems m) {
  return map_impl(BSPArrN, std::move(m), folly::none);
}

Type mapn_impl(trep bits, Type k, Type v, ProvTag tag) {
  assert(k.subtypeOf(BArrKey));

  // A MapN cannot have a constant key (because that can actually make it be a
  // subtype of Map sometimes), so if it does, make it a Map instead.
  if (auto val = tv(k)) {
    MapElems m;
    m.emplace_back(*val, std::move(v));
    return map_impl(bits, std::move(m), tag);
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
  return mapn_impl(BPArrN, std::move(k), std::move(v), folly::none);
}

Type sarr_mapn(Type k, Type v) {
  return mapn_impl(BSPArrN, std::move(k), std::move(v), folly::none);
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
  assertx(t.subtypeOf(BInitGen));
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
      return t.m_data.map->map.size();

    case DataTag::ArrLikePacked:
      return t.m_data.packed->elems.size();

    case DataTag::None:
    case DataTag::Int:
    case DataTag::Dbl:
    case DataTag::Str:
    case DataTag::RefInner:
    case DataTag::ArrLikePackedN:
    case DataTag::ArrLikeMapN:
    case DataTag::Obj:
    case DataTag::Cls:
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
  auto checkKey = [&] (const Cell& key) {
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
                [&] (Cell k, TypedValue) {
                  return checkKey(k);
                });
      break;

    case DataTag::ArrLikeMap:
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
    case DataTag::RefInner:
    case DataTag::ArrLikePackedN:
    case DataTag::ArrLikeMapN:
    case DataTag::Obj:
    case DataTag::Cls:
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
                [&] (Cell k, TypedValue) {
                  assert(isStringType(k.m_type));
                  strs.push_back(k.m_data.pstr);
                });
      break;

    case DataTag::ArrLikeMap:
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
    case DataTag::RefInner:
    case DataTag::ArrLikePackedN:
    case DataTag::ArrLikeMapN:
    case DataTag::Obj:
    case DataTag::Cls:
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
    return H::template make<KindOfPersistentArray>(staticEmptyArray());
  case BVArrE:
  case BSVArrE:
    assertx(!RuntimeOption::EvalHackArrDVArrs);
    return H::template make<KindOfPersistentArray>(staticEmptyVArray());
  case BDArrE:
  case BSDArrE:
    assertx(!RuntimeOption::EvalHackArrDVArrs);
    return H::template make<KindOfPersistentArray>(staticEmptyDArray());
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
        return H::template make<KindOfPersistentArray>(
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
      if (t.subtypeOf(BDictN)) {
        return H::template fromMap<DictInit>(t.m_data.map->map,
                                             t.m_data.map->provenance);
      } else if (t.subtypeOf(BKeysetN)) {
        return H::template fromMap<KeysetInit>(t.m_data.map->map, folly::none);
      } else if (t.subtypeOf(BPArrN)) {
        return H::template fromMap<MixedArrayInit>(t.m_data.map->map,
                                                   folly::none);
      } else if (t.subtypeOf(BDArrN)) {
        assertx(!RuntimeOption::EvalHackArrDVArrs);
        return H::template fromMap<DArrayInit>(t.m_data.map->map, folly::none);
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
                                                     folly::none);
      } else if (t.subtypeOf(BPArrN)) {
        return H::template fromVec<PackedArrayInit>(t.m_data.packed->elems,
                                                    folly::none);
      } else if (t.subtypeOf(BVArrN)) {
        assertx(!RuntimeOption::EvalHackArrDVArrs);
        return H::template fromVec<VArrayInit>(t.m_data.packed->elems,
                                               folly::none);
      } else if (t.subtypeOf(BDArrN)) {
        assertx(!RuntimeOption::EvalHackArrDVArrs);
        return H::template fromVec<DArrayInit>(t.m_data.packed->elems,
                                               folly::none);
      }
      break;
    case DataTag::RefInner:
    case DataTag::ArrLikePackedN:
    case DataTag::ArrLikeMapN:
    case DataTag::Obj:
    case DataTag::Cls:
    case DataTag::None:
      break;
    }
  }

  return R{};
}

folly::Optional<Cell> tv(const Type& t) {
  return tvImpl<folly::Optional<Cell>, true>(t);
}

folly::Optional<Cell> tvNonStatic(const Type& t) {
  return tvImpl<folly::Optional<Cell>, false>(t);
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
                  BSVecE | BSDictE;
      return t;
    case DataTag::Str:
      t.m_bits &= BSStr;
      return t;
    case DataTag::ArrLikeMap:
    case DataTag::ArrLikePacked:
      return from_cell(*tv(t));
    case DataTag::RefInner:
    case DataTag::ArrLikePackedN:
    case DataTag::ArrLikeMapN:
    case DataTag::Obj:
    case DataTag::Cls:
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
    case DataTag::RefInner:
    case DataTag::ArrLikePackedN:
    case DataTag::ArrLikeMapN:
    case DataTag::Obj:
    case DataTag::Cls:
      return folly::none;
    case DataTag::ArrLikeVal:
      return t.m_data.aval->size();
    case DataTag::ArrLikeMap:
      return t.m_data.map->map.size();
    case DataTag::ArrLikePacked:
      return t.m_data.packed->elems.size();
  }
  not_reached();
}

folly::Optional<std::pair<Type,Type>>
array_get_by_index(const Type& t, ssize_t index) {
  if (!t.subtypeOf(BArrLike)) return folly::none;
  switch (t.m_dataTag) {
    case DataTag::None:
    case DataTag::Int:
    case DataTag::Dbl:
    case DataTag::Str:
    case DataTag::RefInner:
    case DataTag::ArrLikePackedN:
    case DataTag::ArrLikeMapN:
    case DataTag::Obj:
    case DataTag::Cls:
      return folly::none;

    case DataTag::ArrLikeVal: {
      ssize_t pos{};
      if (index < 0) {
        index = -index - 1;
        if (index >= t.m_data.aval->size()) {
          return folly::none;
        }
        pos = t.m_data.aval->iter_end();
        while (index--) pos = t.m_data.aval->iter_advance(pos);
      } else {
        if (index >= t.m_data.aval->size()) {
          return folly::none;
        }
        pos = t.m_data.aval->iter_begin();
        while (index--) pos = t.m_data.aval->iter_advance(pos);
      }
      auto k = eval_cell([&]{ return t.m_data.aval->atPos(pos); });
      auto v = eval_cell([&]{ return t.m_data.aval->nvGetKey(pos); });
      if (k && v) return std::make_pair(*k, *v);
      return folly::none;
    }
    case DataTag::ArrLikeMap: {
      if ((index < 0 ? -index - 1 : index) >= t.m_data.map->map.size()) {
        return folly::none;
      }
      auto it = index < 0 ? t.m_data.map->map.end() : t.m_data.map->map.begin();
      std::advance(it, index);
      return std::make_pair(from_cell(it->first), it->second);
    }
    case DataTag::ArrLikePacked:
      if (index < 0) index += t.m_data.packed->elems.size();
      if (index < 0 || index >= t.m_data.packed->elems.size()) {
        return folly::none;
      }
      return std::make_pair(ival(index), t.m_data.packed->elems[index]);
  }
  not_reached();
}

Type type_of_istype(IsTypeOp op) {
  switch (op) {
  case IsTypeOp::Null:   return TNull;
  case IsTypeOp::Bool:   return TBool;
  case IsTypeOp::Int:    return TInt;
  case IsTypeOp::Dbl:    return TDbl;
  case IsTypeOp::Str:    return TStrLike;
  case IsTypeOp::Res:    return TRes;
  case IsTypeOp::Arr:    return TArr;
  case IsTypeOp::Vec:    return TVec;
  case IsTypeOp::Dict:   return TDict;
  case IsTypeOp::Keyset: return TKeyset;
  case IsTypeOp::Obj:    return TObj;
  case IsTypeOp::VArray:
    assertx(!RuntimeOption::EvalHackArrDVArrs);
    return TVArr;
  case IsTypeOp::DArray:
    assertx(!RuntimeOption::EvalHackArrDVArrs);
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

folly::Optional<Type> type_of_type_structure(SArray ts) {
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
        auto t = type_of_type_structure(tsElems->getValue(i).getArrayData());
        if (!t) return folly::none;
        v.emplace_back(std::move(t.value()));
      }
      if (v.empty()) return folly::none;
      auto const arrT = arr_packed_varray(v);
      return is_nullable ? union_of(std::move(arrT), TNull) : arrT;
    }
    case TypeStructure::Kind::T_shape: {
      // Taking a very conservative approach to shapes where we dont do any
      // conversions if the shape contains unknown or optional fields
      if (does_ts_shape_allow_unknown_fields(ts)) return folly::none;
      auto map = MapElems{};
      auto const fields = get_ts_fields(ts);
      for (auto i = 0; i < fields->size(); i++) {
        auto const key = fields->getKey(i).getStringData();
        auto const wrapper = fields->getValue(i).getArrayData();
        // Optional fields are hard to represent as a type
        if (is_optional_ts_shape_field(wrapper)) return folly::none;
        auto t = type_of_type_structure(get_ts_value(wrapper));
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

Type from_cell(Cell cell) {
  assert(cellIsPlausible(cell));

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
    always_assert(cell.m_data.parr->isVecArray());
    return vec_val(cell.m_data.parr);

  case KindOfPersistentDict:
  case KindOfDict:
    always_assert(cell.m_data.parr->isStatic());
    always_assert(cell.m_data.parr->isDict());
    return dict_val(cell.m_data.parr);

  case KindOfPersistentKeyset:
  case KindOfKeyset:
    always_assert(cell.m_data.parr->isStatic());
    always_assert(cell.m_data.parr->isKeyset());
    return keyset_val(cell.m_data.parr);

  case KindOfRecord: // TODO(arnabde)
    not_implemented();

  case KindOfPersistentArray:
  case KindOfArray:
    always_assert(cell.m_data.parr->isStatic());
    always_assert(cell.m_data.parr->isPHPArray());
    return aval(cell.m_data.parr);

  case KindOfRef:
  case KindOfObject:
  case KindOfResource:
  case KindOfFunc:
  case KindOfClass:
  case KindOfClsMeth:
    break;
  }
  always_assert(0 && "reference counted/class/func/clsmeth type in from_cell");
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
  case KindOfPersistentArray:
  case KindOfArray:    return TArr;
  case KindOfRef:      return TRef;
  case KindOfObject:   return TObj;
  case KindOfResource: return TRes;
  case KindOfFunc:     return TFunc;
  case KindOfClass:    return TCls;
  case KindOfClsMeth:  return TClsMeth;
  }
  always_assert(0 && "dt in from_DataType didn't satisfy preconditions");
}

Type from_hni_constraint(SString s) {
  if (!s) return TGen;

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
  if (!strcasecmp(p, "HH\\mixed"))    return TInitGen;
  if (!strcasecmp(p, "HH\\nonnull"))  return TInitGen;

  // It might be an object, or we might want to support type aliases in HNI at
  // some point.  For now just be conservative.
  return TGen;
}

Type intersection_of(Type a, Type b) {
  auto const isect = a.m_bits & b.m_bits;
  if (!mayHaveData(isect)) return Type { isect };

  auto fix = [&] (Type& t) {
    t.m_bits = isect;
    return std::move(t);
  };

  auto aProjected = project_data(a, isect);
  auto bProjected = project_data(b, isect);

  if (!bProjected.hasData()) return fix(aProjected);
  if (!aProjected.hasData()) return fix(bProjected);
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
        case DataTag::Str:
        case DataTag::ArrLikeVal:
        case DataTag::Int:
        case DataTag::Dbl:
          // Neither is a subtype of the other, so the intersection is empty
          return TBottom;
        case DataTag::RefInner:
        {
          auto inner = intersection_of(*a.m_data.inner, *b.m_data.inner);
          if (inner == TBottom) return TBottom;
          *a.m_data.inner.mutate() = inner;
          return fix(a);
        }
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
    isect & ~(BInt|BDbl|BSStr|BArrN|BVecN|BDictN|BKeysetN|BObj|BRef);
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

  if (is_ref_with_inner(a) && is_ref_with_inner(b)) {
    return ref_to(union_of(*a.m_data.inner, *b.m_data.inner));
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
  X(Cls)

  // optional types
  Y(Bool)
  Y(Int)
  Y(Dbl)
  Y(Num)
  Y(SStr)
  Y(Str)
  Y(Obj)

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

  Y(UncStrLike)
  Y(StrLike)

  Y(ClsMeth)
  Y(Func)

  // non-optional types that contain other types above (and hence
  // must come after them).
  X(InitPrim)
  X(Prim)
  X(InitUnc)
  X(Unc)
  X(InitCell)
  X(Cell)
  X(InitGen)
  X(Gen)

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
    } else if (trep(t & BInitCell) == t) {
      t = BInitCell;
    } else {
      t = BInitGen;
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
      return cellToBool(*v) ? Emptiness::Maybe : Emptiness::Empty;
    }
  } else if (auto v = tv(t)) {
    return cellToBool(*v) ? Emptiness::NonEmpty : Emptiness::Empty;
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
    case DataTag::ArrLikeVal:
      return;

    case DataTag::Obj:
      if (t.m_data.dobj.whType) {
        widen_type_impl(*t.m_data.dobj.whType.mutate(), depth + 1);
      }
      return;

    case DataTag::RefInner:
      return widen_type_impl(*t.m_data.inner.mutate(), depth + 1);

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
  if (a.subtypeOf(BRef))      return TRef;
  if (a.subtypeOf(BGen))      return TGen;
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
    case DataTag::ArrLikeVal:
      break;

    case DataTag::Obj:
      if (t.m_data.dobj.whType) {
        auto whType = t.m_data.dobj.whType.mutate();
        *whType = loosen_staticness(std::move(*whType));
      }
      break;

    case DataTag::RefInner: {
      auto inner = t.m_data.inner.mutate();
      *inner = loosen_staticness(std::move(*inner));
      break;
    }

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
    if (auto p = toDArrLikePacked(t.m_data.aval)) {
      t = packed_impl(t.m_bits, std::move(p->elems), p->provenance);
    } else {
      auto d = toDArrLikeMap(t.m_data.aval);
      t = map_impl(t.m_bits, std::move(d->map), d->provenance);
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
  if (t.couldBe(BVec) || t.couldBe(BDict)) {
    switch (t.m_dataTag) {
    case DataTag::None:
    case DataTag::Str:
    case DataTag::Int:
    case DataTag::Dbl:
    case DataTag::Cls:
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

    case DataTag::RefInner: {
      auto inner = t.m_data.inner.mutate();
      *inner = loosen_provenance(std::move(*inner));
      break;
    }

    case DataTag::ArrLikePacked: {
      auto& packed = *t.m_data.packed.mutate();
      packed.provenance = folly::none;
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
      map.provenance = folly::none;
      for (auto it = map.map.begin(); it != map.map.end(); it++) {
        map.map.update(it, loosen_provenance(it->second));
      }
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
  if (a.couldBe(BArr))    a |= TArr;
  if (a.couldBe(BVec))    a |= TVec;
  if (a.couldBe(BDict))   a |= TDict;
  if (a.couldBe(BKeyset)) a |= TKeyset;
  return a;
}

Type loosen_values(Type a) {
  auto t = [&]{
    switch (a.m_dataTag) {
    case DataTag::Str:
    case DataTag::Int:
    case DataTag::Dbl:
    case DataTag::RefInner:
    case DataTag::ArrLikeVal:
    case DataTag::ArrLikePacked:
    case DataTag::ArrLikePackedN:
    case DataTag::ArrLikeMap:
    case DataTag::ArrLikeMapN:
      return Type { a.m_bits };
    case DataTag::None:
    case DataTag::Obj:
    case DataTag::Cls:
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

Type loosen_all(Type t) {
  return loosen_dvarrayness(
    loosen_staticness(
      loosen_emptiness(
        loosen_values(std::move(t))
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
  assert(t.subtypeOf(BGen));
  if (!t.couldBe(BUninit))  return t;
  if (isPredefined(t.m_bits & ~BUninit)) {
    t.m_bits &= ~BUninit;
    return t;
  }
  return t.subtypeOf(BCell) ? TInitCell : TInitGen;
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
  if (auto const v = tv(t)) return cellToBool(*v) ? t : TBottom;
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
  IterateKV(ad, [&] (Cell k, TypedValue v) {
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
      folly::none
    );
  }
  return false;
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

  auto const tag = (map.m_bits & BDictN) ?
    (map.m_data.map->provenance ? map.m_data.map->provenance : src) :
    folly::none;

  if (auto const k = key.tv()) {
    auto r = map.m_data.map.mutate()->map.emplace_back(*k, val);
    // if the element existed, and was a ref, its still a ref after
    // assigning to it
    if (!r.second && r.first->second.subtypeOf(BInitCell)) {
      map.m_data.map.mutate()->map.update(r.first, val);
    }
    map.m_data.map.mutate()->provenance = tag;
    return true;
  }
  auto mkv = map_key_values(*map.m_data.map);
  map = mapn_impl_from_map(
    map.m_bits,
    union_of(std::move(mkv.first), key.type),
    union_of(std::move(mkv.second), val),
    tag
  );
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
  auto const tag = pack.m_bits & (BVecN | BDictN) ?
    (pack.m_data.packed->provenance ? pack.m_data.packed->provenance : src) :
    folly::none;

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
      pack = map_impl(pack.m_bits, std::move(elems), tag);
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
  auto const tag = map.m_bits & BDictN ?
    (map.m_data.map->provenance ? map.m_data.map->provenance : src) :
    folly::none;
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
    case DataTag::RefInner:
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
  const bool isPhpArray = arr.couldBe(BArr);
  DEBUG_ONLY const bool isVArray   = arr.subtypeOrNull(BVArr);
  const bool validKey   = key.type.subtypeOf(isVector ? BInt : BArrKey);

  trep bits = combine_dv_arr_like_bits(arr.m_bits, BArrLikeN);
  if (validKey) bits &= ~BArrLikeE;

  auto const fixRef  = !isPhpArray && valIn.couldBe(BRef);
  auto const throwMode = !fixRef && validKey && !key.mayThrow ?
    ThrowMode::None : ThrowMode::BadOperation;
  auto const& val    = fixRef ? TInitCell : valIn;
  // We don't want to store types more general than TArrKey into specialized
  // array type keys. If the key was strange (array or object), it will be more
  // general than TArrKey (this is needed so we can set validKey above), so
  // force it to TArrKey.
  auto const& fixedKey = validKey
    ? key
    : []{ ArrKey key; key.type = TArrKey; key.mayThrow = true; return key; }();

  if (!arr.couldBe(BArrLikeN)) {
    assert(maybeEmpty);
    if (isVector) return { TBottom, ThrowMode::BadOperation };
    if (fixedKey.i) {
      if (!*fixedKey.i) {
        return { packed_impl(bits, { val }, src), throwMode };
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
      return { map_impl(bits, std::move(m), src), throwMode };
    }
    return { mapn_impl_from_map(bits, fixedKey.type, val, src), throwMode };
  }

  auto emptyHelper = [&] (const Type& inKey,
                          const Type& inVal) -> std::pair<Type,ThrowMode> {
    bits = fixedKey.type.subtypeOf(BStr)
      ? promote_varray(bits)
      : maybe_promote_varray(bits);
    return { mapn_impl_from_map(bits,
                                union_of(inKey, fixedKey.type),
                                union_of(inVal, val),
                                src), throwMode };
  };

  arr.m_bits = bits;

  switch (arr.m_dataTag) {
  case DataTag::Str:
  case DataTag::Obj:
  case DataTag::Int:
  case DataTag::Dbl:
  case DataTag::Cls:
  case DataTag::RefInner:
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
        return array_like_set(packed_impl(bits,
                                          std::move(d->elems),
                                          d->provenance),
                              key, valIn, src);
      }
      assert(!isVector);
      assert(!isVArray);
      // We know its not packed, so this should always succeed
      auto d = toDArrLikeMap(arr.m_data.aval);
      return array_like_set(map_impl(bits, std::move(d->map), d->provenance),
                            key, valIn, src);
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

  // Unless you know an array can't cow, you don't know if the TRef
  // will stay a TRef or turn back into a TInitCell.  Generally you
  // want a TInitGen.
  always_assert((val == TBottom || !val.subtypeOf(BRef)) &&
                "You probably don't want to put Ref types into arrays ...");

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
    return { packed_impl(bits, { val }, src), ival(0) };
  }

  auto emptyHelper = [&] (const Type& inKey,
                          const Type& inVal) -> std::pair<Type,Type> {
    if (isVector || isVArray) {
      assert(inKey.subtypeOf(BInt));
      return { packedn_impl(bits, union_of(inVal, val)), TInt };
    }

    return { mapn_impl_from_map(bits,
                                union_of(inKey, TInt),
                                union_of(inVal, val),
                                src), TInt };
  };

  switch (arr.m_dataTag) {
  case DataTag::Str:
  case DataTag::Obj:
  case DataTag::Int:
  case DataTag::Dbl:
  case DataTag::Cls:
  case DataTag::RefInner:
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
      return array_like_newelem(map_impl(bits, std::move(d->map),
                                         d->provenance), val, src);
    }

  case DataTag::ArrLikePacked:
    if (maybeEmpty) {
      return emptyHelper(TInt, packed_values(*arr.m_data.packed));
    } else {
      arr.m_bits = bits;
      auto len = arr.m_data.packed->elems.size();
      arr.m_data.packed.mutate()->elems.push_back(val);
      if (arr.m_bits & (BVecN | BDictN)) {
        arr.m_data.packed.mutate()->provenance = arr.m_data.packed->provenance ?
          arr.m_data.packed->provenance :
          src;
      }
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
    return { mapn_impl_from_map(bits,
                                union_of(arr.m_data.mapn->key, TInt),
                                union_of(arr.m_data.mapn->val, val),
                                src),
             TInt };
  }

  not_reached();
}

std::pair<Type,Type> array_newelem(Type arr, const Type& val, ProvTag src) {
  assert(arr.subtypeOf(BArr));

  // Unless you know an array can't cow, you don't know if the TRef
  // will stay a TRef or turn back into a TInitCell.  Generally you
  // want a TInitGen.
  always_assert((val == TBottom || !val.subtypeOf(BRef)) &&
         "You probably don't want to put Ref types into arrays ...");

  return array_like_newelem(std::move(arr), val, src);
}

IterTypes iter_types(const Type& iterable) {
  // Only array types and objects can be iterated. Everything else raises a
  // warning and jumps out of the loop.
  if (!iterable.couldBeAny(TArr, TVec, TDict, TKeyset, TObj)) {
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
  case DataTag::RefInner:
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
      count(iterable.m_data.map->map.size()),
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

  if (t.couldBe(BRef)) {
    if (!couldBeArrWithDestructors && is_ref_with_inner(t)) {
      return could_contain_objects(*t.m_data.inner);
    }
    return true;
  }

  if (!couldBeArrWithDestructors) return false;

  switch (t.m_dataTag) {
  case DataTag::None:
  case DataTag::Str:
  case DataTag::Obj:
  case DataTag::Int:
  case DataTag::Dbl:
  case DataTag::Cls:
  case DataTag::RefInner:
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
  if (is_opt(testTy)) return is_type_might_raise(unopt(testTy), valTy);
  if (testTy == TStrLike) {
    return valTy.couldBe(BFunc | BCls);
  } else if (testTy == TArr) {
    if (RuntimeOption::EvalHackArrCompatIsArrayNotices &&
        valTy.couldBe(BVec | BDict | BKeyset)) {
      return true;
    }
    return RuntimeOption::EvalIsVecNotices &&
          !RuntimeOption::EvalHackArrDVArrs && valTy.couldBe(BClsMeth);
  } else if (testTy == TVArr) {
    if (RuntimeOption::EvalHackArrCompatIsArrayNotices && valTy.couldBe(BVec)) {
      return true;
    }
    return RuntimeOption::EvalIsVecNotices && valTy.couldBe(BClsMeth);
  } else if (testTy == TDArr) {
    return RuntimeOption::EvalHackArrCompatIsArrayNotices &&
           valTy.couldBe(BDict);
 } else if (testTy == TVec) {
   if (RuntimeOption::EvalHackArrCompatIsVecDictNotices &&
       valTy.couldBe(BVArr)) {
     return true;
   }
   return RuntimeOption::EvalIsVecNotices &&
          RuntimeOption::EvalHackArrDVArrs && valTy.couldBe(BClsMeth);
  } else if (testTy == TDict) {
    return RuntimeOption::EvalHackArrCompatIsArrayNotices &&
           valTy.couldBe(BDArr);
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
      case DataTag::RefInner:
        not_reached();

      case DataTag::ArrLikeVal:
        sz = t.m_data.aval->size();
        return true;
      case DataTag::ArrLikePacked:
        sz = t.m_data.packed->elems.size();
        return true;
      case DataTag::ArrLikePackedN:
        return t.m_data.packedn->type.couldBe(BArrLike | BObj);
      case DataTag::ArrLikeMap:
        sz = t.m_data.map->map.size();
        return true;
      case DataTag::ArrLikeMapN:
        return t.m_data.mapn->val.couldBe(BArrLike | BObj);
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

  for (size_t i = 0; i < numToCheck; i++) {
    auto const nextType = [&] (const Type& t, ArrPos& p) {
      switch (t.m_dataTag) {
        case DataTag::None:
          return TInitCell;

        case DataTag::Str:
        case DataTag::Obj:
        case DataTag::Int:
        case DataTag::Dbl:
        case DataTag::Cls:
        case DataTag::RefInner:
          not_reached();

        case DataTag::ArrLikeVal:
          if (!i) {
            p.pos = t.m_data.aval->iter_begin();
          } else {
            p.pos = t.m_data.aval->iter_advance(p.pos);
          }
          return from_cell(t.m_data.aval->atPos(p.pos));
        case DataTag::ArrLikePacked:
          return t.m_data.packed->elems[i];
        case DataTag::ArrLikePackedN:
          return t.m_data.packedn->type;
        case DataTag::ArrLikeMap:
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
    if (compare_might_raise(nextType(t1, p1), nextType(t2, p2))) {
      return true;
    }
  }
  return false;
}

bool compare_might_raise(const Type& t1, const Type& t2) {
  if (!RuntimeOption::EvalHackArrCompatNotices &&
      !RuntimeOption::EvalHackArrCompatDVCmpNotices) {
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
  return array_like_newelem(std::move(keyset), val, folly::none);
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
    case DataTag::RefInner:
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
  X(InitUnc)
  X(Unc)
  X(InitCell)
  X(Cell)
  X(Ref)
  X(InitGen)
  X(Gen)
#undef X
  not_reached();
}

//////////////////////////////////////////////////////////////////////

Type adjust_type_for_prop(const Index& index,
                          const php::Class& propCls,
                          const TypeConstraint* tc,
                          const Type& ty) {
  // If the type-hint might not be enforced, we must be conservative.
  if (!tc || index.prop_tc_maybe_unenforced(propCls, *tc)) return ty;
  auto const ctx = Context { nullptr, nullptr, &propCls };
  // Otherwise lookup what we know about the constraint.
  auto tcType = unctx(
    loosen_dvarrayness(remove_uninit(index.lookup_constraint(ctx, *tc, ty)))
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
  return intersection_of(tcType, ty);
}

//////////////////////////////////////////////////////////////////////

}}
