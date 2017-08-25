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

#include "hphp/hhbbc/eval-cell.h"
#include "hphp/hhbbc/index.h"

namespace HPHP { namespace HHBBC {

TRACE_SET_MOD(hhbbc);

#define X(y) const Type T##y = Type(B##y);
TYPES(X)
#undef X

namespace {

//////////////////////////////////////////////////////////////////////

const StaticString s_WaitHandle("HH\\WaitHandle");
const StaticString s_empty("");

//////////////////////////////////////////////////////////////////////

// Legal to call with !isPredefined(bits)
bool mayHaveData(trep bits) {
  switch (bits) {
  case BSStr:    case BObj:    case BInt:    case BDbl:
  case BOptSStr: case BOptObj: case BOptInt: case BOptDbl:
  case BCls:
  case BArr:     case BSArr:     case BCArr:
  case BArrN:    case BSArrN:    case BCArrN:
  case BOptArr:  case BOptSArr:  case BOptCArr:
  case BOptArrN: case BOptSArrN: case BOptCArrN:
  case BRef:
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

  case BBottom:
  case BUninit:
  case BInitNull:
  case BFalse:
  case BTrue:
  case BCStr:
  case BSArrE:
  case BCArrE:
  case BSVecE:
  case BCVecE:
  case BSDictE:
  case BCDictE:
  case BSKeysetE:
  case BCKeysetE:
  case BRes:
  case BNull:
  case BNum:
  case BBool:
  case BStr:
  case BArrE:
  case BVecE:
  case BDictE:
  case BKeysetE:
  case BInitPrim:
  case BPrim:
  case BInitUnc:
  case BUnc:
  case BArrKey:
  case BUncArrKey:
  case BOptTrue:
  case BOptFalse:
  case BOptBool:
  case BOptNum:
  case BOptCStr:
  case BOptStr:
  case BOptSArrE:
  case BOptCArrE:
  case BOptArrE:
  case BOptSVecE:
  case BOptCVecE:
  case BOptVecE:
  case BOptSDictE:
  case BOptCDictE:
  case BOptDictE:
  case BOptSKeysetE:
  case BOptCKeysetE:
  case BOptKeysetE:
  case BOptRes:
  case BOptArrKey:
  case BOptUncArrKey:
  case BInitCell:
  case BCell:
  case BInitGen:
  case BGen:
  case BTop:
    break;
  }
  return false;
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
  switch (bits) {
  case BBottom:
  case BUninit:
  case BInitNull:
  case BFalse:
  case BTrue:
  case BInt:
  case BDbl:
  case BSStr:
  case BCStr:
  case BSArrE:
  case BCArrE:
  case BSArrN:
  case BCArrN:
  case BSVecE:
  case BCVecE:
  case BSVecN:
  case BCVecN:
  case BSDictE:
  case BCDictE:
  case BSDictN:
  case BCDictN:
  case BSKeysetE:
  case BCKeysetE:
  case BSKeysetN:
  case BCKeysetN:
  case BObj:
  case BRes:
  case BCls:
  case BRef:
  case BNull:
  case BNum:
  case BBool:
  case BStr:
  case BSArr:
  case BCArr:
  case BArrE:
  case BArrN:
  case BArr:
  case BSVec:
  case BCVec:
  case BVecE:
  case BVecN:
  case BVec:
  case BSDict:
  case BCDict:
  case BDictE:
  case BDictN:
  case BDict:
  case BSKeyset:
  case BCKeyset:
  case BKeysetE:
  case BKeysetN:
  case BKeyset:
  case BInitPrim:
  case BPrim:
  case BInitUnc:
  case BUnc:
  case BUncArrKey:
  case BArrKey:
  case BOptTrue:
  case BOptFalse:
  case BOptBool:
  case BOptInt:
  case BOptDbl:
  case BOptNum:
  case BOptSStr:
  case BOptCStr:
  case BOptStr:
  case BOptSArrN:
  case BOptCArrN:
  case BOptSArrE:
  case BOptCArrE:
  case BOptSArr:
  case BOptCArr:
  case BOptArrE:
  case BOptArrN:
  case BOptArr:
  case BOptSVecN:
  case BOptCVecN:
  case BOptSVecE:
  case BOptCVecE:
  case BOptSVec:
  case BOptCVec:
  case BOptVecE:
  case BOptVecN:
  case BOptVec:
  case BOptSDictN:
  case BOptCDictN:
  case BOptSDictE:
  case BOptCDictE:
  case BOptSDict:
  case BOptCDict:
  case BOptDictE:
  case BOptDictN:
  case BOptDict:
  case BOptSKeysetN:
  case BOptCKeysetN:
  case BOptSKeysetE:
  case BOptCKeysetE:
  case BOptSKeyset:
  case BOptCKeyset:
  case BOptKeysetE:
  case BOptKeysetN:
  case BOptKeyset:
  case BOptObj:
  case BOptRes:
  case BOptUncArrKey:
  case BOptArrKey:
  case BInitCell:
  case BCell:
  case BInitGen:
  case BGen:
  case BTop:
    return true;
  }
  return false;
}

// Pre: isPredefined(bits)
bool canBeOptional(trep bits) {
  switch (bits) {
  case BBottom:
    return false;

  case BUninit:
  case BInitNull:
    return false;
  case BFalse:
  case BTrue:
  case BInt:
  case BDbl:
  case BSStr:
  case BCStr:
  case BSArrE:
  case BSArrN:
  case BCArrE:
  case BCArrN:
  case BSVecE:
  case BSVecN:
  case BCVecE:
  case BCVecN:
  case BSDictE:
  case BSDictN:
  case BCDictE:
  case BCDictN:
  case BSKeysetE:
  case BSKeysetN:
  case BCKeysetE:
  case BCKeysetN:
  case BObj:
  case BRes:
    return true;

  case BNull:
  case BNum:
  case BBool:
  case BStr:
  case BUncArrKey:
  case BArrKey:
  case BSArr:
  case BCArr:
  case BArrE:
  case BArrN:
  case BArr:
  case BSVec:
  case BCVec:
  case BVecE:
  case BVecN:
  case BVec:
  case BSDict:
  case BCDict:
  case BDictE:
  case BDictN:
  case BDict:
  case BSKeyset:
  case BCKeyset:
  case BKeysetE:
  case BKeysetN:
  case BKeyset:
    return true;

  case BCls:
  case BRef:
    return false;

  case BOptTrue:
  case BOptFalse:
  case BOptBool:
  case BOptInt:
  case BOptDbl:
  case BOptNum:
  case BOptSStr:
  case BOptCStr:
  case BOptStr:
  case BOptSArrE:
  case BOptCArrE:
  case BOptSArrN:
  case BOptCArrN:
  case BOptSArr:
  case BOptCArr:
  case BOptArrN:
  case BOptArrE:
  case BOptArr:
  case BOptSVecE:
  case BOptCVecE:
  case BOptSVecN:
  case BOptCVecN:
  case BOptSVec:
  case BOptCVec:
  case BOptVecN:
  case BOptVecE:
  case BOptVec:
  case BOptSDictE:
  case BOptCDictE:
  case BOptSDictN:
  case BOptCDictN:
  case BOptSDict:
  case BOptCDict:
  case BOptDictN:
  case BOptDictE:
  case BOptDict:
  case BOptSKeysetE:
  case BOptCKeysetE:
  case BOptSKeysetN:
  case BOptCKeysetN:
  case BOptSKeyset:
  case BOptCKeyset:
  case BOptKeysetN:
  case BOptKeysetE:
  case BOptKeyset:
  case BOptObj:
  case BOptRes:
  case BOptUncArrKey:
  case BOptArrKey:
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
  }
  not_reached();
}

/*
 * Combine array bits.  Our type system currently avoids arbitrary unions (see
 * rationale above), so we don't have predefined types like CArr|SArrN, or
 * SArrN|CArrE.  This function checks a few cases to ensure combining array
 * type bits leaves it predefined.
 */
template<trep B>
trep combine_arrish_bits(trep a, trep b) {
  DEBUG_ONLY constexpr trep OptB = static_cast<trep>(BInitNull | B);
  auto const combined = static_cast<trep>(a | b);
  assert((combined & OptB) == combined);
  auto const arr_part = static_cast<trep>(combined & B);
  if (!isPredefined(arr_part)) return static_cast<trep>(combined|B);
  assert(isPredefined(combined));
  return combined;
}

trep combine_arr_bits(trep a, trep b) {
  return combine_arrish_bits<BArr>(a, b);
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
trep combine_arr_like_bits(trep a, trep b) {
  auto check = [] (trep a, trep x) { return (a & x) == a; };
  assert(a && isPredefined(a) && !check(a, BInitNull));
  if (check(a, BOptArr))    return combine_arr_bits(a,    trep(b & BOptArr));
  if (check(a, BOptVec))    return combine_vec_bits(a,    trep(b & BOptVec));
  if (check(a, BOptDict))   return combine_dict_bits(a,   trep(b & BOptDict));
  if (check(a, BOptKeyset)) return combine_keyset_bits(a, trep(b & BOptKeyset));
  not_reached();
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

  return DArrLikePacked { std::move(elems) };
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
  return DArrLikeMap { std::move(map) };
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

bool subtypePacked(const DArrLikePacked& a, const DArrLikePacked& b) {
  auto const asz = a.elems.size();
  auto const bsz = b.elems.size();
  if (asz != bsz) return false;
  for (auto i = size_t{0}; i < asz; ++i) {
    if (!a.elems[i].subtypeOf(b.elems[i])) {
      return false;
    }
  }
  return true;
}

bool subtypeMap(const DArrLikeMap& a, const DArrLikeMap& b) {
  if (a.map.size() != b.map.size()) return false;
  auto aIt = begin(a.map);
  auto bIt = begin(b.map);
  for (; aIt != end(a.map); ++aIt, ++bIt) {
    if (!cellSame(aIt->first, bIt->first)) return false;
    if (!aIt->second.subtypeOf(bIt->second)) return false;
  }
  return true;
}

bool couldBePacked(const DArrLikePacked& a, const DArrLikePacked& b) {
  auto const asz = a.elems.size();
  auto const bsz = b.elems.size();
  if (asz != bsz) return false;
  for (auto i = size_t{0}; i < asz; ++i) {
    if (!a.elems[i].couldBe(b.elems[i])) {
      return false;
    }
  }
  return true;
}

bool couldBeMap(const DArrLikeMap& a, const DArrLikeMap& b) {
  if (a.map.size() != b.map.size()) return false;
  auto aIt = begin(a.map);
  auto bIt = begin(b.map);
  for (; aIt != end(a.map); ++aIt, ++bIt) {
    if (!cellSame(aIt->first, bIt->first)) return false;
    if (!aIt->second.couldBe(bIt->second)) return false;
  }
  return true;
}

//////////////////////////////////////////////////////////////////////

std::pair<Type,Type> val_key_values(SArray a) {
  auto ret = std::make_pair(TBottom, TBottom);
  for (ArrayIter iter(a); iter; ++iter) {
    ret.first |= from_cell(*iter.first().asTypedValue());
    ret.second |= from_cell(*iter.secondRef().asTypedValue());
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

  template<class B>
  result_type operator()(SArray a, const B& b) const {
    return InnerFn::operator()(b, a);
  }

  template<class B>
  result_type operator()(const DArrLikeMap& a, const B& b) const {
    return InnerFn::operator()(b, a);
  }

  template<class B>
  result_type operator()(const DArrLikePackedN& a, const B& b) const {
    return InnerFn::operator()(b, a);
  }

  template<class B>
  result_type operator()(const DArrLikeMapN& a, const B& b) const {
    return InnerFn::operator()(b, a);
  }
};

struct DualDispatchEqImpl {
  static constexpr bool disjoint = true;
  using result_type = bool;

  bool operator()() const { return false; }

  bool operator()(const DArrLikePacked& a, SArray b) const {
    if (a.elems.size() != b->size()) return false;
    auto const p = toDArrLikePacked(b);
    return p && a.elems == p->elems;
  }

  bool operator()(const DArrLikeMap& a, SArray b) const {
    if (a.map.size() != b->size()) return false;
    auto const m = toDArrLikeMap(b);
    return m && a.map == m->map;
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
    auto const p = toDArrLikePacked(b);
    return p && couldBePacked(a, *p);
  }

  bool operator()(const DArrLikeMap& a, SArray b) const {
    if (a.map.size() != b->size()) return false;
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

// The countedness or possible-emptiness of the arrays is handled
// outside of this function, so it's ok to just return TArr from all
// of these here.

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
    return packed_impl(bits, std::move(ret));
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

      return mapn_impl(
        bits,
        union_of(std::move(mkva.first), std::move(mkvb.first)),
        union_of(std::move(mkva.second), std::move(mkvb.second)));
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
    return map_impl(bits, std::move(retStruct));
  }

  Type operator()(SArray a, SArray b) const {
    assert(a != b); // Should've been handled earlier in union_of.

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
    return mapn_impl(bits, union_of(a.key, b.key), union_of(a.val, b.val));
  }

  Type operator()(const DArrLikePacked& a, SArray b) const {
    auto const p = toDArrLikePacked(b);
    if (p) return (*this)(a, *p);
    return (*this)(a, *toDArrLikeMap(b));
  }

  Type operator()(const DArrLikePackedN& a, SArray b) const {
    auto const p = toDArrLikePackedN(b);
    if (p) return (*this)(a, *p);
    return (*this)(a, *toDArrLikeMap(b));
  }

  Type operator()(const DArrLikeMap& a, SArray b) const {
    auto const m = toDArrLikeMap(b);
    if (m) return (*this)(a, *m);
    return (*this)(*toDArrLikePacked(b), a);
  }

  Type operator()(const DArrLikeMapN& a, SArray b) const {
    auto const m1 = toDArrLikeMapN(b);
    if (m1) return (*this)(a, *m1);
    auto const m2 = toDArrLikeMap(b);
    if (m2) return (*this)(*m2, a);
    return (*this)(*toDArrLikePackedN(b), a);
  }

  Type operator()(const DArrLikePacked& a, const DArrLikeMap& b) const {
    auto mkv = map_key_values(b);
    return mapn_impl(bits, union_of(TInt, std::move(mkv.first)),
                     union_of(packed_values(a), std::move(mkv.second)));
  }

  Type operator()(const DArrLikePacked& a, const DArrLikeMapN& b) const {
    return mapn_impl(bits, union_of(b.key, TInt),
                     union_of(packed_values(a), b.val));
  }

  Type operator()(const DArrLikePackedN& a, const DArrLikeMap& b) const {
    auto mkv = map_key_values(b);
    return mapn_impl(bits, union_of(TInt, std::move(mkv.first)),
                     union_of(a.type, std::move(mkv.second)));
  }

  Type operator()(const DArrLikePackedN& a, const DArrLikeMapN& b) const {
    return mapn_impl(bits, union_of(TInt, b.key), union_of(a.type, b.val));
  }

  Type operator()(const DArrLikeMap& a, const DArrLikeMapN& b) const {
    auto mkv = map_key_values(a);
    return mapn_impl(bits,
                     union_of(std::move(mkv.first), b.key),
                     union_of(std::move(mkv.second), b.val));
  }

private:
  trep bits;
};

/*
 * Subtype is not a commutative relation, so this is the only
 * dualDispatchDataFn helper that doesn't use Commute<>.
 */
struct DualDispatchSubtype {
  static constexpr bool disjoint = true;
  using result_type = bool;

  bool operator()() const { return false; }

  bool operator()(const DArrLikeMap& a, SArray b) const {
    if (a.map.size() != b->size()) return false;
    auto const m = toDArrLikeMap(b);
    return m && subtypeMap(a, *m);
  }

  bool operator()(SArray a, const DArrLikeMap& b) const {
    if (a->size() != b.map.size()) return false;
    auto const m = toDArrLikeMap(a);
    return m && subtypeMap(*m, b);
  }

  bool operator()(SArray a, const DArrLikePacked& b) const {
    if (a->size() != b.elems.size()) return false;
    auto const p = toDArrLikePacked(a);
    return p && subtypePacked(*p, b);
  }

  bool operator()(const DArrLikePacked& a, SArray b) const {
    if (a.elems.size() != b->size()) return false;
    auto const p = toDArrLikePacked(b);
    return p && subtypePacked(a, *p);
  }

  bool operator()(const DArrLikePackedN& a, const DArrLikeMapN& b) const {
    return b.key.couldBe(TInt) && a.type.subtypeOf(b.val);
  }

  bool operator()(const DArrLikePacked& a, const DArrLikeMapN& b) const {
    if (!b.key.couldBe(TInt)) return false;
    for (auto const& v : a.elems) {
      if (!v.subtypeOf(b.val)) return false;
    }
    return true;
  }

  bool operator()(const DArrLikeMap& a, const DArrLikeMapN& b) const {
    for (auto const& kv : a.map) {
      if (!from_cell(kv.first).subtypeOf(b.key)) return false;
      if (!kv.second.subtypeOf(b.val)) return false;
    }
    return true;
  }

  bool operator()(SArray a, const DArrLikeMapN& b) const {
    assert(!a->empty());
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
      if (!t.subtypeOf(b.type)) return false;
    }
    return true;
  }

  bool operator()(SArray a, const DArrLikePackedN& b) const {
    auto p = toDArrLikePackedN(a);
    return p && p->type.subtypeOf(b.type);
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

using DualDispatchEq      = Commute<DualDispatchEqImpl>;
using DualDispatchCouldBe = Commute<DualDispatchCouldBeImpl>;
using DualDispatchUnion   = Commute<DualDispatchUnionImpl>;

//////////////////////////////////////////////////////////////////////
// Helpers for creating literal array-like types

template<typename AInit>
folly::Optional<Cell> fromTypeVec(const std::vector<Type> &elems) {
  AInit ai(elems.size());
  for (auto const& t : elems) {
    auto const v = tv(t);
    if (!v) return folly::none;
    ai.append(tvAsCVarRef(&*v));
  }
  auto var = ai.toVariant();
  var.setEvalScalar();
  return *var.asTypedValue();
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

template<typename AInit, typename Key>
folly::Optional<Cell> fromTypeMap(const ArrayLikeMap<Key> &elems) {
  auto val = eval_cell_value([&] () -> Cell {
    AInit ai(elems.size());
    for (auto const& elm : elems) {
      auto const v = tv(elm.second);
      if (!v) return make_tv<KindOfUninit>();
      add(ai, keyHelper(elm.first), tvAsCVarRef(&*v));
    }
    auto var = ai.toVariant();
    var.setEvalScalar();
    return *var.asTypedValue();
  });
  if (val && val->m_type == KindOfUninit) val.clear();
  return val;
}

//////////////////////////////////////////////////////////////////////

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

//////////////////////////////////////////////////////////////////////

template<class Ret, class T, class Function>
struct Type::DDHelperFn {
  template<class Y>
  typename std::enable_if<!std::is_same<Y,T>::value ||
                          !Function::disjoint, Ret>::type
  operator()(const Y& y) const { return f(t, y); }

  template <class Y>
  typename std::enable_if<std::is_same<Y, T>::value && Function::disjoint,
                          Ret>::type
  operator()(const Y& /*y*/) const {
    not_reached();
  }
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
  case DataTag::Str:            return f();
  case DataTag::Obj:            return f();
  case DataTag::Int:            return f();
  case DataTag::Dbl:            return f();
  case DataTag::Cls:            return f();
  case DataTag::RefInner:       return f();
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
  case DataTag::Str:            return f();
  case DataTag::Obj:            return f();
  case DataTag::Int:            return f();
  case DataTag::Dbl:            return f();
  case DataTag::Cls:            return f();
  case DataTag::RefInner:       return f();
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

bool Type::equivData(const Type& o) const {
  if (m_dataTag != o.m_dataTag) {
    return dualDispatchDataFn(o, DualDispatchEq{});
  }

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
    // For purposes of Type equivalence, NaNs are equal.
    return m_data.dval == o.m_data.dval ||
           (std::isnan(m_data.dval) && std::isnan(o.m_data.dval));
  case DataTag::Obj:
    assert(!m_data.dobj.whType);
    assert(!o.m_data.dobj.whType);
    return m_data.dobj.type == o.m_data.dobj.type &&
           m_data.dobj.cls.same(o.m_data.dobj.cls);
  case DataTag::Cls:
    return m_data.dcls.type == o.m_data.dcls.type &&
           m_data.dcls.cls.same(o.m_data.dcls.cls);
  case DataTag::RefInner:
    return *m_data.inner == *o.m_data.inner;
  case DataTag::ArrLikePacked:
    return m_data.packed->elems == o.m_data.packed->elems;
  case DataTag::ArrLikePackedN:
    return m_data.packedn->type == o.m_data.packedn->type;
  case DataTag::ArrLikeMap:
    return m_data.map->map == o.m_data.map->map;
  case DataTag::ArrLikeMapN:
    return m_data.mapn->key == o.m_data.mapn->key &&
           m_data.mapn->val == o.m_data.mapn->val;
  }
  not_reached();
}

bool Type::subtypeData(const Type& o) const {
  if (m_dataTag != o.m_dataTag) {
    return dualDispatchDataFn(o, DualDispatchSubtype{});
  }

  switch (m_dataTag) {
  case DataTag::Obj:
    assert(!m_data.dobj.whType);
    assert(!o.m_data.dobj.whType);
    if (m_data.dobj.type == o.m_data.dobj.type &&
        m_data.dobj.cls.same(o.m_data.dobj.cls)) {
      return true;
    }
    if (o.m_data.dobj.type == DObj::Sub) {
      return m_data.dobj.cls.subtypeOf(o.m_data.dobj.cls);
    }
    return false;
  case DataTag::Cls:
    if (m_data.dcls.type == o.m_data.dcls.type &&
        m_data.dcls.cls.same(o.m_data.dcls.cls)) {
      return true;
    }
    if (o.m_data.dcls.type == DCls::Sub) {
      return m_data.dcls.cls.subtypeOf(o.m_data.dcls.cls);
    }
    return false;
  case DataTag::Str:
  case DataTag::ArrLikeVal:
  case DataTag::Int:
  case DataTag::Dbl:
  case DataTag::None:
    return equivData(o);
  case DataTag::RefInner:
    return m_data.inner->subtypeOf(*o.m_data.inner);
  case DataTag::ArrLikePacked:
    return subtypePacked(*m_data.packed, *o.m_data.packed);
  case DataTag::ArrLikePackedN:
    return m_data.packedn->type.subtypeOf(o.m_data.packedn->type);
  case DataTag::ArrLikeMap:
    return subtypeMap(*m_data.map, *o.m_data.map);
  case DataTag::ArrLikeMapN:
    return m_data.mapn->key.subtypeOf(o.m_data.mapn->key) &&
           m_data.mapn->val.subtypeOf(o.m_data.mapn->val);
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
    assert(!m_data.dobj.whType);
    assert(!o.m_data.dobj.whType);
    if (m_data.dobj.type == o.m_data.dobj.type &&
        m_data.dobj.cls.same(o.m_data.dobj.cls)) {
      return true;
    }
    if (m_data.dobj.type == DObj::Sub || o.m_data.dobj.type == DObj::Sub) {
      return m_data.dobj.cls.couldBe(o.m_data.dobj.cls);
    }
    return false;
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
    return m_data.aval == o.m_data.aval;
  case DataTag::Str:
    return m_data.sval == o.m_data.sval;
  case DataTag::Int:
    return m_data.ival == o.m_data.ival;
  case DataTag::Dbl:
    return m_data.dval == o.m_data.dval;
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

bool Type::operator==(const Type& o) const {
  // NB: We don't assert checkInvariants() here because this can be called from
  // checkInvariants() and it all takes too long if the type is deeply nested.

  if (m_bits != o.m_bits) return false;
  if (hasData() != o.hasData()) return false;
  if (!hasData() && !o.hasData()) return true;

  if (is_specialized_wait_handle(*this)) {
    if (is_specialized_wait_handle(o)) {
      return wait_handle_inner(*this) == wait_handle_inner(o);
    }
    return false;
  }
  if (is_specialized_wait_handle(o)) {
    return false;
  }

  return equivData(o);
}

size_t Type::hash() const {
  using U1 = std::underlying_type<decltype(m_bits)>::type;
  using U2 = std::underlying_type<decltype(m_dataTag)>::type;
  auto const rawBits = U1{m_bits};
  auto const rawTag  = static_cast<U2>(m_dataTag);
  return folly::hash::hash_combine(rawBits, rawTag);
}

bool Type::subtypeOf(const Type& o) const {
  // NB: We don't assert checkInvariants() here because this can be called from
  // checkInvariants() and it all takes too long if the type is deeply nested.

  if (is_specialized_wait_handle(*this)) {
    if (is_specialized_wait_handle(o)) {
      return
        wait_handle_inner(*this).subtypeOf(wait_handle_inner(o)) &&
        wait_handle_outer(*this).subtypeOf(wait_handle_outer(o));
    }
    return wait_handle_outer(*this).subtypeOf(o);
  }
  if (is_specialized_wait_handle(o)) {
    return subtypeOf(wait_handle_outer(o));
  }

  auto const isect = static_cast<trep>(m_bits & o.m_bits);
  if (isect != m_bits) return false;
  if (!mayHaveData(isect)) return true;

  // No data is always more general.
  if (!hasData() && !o.hasData()) return true;
  if (!o.hasData()) {
    assert(hasData());
    return true;
  }

  // Both have data, and the intersection allows it, so it depends on
  // what the data says.
  return hasData() && subtypeData(o);
}

bool Type::strictSubtypeOf(const Type& o) const {
  assert(checkInvariants());
  assert(o.checkInvariants());
  return *this != o && subtypeOf(o);
}

bool Type::couldBe(const Type& o) const {
  assert(checkInvariants());
  assert(o.checkInvariants());

  if (is_specialized_wait_handle(*this)) {
    if (is_specialized_wait_handle(o)) {
      return wait_handle_inner(*this).couldBe(wait_handle_inner(o));
    }
    return o.couldBe(wait_handle_outer(*this));
  }
  if (is_specialized_wait_handle(o)) {
    return couldBe(wait_handle_outer(o));
  }

  auto const isect = static_cast<trep>(m_bits & o.m_bits);
  if (isect == 0) return false;
  if (subtypeOf(o) || o.subtypeOf(*this)) return true;
  if (!mayHaveData(isect)) return true;

  /*
   * From here we have an intersection that may have data, and we know
   * that neither type completely contains the other.
   *
   * For most of our types, where m_data represents an exact constant
   * value, this just means the types only overlap if there is no
   * data.
   *
   * The exception to that are option types with data,
   * objects/classes, and arrays.
   */

  /*
   * If the intersection allowed data, and either type was an option
   * type, we can simplify the case to whether the unopt'd version of
   * the option type couldBe the other type.  (The case where
   * TInitNull was the overlapping part would already be handled
   * above, because !mayHaveData(TInitNull).)
   */
  if (is_opt(*this)) return is_opt(o) ? true : unopt(*this).couldBe(o);
  if (is_opt(o))     return unopt(o).couldBe(*this);

  if (hasData() && o.hasData()) {
    assert(mayHaveData(isect));
    return couldBeData(o);
  }
  return true;
}

bool Type::checkInvariants() const {
  assert(isPredefined(m_bits));
  assert(!hasData() || mayHaveData(m_bits));

  // NB: Avoid copying non-trivial types in here to avoid recursive calls to
  // checkInvariants() which can cause exponential time blow-ups.

  DEBUG_ONLY auto const& keyType = (m_bits & BSArrLike) == m_bits
    ? TUncArrKey : TArrKey;
  DEBUG_ONLY auto const& valType = (m_bits & BOptArr) == m_bits
    ? TInitGen
    : ((m_bits & BOptKeyset) == m_bits) ? TArrKey : TInitCell;
  DEBUG_ONLY auto const isPHPArray = (m_bits & BOptArr) == m_bits;
  DEBUG_ONLY auto const isVector = (m_bits & BOptVec) == m_bits;
  DEBUG_ONLY auto const isKeyset = (m_bits & BOptKeyset) == m_bits;
  DEBUG_ONLY auto const isDict = (m_bits & BOptDict) == m_bits;
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
    assert(!m_data.inner->couldBe(TRef));
    break;
  case DataTag::Cls:    break;
  case DataTag::Obj:    break;
  case DataTag::ArrLikeVal:
    assert(m_data.aval->isStatic());
    assert(!m_data.aval->empty());
    assert(!isPHPArray || m_data.aval->isPHPArray());
    assert(!isVector || m_data.aval->isVecArray());
    assert(!isKeyset || m_data.aval->isKeyset());
    assert(!isDict || m_data.aval->isDict());
    break;
  case DataTag::ArrLikePacked: {
    assert(!m_data.packed->elems.empty());
    DEBUG_ONLY auto idx = size_t{0};
    for (DEBUG_ONLY auto const& v : m_data.packed->elems) {
      assert(v.subtypeOf(valType) && v != TBottom);
      assert(!isKeyset || v == ival(idx++));
    }
    break;
  }
  case DataTag::ArrLikeMap: {
    assert(!isVector);
    assert(!m_data.map->map.empty());
    DEBUG_ONLY auto idx = size_t{0};
    DEBUG_ONLY auto packed = true;
    for (DEBUG_ONLY auto const& kv : m_data.map->map) {
      assert(cellIsPlausible(kv.first));
      assert(isIntType(kv.first.m_type) ||
             kv.first.m_type == KindOfPersistentString);
      assert(kv.second.subtypeOf(valType) && kv.second != TBottom);
      assert(!isKeyset || from_cell(kv.first) == kv.second);
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
    assert(m_data.packedn->type.subtypeOf(valType));
    assert(m_data.packedn->type != TBottom);
    assert(!isKeyset || m_data.packedn->type == TInt);
    break;
  case DataTag::ArrLikeMapN:
    assert(!isVector);
    assert(m_data.mapn->key.subtypeOf(keyType));
    // MapN shouldn't have a specialized key. If it does, then that implies it
    // only contains arrays of size 1, which means it should be Map instead.
    assert(m_data.mapn->key.m_dataTag == DataTag::None);
    assert(m_data.mapn->val.subtypeOf(valType));
    assert(m_data.mapn->key != TBottom);
    assert(m_data.mapn->val != TBottom);
    assert(!isKeyset || m_data.mapn->key == m_data.mapn->val);
    break;
  }
  return true;
}

//////////////////////////////////////////////////////////////////////

Type wait_handle(const Index& index, Type inner) {
  auto const rwh = index.builtin_class(s_WaitHandle.get());
  auto t = subObj(rwh);
  t.m_data.dobj.whType.emplace(std::move(inner));
  return t;
}

bool is_specialized_wait_handle(const Type& t) {
  return
    t.m_dataTag == DataTag::Obj &&
    !!t.m_data.dobj.whType.get();
}

Type wait_handle_inner(const Type& t) {
  assert(is_specialized_wait_handle(t));
  return *t.m_data.dobj.whType;
}

Type Type::wait_handle_outer(const Type& wh) {
  auto ret      = Type{wh.m_bits};
  ret.m_dataTag = DataTag::Obj;
  construct(ret.m_data.dobj, wh.m_data.dobj.type, wh.m_data.dobj.cls);
  return ret;
}

Type sval(SString val) {
  assert(val->isStatic());
  auto r        = Type { BSStr };
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
  assert(!val->isDict() && !val->isVecArray() && !val->isKeyset());
  if (val->empty()) return aempty();
  auto r        = Type { BSArrN };
  r.m_data.aval = val;
  r.m_dataTag   = DataTag::ArrLikeVal;
  return r;
}

Type aempty()         { return Type { BSArrE }; }
Type sempty()         { return sval(s_empty.get()); }
Type counted_aempty() { return Type { BCArrE }; }
Type some_aempty()    { return Type { BArrE }; }

Type vec_val(SArray val) {
  assert(val->isStatic());
  assert(val->isVecArray());
  if (val->empty()) return vec_empty();
  auto r        = Type { BSVecN };
  r.m_data.aval = val;
  r.m_dataTag   = DataTag::ArrLikeVal;
  return r;
}

Type vec_empty()         { return Type { BSVecE }; }
Type counted_vec_empty() { return Type { BCVecE }; }
Type some_vec_empty()    { return Type { BVecE }; }

Type packedn_impl(trep bits, Type t) {
  auto r = Type { bits };
  construct_inner(r.m_data.packedn, std::move(t));
  r.m_dataTag = DataTag::ArrLikePackedN;
  return r;
}

Type packed_impl(trep bits, std::vector<Type> elems) {
  assert(!elems.empty());
  auto r = Type { bits };
  construct_inner(r.m_data.packed, std::move(elems));
  r.m_dataTag = DataTag::ArrLikePacked;
  return r;
}

Type vec_n(Type ty) {
  return packedn_impl(BVecN, std::move(ty));
}

Type svec_n(Type ty) {
  return packedn_impl(BSVecN, std::move(ty));
}

Type vec(std::vector<Type> elems) {
  return packed_impl(BVecN, std::move(elems));
}

Type cvec(std::vector<Type> elems) {
  return packed_impl(BCVecN, std::move(elems));
}

Type svec(std::vector<Type> elems) {
  return packed_impl(BSVecN, std::move(elems));
}

Type dict_val(SArray val) {
  assert(val->isStatic());
  assert(val->isDict());
  if (val->empty()) return dict_empty();
  auto r        = Type { BSDictN };
  r.m_data.aval = val;
  r.m_dataTag   = DataTag::ArrLikeVal;
  return r;
}

Type dict_empty()         { return Type { BSDictE }; }
Type counted_dict_empty() { return Type { BCDictE }; }
Type some_dict_empty()    { return Type { BDictE }; }

Type dict_n(Type k, Type v) {
  return mapn_impl(BDictN, std::move(k), std::move(v));
}

Type sdict_n(Type k, Type v) {
  return mapn_impl(BSDictN, std::move(k), std::move(v));
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
Type counted_keyset_empty() { return Type { BCKeysetE }; }
Type some_keyset_empty()    { return Type { BKeysetE }; }

Type keyset_n(Type kv) {
  assert(kv.subtypeOf(TArrKey));
  auto v = kv;
  return mapn_impl(BKeysetN, std::move(kv), std::move(v));
}

Type skeyset_n(Type kv) {
  assert(kv.subtypeOf(TUncArrKey));
  auto v = kv;
  return mapn_impl(BSKeysetN, std::move(kv), std::move(v));
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
  assert(t.subtypeOf(TInitCell));
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
  return t.subtypeOf(TOptArr) && is_specialized_array_like(t);
}

bool is_specialized_vec(const Type& t) {
  return t.subtypeOf(TOptVec) && is_specialized_array_like(t);
}

bool is_specialized_dict(const Type& t) {
  return t.subtypeOf(TOptDict) && is_specialized_array_like(t);
}

bool is_specialized_keyset(const Type& t) {
  return t.subtypeOf(TOptKeyset) && is_specialized_array_like(t);
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
                           const Type& opt_e,
                           const Type& opt) {
  // If b isn't the same kind of array-like, we'll have to treat it as
  // a union of two separate types
  if (!b.subtypeOf(opt)) return TBottom;
  auto const bits = combine_arr_like_bits(spec_a.m_bits, b.m_bits);
  if (!is_specialized_array_like(b)) {
    // We can keep a's specialization if b is an empty array-like
    // or a nullable empty array-like.
    if (b.subtypeOf(opt_e)) {
      spec_a.m_bits = bits;
      return std::move(spec_a);
    }
    // otherwise drop the specialized bits
    return Type { bits };
  }

  DEBUG_ONLY auto const shouldBeOpt = is_opt(spec_a) || is_opt(b);
  auto const t = Type::unionArrLike(std::move(spec_a), std::move(b));
  assert(!shouldBeOpt || is_opt(t));
  return t;
}

Type arr_packed(std::vector<Type> elems) {
  return packed_impl(BArrN, std::move(elems));
}

Type sarr_packed(std::vector<Type> elems) {
  return packed_impl(BSArrN, std::move(elems));
}

Type carr_packed(std::vector<Type> elems) {
  return packed_impl(BCArrN, std::move(elems));
}

Type arr_packedn(Type t) {
  return packedn_impl(BArrN, std::move(t));
}

Type sarr_packedn(Type t) {
  return packedn_impl(BSArrN, std::move(t));
}

Type carr_packedn(Type t) {
  return packedn_impl(BCArrN, std::move(t));
}

Type map_impl(trep bits, MapElems m) {
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
    return packed_impl(bits, std::move(elems));
  }

  auto r = Type { bits };
  construct_inner(r.m_data.map, std::move(m));
  r.m_dataTag = DataTag::ArrLikeMap;
  return r;
}

Type arr_map(MapElems m) {
  return map_impl(BArrN, std::move(m));
}

Type sarr_map(MapElems m) {
  return map_impl(BSArrN, std::move(m));
}

Type mapn_impl(trep bits, Type k, Type v) {
  assert(k.subtypeOf(TArrKey));

  // A MapN cannot have a constant key (because that can actually make it be a
  // subtype of Map sometimes), so if it does, make it a Map instead.
  if (auto val = tv(k)) {
    MapElems m;
    m.emplace_back(*val, std::move(v));
    return map_impl(bits, std::move(m));
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

Type arr_mapn(Type k, Type v) {
  return mapn_impl(BArrN, std::move(k), std::move(v));
}

Type sarr_mapn(Type k, Type v) {
  return mapn_impl(BSArrN, std::move(k), std::move(v));
}

Type carr_mapn(Type k, Type v) {
  return mapn_impl(BCArrN, std::move(k), std::move(v));
}

Type opt(Type t) {
  assert(canBeOptional(t.m_bits));
  auto ret = t;
  ret.m_bits = static_cast<trep>(ret.m_bits | BInitNull);
  return ret;
}

Type unopt(Type t) {
  assert(is_opt(t));
  t.m_bits = static_cast<trep>(t.m_bits & ~BInitNull);
  assert(!is_opt(t));
  return t;
}

bool is_opt(const Type& t) {
  if (t.m_bits == BInitNull) return false;
  if (!t.couldBe(TInitNull)) return false;
  auto const nonNullBits = static_cast<trep>(t.m_bits & ~BInitNull);
  return isPredefined(nonNullBits) && canBeOptional(nonNullBits);
}

bool is_specialized_obj(const Type& t) {
  return t.m_dataTag == DataTag::Obj;
}

bool is_specialized_cls(const Type& t) {
  return t.m_dataTag == DataTag::Cls;
}

Type objcls(const Type& t) {
  if (t.subtypeOf(TObj) && is_specialized_obj(t)) {
    auto const d = dobj_of(t);
    return d.type == DObj::Exact ? clsExact(d.cls) : subCls(d.cls);
  }
  return TCls;
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
  auto val = true;
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

folly::Optional<Cell> tv(const Type& t) {
  assert(t.checkInvariants());

  switch (t.m_bits) {
  case BUninit:      return make_tv<KindOfUninit>();
  case BInitNull:    return make_tv<KindOfNull>();
  case BTrue:        return make_tv<KindOfBoolean>(true);
  case BFalse:       return make_tv<KindOfBoolean>(false);
  case BCArrE:       /* fallthrough */
  case BSArrE:       return make_tv<KindOfPersistentArray>(staticEmptyArray());
  case BCVecE:
  case BSVecE:
    return make_tv<KindOfPersistentVec>(staticEmptyVecArray());
  case BCDictE:
  case BSDictE:
    return make_tv<KindOfPersistentDict>(staticEmptyDictArray());
  case BCKeysetE:
  case BSKeysetE:
    return make_tv<KindOfPersistentKeyset>(staticEmptyKeysetArray());
  default:
    if (is_opt(t)) {
      break;
    }
    switch (t.m_dataTag) {
    case DataTag::Int:    return make_tv<KindOfInt64>(t.m_data.ival);
    case DataTag::Dbl:    return make_tv<KindOfDouble>(t.m_data.dval);
    case DataTag::Str:    return make_tv<KindOfPersistentString>(t.m_data.sval);
    case DataTag::ArrLikeVal:
      if ((t.m_bits & BArrN) == t.m_bits) {
        return make_tv<KindOfPersistentArray>(
          const_cast<ArrayData*>(t.m_data.aval)
        );
      }
      if ((t.m_bits & BVecN) == t.m_bits) {
        return make_tv<KindOfPersistentVec>(
          const_cast<ArrayData*>(t.m_data.aval)
        );
      }
      if ((t.m_bits & BDictN) == t.m_bits) {
        return make_tv<KindOfPersistentDict>(
          const_cast<ArrayData*>(t.m_data.aval)
        );
      }
      if ((t.m_bits & BKeysetN) == t.m_bits) {
        return make_tv<KindOfPersistentKeyset>(
          const_cast<ArrayData*>(t.m_data.aval)
        );
      }
      break;
    case DataTag::ArrLikeMap:
      if ((t.m_bits & BDictN) == t.m_bits) {
        return fromTypeMap<DictInit>(t.m_data.map->map);
      } else if ((t.m_bits & BKeysetN) == t.m_bits) {
        return fromTypeMap<KeysetInit>(t.m_data.map->map);
      } else if ((t.m_bits & BArrN) == t.m_bits) {
        return fromTypeMap<MixedArrayInit>(t.m_data.map->map);
      }
      break;
    case DataTag::ArrLikePacked:
      if ((t.m_bits & BVecN) == t.m_bits) {
        return fromTypeVec<VecArrayInit>(t.m_data.packed->elems);
      } else if ((t.m_bits & BDictN) == t.m_bits) {
        return fromTypeVec<DictInit>(t.m_data.packed->elems);
      } else if ((t.m_bits & BArrN) == t.m_bits) {
        return fromTypeVec<PackedArrayInit>(t.m_data.packed->elems);
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

  return folly::none;
}

Type type_of_istype(IsTypeOp op) {
  switch (op) {
  case IsTypeOp::Uninit: return TUninit;
  case IsTypeOp::Null:   return TNull;
  case IsTypeOp::Bool:   return TBool;
  case IsTypeOp::Int:    return TInt;
  case IsTypeOp::Dbl:    return TDbl;
  case IsTypeOp::Str:    return TStr;
  case IsTypeOp::Arr:    return TArr;
  case IsTypeOp::Vec:    return TVec;
  case IsTypeOp::Dict:   return TDict;
  case IsTypeOp::Keyset: return TKeyset;
  case IsTypeOp::Obj:    return TObj;
  case IsTypeOp::Scalar: always_assert(0);
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

  case KindOfPersistentArray:
  case KindOfArray:
    always_assert(cell.m_data.parr->isStatic());
    always_assert(cell.m_data.parr->isPHPArray());
    return aval(cell.m_data.parr);

  case KindOfRef:
  case KindOfObject:
  case KindOfResource:
    break;
  }
  always_assert(0 && "reference counted/class type in from_cell");
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
  case KindOfPersistentArray:
  case KindOfArray:    return TArr;
  case KindOfRef:      return TRef;
  case KindOfObject:   return TObj;
  case KindOfResource: return TRes;
    break;
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

  if (!strcasecmp(p, "HH\\resource")) return union_of(ret, TRes);
  if (!strcasecmp(p, "HH\\bool"))     return union_of(ret, TBool);
  if (!strcasecmp(p, "HH\\int"))      return union_of(ret, TInt);
  if (!strcasecmp(p, "HH\\float"))    return union_of(ret, TDbl);
  if (!strcasecmp(p, "HH\\num"))      return union_of(ret, TNum);
  if (!strcasecmp(p, "HH\\string"))   return union_of(ret, TStr);
  if (!strcasecmp(p, "HH\\dict"))     return union_of(ret, TDict);
  if (!strcasecmp(p, "HH\\vec"))      return union_of(ret, TVec);
  if (!strcasecmp(p, "HH\\keyset"))   return union_of(ret, TKeyset);
  if (!strcasecmp(p, "HH\\varray"))   return union_of(ret, TArr);
  if (!strcasecmp(p, "HH\\darray"))   return union_of(ret, TArr);
  if (!strcasecmp(p, "HH\\varray_or_darray"))   return union_of(ret, TArr);
  if (!strcasecmp(p, "array"))        return union_of(ret, TArr);
  if (!strcasecmp(p, "HH\\arraykey")) return union_of(ret, TArrKey);
  if (!strcasecmp(p, "HH\\mixed"))    return TInitGen;

  // It might be an object, or we might want to support type aliases in HNI at
  // some point.  For now just be conservative.
  return TGen;
}

Type Type::unionArrLike(const Type& a, const Type& b) {
  assert(!a.subtypeOf(b));
  assert(!b.subtypeOf(a));

  auto ret = Type{};
  auto const newBits = combine_arr_like_bits(a.m_bits, b.m_bits);

  return a.dualDispatchDataFn(b, DualDispatchUnion{ newBits });
}

Type union_of(Type a, Type b) {
  if (a.subtypeOf(b)) return b;
  if (b.subtypeOf(a)) return a;

  /*
   * We need to check this before specialized objects, including the case where
   * one of them was TInitNull, because otherwise we'll go down the
   * is_specialized_obj paths and lose the wait handle information.
   */
  if (is_specialized_wait_handle(a)) {
    if (is_specialized_wait_handle(b)) {
      *a.m_data.dobj.whType.mutate() |= *b.m_data.dobj.whType;
      return a;
    }
    if (b == TInitNull) return opt(a);
  }
  if (is_specialized_wait_handle(b)) {
    if (a == TInitNull) return opt(b);
  }

  // When both types are strict subtypes of TObj or TOptObj or both
  // are strict subtypes of TCls we look for a common ancestor if one
  // exists.
  if (is_specialized_obj(a) && is_specialized_obj(b)) {
    auto keepOpt = is_opt(a) || is_opt(b);
    auto t = a.m_data.dobj.cls.commonAncestor(dobj_of(b).cls);
    // We need not to distinguish between Obj<=T and Obj=T, and always
    // return an Obj<=Ancestor, because that is the single type that
    // includes both children.
    if (t) return keepOpt ? opt(subObj(*t)) : subObj(*t);
    return keepOpt ? TOptObj : TObj;
  }
  if (a.strictSubtypeOf(TCls) && b.strictSubtypeOf(TCls)) {
    auto t = a.m_data.dcls.cls.commonAncestor(dcls_of(b).cls);
    // Similar to above, this must always return an Obj<=Ancestor.
    return t ? subCls(*t) : TCls;
  }

  if (is_specialized_array(a)) {
    auto t = spec_array_like_union(a, b, TOptArrE, TOptArr);
    if (t != TBottom) return t;
  } else if (is_specialized_array(b)) {
    auto t = spec_array_like_union(b, a, TOptArrE, TOptArr);
    if (t != TBottom) return t;
  }

  if (is_specialized_vec(a)) {
    auto t = spec_array_like_union(a, b, TOptVecE, TOptVec);
    if (t != TBottom) return t;
  } else if (is_specialized_vec(b)) {
    auto t = spec_array_like_union(b, a, TOptVecE, TOptVec);
    if (t != TBottom) return t;
  }

  if (is_specialized_dict(a)) {
    auto t = spec_array_like_union(a, b, TOptDictE, TOptDict);
    if (t != TBottom) return t;
  } else if (is_specialized_dict(b)) {
    auto t = spec_array_like_union(b, a, TOptDictE, TOptDict);
    if (t != TBottom) return t;
  }

  if (is_specialized_keyset(a)) {
    auto t = spec_array_like_union(a, b, TOptKeysetE, TOptKeyset);
    if (t != TBottom) return t;
  } else if (is_specialized_keyset(b)) {
    auto t = spec_array_like_union(b, a, TOptKeysetE, TOptKeyset);
    if (t != TBottom) return t;
  }

  if (is_ref_with_inner(a) && is_ref_with_inner(b)) {
    return ref_to(union_of(*a.m_data.inner, *b.m_data.inner));
  }

#define X(y) if (a.subtypeOf(y) && b.subtypeOf(y)) return y;
  X(TInt)
  X(TDbl)
  X(TSStr)
  X(TCStr)
  X(TSArr)
  X(TCArr)
  X(TArrE)
  X(TArrN)
  X(TObj)
  X(TCls)
  X(TNull)
  X(TBool)
  X(TNum)
  X(TStr)
  X(TArr)

  X(TSVec)
  X(TCVec)
  X(TVecE)
  X(TVecN)
  X(TVec)
  X(TSDict)
  X(TCDict)
  X(TDictE)
  X(TDictN)
  X(TDict)
  X(TSKeyset)
  X(TCKeyset)
  X(TKeysetE)
  X(TKeysetN)
  X(TKeyset)

  X(TUncArrKey)
  X(TArrKey)

  /*
   * Merging option types tries to preserve subtype information where it's
   * possible.  E.g. if you union InitNull and Obj<=Foo, we want OptObj<=Foo to
   * be the result.
   */
  if (a == TInitNull && canBeOptional(b.m_bits)) return opt(b);
  if (b == TInitNull && canBeOptional(a.m_bits)) return opt(a);

  // Optional types where the non-Null part is already a union or can
  // have a value need to be manually tried (e.g. if we are merging
  // TOptTrue and TOptFalse, we want TOptBool, or merging TOptInt=1
  // and TOptInt=2 should give us TOptInt).
  X(TOptBool)
  X(TOptInt)
  X(TOptDbl)
  X(TOptNum)
  X(TOptSStr)
  X(TOptStr)
  X(TOptArrN)
  X(TOptArrE)
  X(TOptSArr)
  X(TOptCArr)
  X(TOptArr)
  X(TOptObj)

  X(TOptSVec)
  X(TOptCVec)
  X(TOptVecE)
  X(TOptVecN)
  X(TOptVec)
  X(TOptSDict)
  X(TOptCDict)
  X(TOptDictE)
  X(TOptDictN)
  X(TOptDict)
  X(TOptSKeyset)
  X(TOptCKeyset)
  X(TOptKeysetE)
  X(TOptKeysetN)
  X(TOptKeyset)

  X(TOptUncArrKey)
  X(TOptArrKey)

  X(TInitPrim)
  X(TPrim)
  X(TInitUnc)
  X(TUnc)
  X(TInitCell)
  X(TCell)
  X(TInitGen)
  X(TGen)
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

Type widening_union(const Type& a, const Type& b) {
  if (a == b) return a;

  auto const u = union_of(a, b);

  // Currently the only types in our typesystem that have infinitely
  // growing chains of union_of are specialized arrays.
  if (!is_specialized_array_like(a) || !is_specialized_array_like(b)) {
    return u;
  }

  // This (throwing away the data) is overly conservative, but works
  // for now.
  return is_specialized_array_like(u) ? Type { u.m_bits } : u;
}

Type stack_flav(Type a) {
  if (a.subtypeOf(TUninit))   return TUninit;
  if (a.subtypeOf(TInitCell)) return TInitCell;
  if (a.subtypeOf(TRef))      return TRef;
  if (a.subtypeOf(TGen))      return TGen;
  always_assert(0 && "stack_flav passed invalid type");
}

Type loosen_staticness(Type t) {
  auto const check = [&](trep a){
    if (t.m_bits & a) t.m_bits = static_cast<trep>(t.m_bits | a);
  };
  // Need to remove any constant value from a string because a TStr cannot have
  // one.
  if (t.couldBe(TStr)) t |= TStr;
  check(BArrE);
  check(BArrN);
  check(BVecE);
  check(BVecN);
  check(BDictE);
  check(BDictN);
  check(BKeysetE);
  check(BKeysetN);
  return t;
}

Type loosen_arrays(Type a) {
  if (a.couldBe(TArr))    a |= TArr;
  if (a.couldBe(TVec))    a |= TVec;
  if (a.couldBe(TDict))   a |= TDict;
  if (a.couldBe(TKeyset)) a |= TKeyset;
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
  if (t.couldBe(TFalse) || t.couldBe(TTrue)) t |= TBool;
  return t;
}

Type loosen_emptiness(Type t) {
  auto const check = [&](trep a){
    if (t.m_bits & a) t.m_bits = static_cast<trep>(t.m_bits | a);
  };
  check(BSArr);
  check(BCArr);
  check(BSVec);
  check(BCVec);
  check(BSDict);
  check(BCDict);
  check(BSKeyset);
  check(BCKeyset);
  return t;
}

Type loosen_all(Type t) {
  return loosen_staticness(loosen_emptiness(loosen_values(std::move(t))));
}

Type add_nonemptiness(Type t) {
  auto const check = [&](trep a, trep b){
    if (t.m_bits & a) t.m_bits = static_cast<trep>(t.m_bits | b);
  };
  check(BSArrE, BSArrN);
  check(BCArrE, BCArrN);
  check(BSVecE, BSVecN);
  check(BCVecE, BCVecN);
  check(BSDictE, BSDictN);
  check(BCDictE, BCDictN);
  check(BSKeysetE, BSKeysetN);
  check(BCKeysetE, BCKeysetN);
  return t;
}

Type remove_uninit(Type t) {
  assert(t.subtypeOf(TCell));
  if (!t.couldBe(TUninit))  return t;
  if (t.subtypeOf(TUninit)) return TBottom;
  if (t.subtypeOf(TNull))   return TInitNull;
  if (t.subtypeOf(TPrim))   return TInitPrim;
  if (t.subtypeOf(TUnc))    return TInitUnc;
  return TInitCell;
}

//////////////////////////////////////////////////////////////////////

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

  if (keyTy.subtypeOf(TOptInt)) {
    if (keyTy.subtypeOf(TInt)) {
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

  if (keyTy.subtypeOf(TOptStr)) {
    if (keyTy.subtypeOf(TStr)) {
      if (keyTy.strictSubtypeOf(TStr) && keyTy.m_dataTag == DataTag::Str) {
        int64_t i;
        if (keyTy.m_data.sval->isStrictlyInteger(i)) {
          ret.i = i;
          ret.type = ival(i);
          ret.mayThrow = RuntimeOption::EvalHackArrCompatNotices;
        } else {
          ret.s = keyTy.m_data.sval;
          ret.type = keyTy;
        }
        return ret;
      }
      // Might stay a string or become an integer. The effective type is
      // uncounted if the string is static.
      ret.type = keyTy.subtypeOf(TSStr) ? TUncArrKey : TArrKey;
      ret.mayThrow = RuntimeOption::EvalHackArrCompatNotices;
      return ret;
    }
    // If we have an OptStr with a value, we can at least exclude the
    // possibility of integer-like strings by looking at that value.
    // But we can't use the value itself, because if it is null the key
    // will act like the empty string.  In that case, the code uses the
    // static empty string, so if it was an OptCStr it needs to
    // incorporate SStr, but an OptSStr can stay as SStr.
    if (keyTy.strictSubtypeOf(TOptStr) && keyTy.m_dataTag == DataTag::Str) {
      int64_t ignore;
      if (!keyTy.m_data.sval->isStrictlyInteger(ignore)) {
        ret.type = keyTy.strictSubtypeOf(TOptSStr) ? TSStr : TStr;
        ret.mayThrow = RuntimeOption::EvalHackArrCompatNotices;
        return ret;
      }
    }
    // An optional string is fine because a null will just become the empty
    // string. So, if the string is static, the effective type is uncounted
    // still. The effective type is ArrKey because it might become an integer.
    ret.type = keyTy.subtypeOf(TOptSStr) ? TUncArrKey : TArrKey;
    ret.mayThrow = RuntimeOption::EvalHackArrCompatNotices;
    return ret;
  }

  if (keyTy.subtypeOf(TOptArrKey)) {
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
  if (keyTy.subtypeOf(TNum)) {
    ret.type = TInt;
    ret.mayThrow = RuntimeOption::EvalHackArrCompatNotices;
    return ret;
  }
  if (keyTy.subtypeOf(TNull)) {
    ret.s = s_empty.get();
    ret.type = sempty();
    ret.mayThrow = RuntimeOption::EvalHackArrCompatNotices;
    return ret;
  }
  if (keyTy.subtypeOf(TRes)) {
    ret.type = TInt;
    ret.mayThrow = RuntimeOption::EvalHackArrCompatNotices;
    return ret;
  }
  if (keyTy.subtypeOf(TTrue)) {
    ret.i = 1;
    ret.type = ival(1);
    ret.mayThrow = RuntimeOption::EvalHackArrCompatNotices;
    return ret;
  }
  if (keyTy.subtypeOf(TFalse)) {
    ret.i = 0;
    ret.type = ival(0);
    ret.mayThrow = RuntimeOption::EvalHackArrCompatNotices;
    return ret;
  }
  if (keyTy.subtypeOf(TBool)) {
    ret.type = TInt;
    ret.mayThrow = RuntimeOption::EvalHackArrCompatNotices;
    return ret;
  }
  if (keyTy.subtypeOf(TPrim)) {
    ret.type = TUncArrKey;
    ret.mayThrow = RuntimeOption::EvalHackArrCompatNotices;
    return ret;
  }

  // The key could be something strange like an array or an object. This can
  // raise warnings, so always assume it may throw. Keep the type as-is so that
  // we can detect this case at the point of the set.

  if (!keyTy.subtypeOf(TInitCell)) {
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
  auto const isPhpArray = aval.subtypeOf(TOptArr);
  if (key.i) {
    if (auto const r = ad->rval(*key.i)) {
      return { from_cell(r.tv()), true };
    }
    return { isPhpArray ? TInitNull : TBottom, false };
  } else if (key.s) {
    if (auto const r = ad->rval(*key.s)) {
      return { from_cell(r.tv()), true };
    }
    return { isPhpArray ? TInitNull : TBottom, false };
  }

  auto const couldBeInt = key.type.couldBe(TInt);
  auto const couldBeStr = key.type.couldBe(TStr);
  auto ty = isPhpArray ? TInitNull : TBottom;
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
  auto const isPhpArray = map.subtypeOf(TOptArr);
  if (auto const k = key.tv()) {
    auto r = map.m_data.map->map.find(*k);
    if (r != map.m_data.map->map.end()) return { r->second, true };
    return { isPhpArray ? TInitNull : TBottom, false };
  }
  auto couldBeInt = key.type.couldBe(TInt);
  auto couldBeStr = key.type.couldBe(TStr);
  auto ty = isPhpArray ? TInitNull : TBottom;
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
  auto const isPhpArray = pack.subtypeOf(TOptArr);
  if (key.i) {
    if (*key.i >= 0 && *key.i < pack.m_data.packed->elems.size()) {
      return { pack.m_data.packed->elems[*key.i], true };
    }
    return { isPhpArray ? TInitNull : TBottom, false };
  } else if (!key.type.couldBe(TInt)) {
    return { isPhpArray ? TInitNull : TBottom, false };
  }
  auto ret = packed_values(*pack.m_data.packed);
  if (isPhpArray) {
    ret |= TInitNull;
  }
  return { ret, false };
}

/*
 * Extract pack[key] when pack is known to have DataTag::ArrLikePackedN
 */
std::pair<Type,bool> arr_packedn_elem(const Type& pack, const ArrKey& key) {
  assert(pack.m_dataTag == DataTag::ArrLikePackedN);
  auto const isPhpArray = (pack.m_bits & BOptArr) == pack.m_bits;
  if (key.s || !key.type.couldBe(TInt) || (key.i && *key.i < 0)) {
    return {isPhpArray ? TInitNull : TBottom, false};
  }

  if (isPhpArray) {
    return {union_of(pack.m_data.packedn->type, TInitNull), false};
  }

  return {pack.m_data.packedn->type, false};
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
  assert(key.type.subtypeOf(TArrKey));

  auto const isPhpArray = (pack.m_bits & BOptArr) == pack.m_bits;
  auto const isVecArray = (pack.m_bits & BOptVec) == pack.m_bits;
  auto& ty = pack.m_data.packedn.mutate()->type;
  ty |= val;
  if (key.i) {
    if (maybeEmpty ? isPhpArray && !*key.i :
        !*key.i || (isPhpArray && *key.i == 1)) {
      // The key is known to be in range - its still a packedn
      return true;
    }
  }

  if (!isVecArray) {
    pack = mapn_impl(pack.m_bits, union_of(TInt, key.type), std::move(ty));
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
                 const Type& val) {
  assert(map.m_dataTag == DataTag::ArrLikeMap);
  assert(key.type.subtypeOf(TArrKey));

 if (auto const k = key.tv()) {
    auto r = map.m_data.map.mutate()->map.emplace_back(*k, val);
    // if the element existed, and was a ref, its still a ref after
    // assigning to it
    if (!r.second && r.first->second.subtypeOf(TInitCell)) {
      map.m_data.map.mutate()->map.update(r.first, val);
    }
    return true;
  }
  auto mkv = map_key_values(*map.m_data.map);
  map = mapn_impl(map.m_bits,
                  union_of(std::move(mkv.first), key.type),
                  union_of(std::move(mkv.second), val));
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
                    const Type& val) {
  assert(pack.m_dataTag == DataTag::ArrLikePacked);
  assert(key.type.subtypeOf(TArrKey));

  auto const isVecArray = pack.subtypeOf(TOptVec);
  if (key.i) {
    if (*key.i >= 0) {
      if (*key.i < pack.m_data.packed->elems.size()) {
        auto& current = pack.m_data.packed.mutate()->elems[*key.i];
        // if the element was a ref, its still a ref after assigning to it
        if (current.subtypeOf(TInitCell)) {
          current = val;
        }
        return true;
      }
      if (!isVecArray && *key.i == pack.m_data.packed->elems.size()) {
        pack.m_data.packed.mutate()->elems.push_back(val);
        return true;
      }
    }
    if (isVecArray) {
      pack = TBottom;
      return false;
    }
  }

  if (!isVecArray) {
    if (auto const v = key.tv()) {
      MapElems elems;
      auto idx = int64_t{0};
      for (auto const& t : pack.m_data.packed->elems) {
        elems.emplace_back(make_tv<KindOfInt64>(idx++), t);
      }
      elems.emplace_back(*v, val);
      pack = map_impl(pack.m_bits, std::move(elems));
      return true;
    }

    auto ty = union_of(packed_values(*pack.m_data.packed), val);
    pack = mapn_impl(pack.m_bits, union_of(TInt, key.type), std::move(ty));
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
  assert(key.type.subtypeOf(TArrKey));
  auto& mapn = *map.m_data.mapn.mutate();
  mapn.val |= val;
  mapn.key |= key.type;
  assert(map.checkInvariants());
  return true;
}

Type arr_map_newelem(Type& map, const Type& val) {
  assert(map.m_dataTag == DataTag::ArrLikeMap);
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
  return ival(lastK + 1);
}

std::pair<Type, bool> array_like_elem(const Type& arr, const ArrKey& key) {
  const bool maybeEmpty = arr.m_bits & BArrLikeE;
  const bool mustBeStatic = (arr.m_bits & BSArrLike) == arr.m_bits;

  auto const isPhpArray = arr.subtypeOf(TOptArr);
  if (!(arr.m_bits & BArrLikeN)) {
    assert(maybeEmpty);
    return { isPhpArray ? TInitNull : TBottom, false };
  }
  auto ret = [&]() -> std::pair<Type, bool> {
    switch (arr.m_dataTag) {
    case DataTag::Str:
    case DataTag::Obj:
    case DataTag::Int:
    case DataTag::Dbl:
    case DataTag::Cls:
    case DataTag::RefInner:
      not_reached();

    case DataTag::None:
      return { mustBeStatic ? TInitUnc : TInitCell, false };

    case DataTag::ArrLikeVal:
      return arr_val_elem(arr, key);

    case DataTag::ArrLikePacked:
      return arr_packed_elem(arr, key);

    case DataTag::ArrLikePackedN:
      return arr_packedn_elem(arr, key);

    case DataTag::ArrLikeMap:
      return arr_map_elem(arr, key);

    case DataTag::ArrLikeMapN:
      if (isPhpArray) {
        return { union_of(arr.m_data.mapn->val, TInitNull), false};
      } else {
        return { arr.m_data.mapn->val, false };
      }
    }
    not_reached();
  }();

  if (key.mayThrow) ret.second = false;

  if (!ret.first.subtypeOf(TInitCell)) {
    ret.first = TInitCell;
  }

  if (maybeEmpty) {
    if (isPhpArray) ret.first |= TInitNull;
    ret.second = false;
  }

  return ret;
}

Type array_elem(const Type& arr, const Type& undisectedKey) {
  assert(arr.subtypeOf(TArr));
  auto const key = disect_array_key(undisectedKey);
  return array_like_elem(arr, key).first;
}

/*
 * Note: for now we're merging counted arrays into whatever type it used to
 * have in the following set functions, and returning arr_*'s in some cases
 * where we could know it was a carr_*.
 *
 * To be able to assume it is actually counted it if used to be static, we need
 * to add code checking for keys that are one of the "illegal offset type" of
 * keys.
 *
 * A similar issue applies if you want to take out emptiness when a set occurs.
 * If the key could be an illegal key type, the array may remain empty.
 */

std::pair<Type,bool> array_like_set(Type arr,
                                    const ArrKey& key,
                                    const Type& valIn) {

  const bool maybeEmpty = arr.m_bits & BArrLikeE;
  const bool isVector   = arr.m_bits & BOptVec;
  const bool isPhpArray = arr.m_bits & BOptArr;
  const bool validKey   = key.type.subtypeOf(isVector ? TInt : TArrKey);

  trep bits = combine_arr_like_bits(arr.m_bits, BArrLikeN);
  if (validKey) bits = static_cast<trep>(bits & ~BArrLikeE);

  auto const fixRef  = !isPhpArray && valIn.couldBe(TRef);
  auto const noThrow = !fixRef && validKey && !key.mayThrow;
  auto const& val    = fixRef ? TInitCell : valIn;
  // We don't want to store types more general than TArrKey into specialized
  // array type keys. If the key was strange (array or object), it will be more
  // general than TArrKey (this is needed so we can set validKey above), so
  // force it to TArrKey.
  auto const& fixedKey = validKey
    ? key
    : []{ ArrKey key; key.type = TArrKey; key.mayThrow = true; return key; }();

  if (!(arr.m_bits & BArrLikeN)) {
    assert(maybeEmpty);
    if (isVector) return { TBottom, false };
    if (fixedKey.i && !*fixedKey.i) {
      return { packed_impl(bits, { val }), noThrow };
    }
    if (auto const k = fixedKey.tv()) {
      MapElems m;
      m.emplace_back(*k, val);
      return { map_impl(bits, std::move(m)), noThrow };
    }
    return { mapn_impl(bits, fixedKey.type, val), noThrow };
  }

  auto emptyHelper = [&] (const Type& inKey,
                          const Type& inVal) -> std::pair<Type,bool> {
    return { mapn_impl(bits,
                       union_of(inKey, fixedKey.type),
                       union_of(inVal, val)), noThrow };
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
    return { std::move(arr), false };

  case DataTag::ArrLikeVal:
    if (maybeEmpty && !isVector) {
      auto kv = val_key_values(arr.m_data.aval);
      return emptyHelper(kv.first, kv.second);
    } else {
      if (auto d = toDArrLikePacked(arr.m_data.aval)) {
        return array_like_set(packed_impl(bits, std::move(d->elems)),
                              key, valIn);
      }
      assert(!isVector);
      // We know its not packed, so this should always succeed
      auto d = toDArrLikeMap(arr.m_data.aval);
      return array_like_set(map_impl(bits, std::move(d->map)),
                            key, valIn);
    }

  case DataTag::ArrLikePacked:
    if (maybeEmpty && !isVector) {
      auto ty = packed_values(*arr.m_data.packed);
      return emptyHelper(TInt, packed_values(*arr.m_data.packed));
    } else {
      auto const inRange = arr_packed_set(arr, fixedKey, val);
      return { std::move(arr), inRange && noThrow };
    }

  case DataTag::ArrLikePackedN:
    if (maybeEmpty && !isVector) {
      return emptyHelper(TInt, arr.m_data.packedn->type);
    } else {
      auto const inRange = arr_packedn_set(arr, fixedKey, val, false);
      return { std::move(arr), inRange && noThrow };
    }

  case DataTag::ArrLikeMap:
    assert(!isVector);
    if (maybeEmpty) {
      auto mkv = map_key_values(*arr.m_data.map);
      return emptyHelper(std::move(mkv.first), std::move(mkv.second));
    } else {
      auto const inRange = arr_map_set(arr, fixedKey, val);
      return { std::move(arr), inRange && noThrow };
    }

  case DataTag::ArrLikeMapN:
    assert(!isVector);
    if (maybeEmpty) {
      return emptyHelper(arr.m_data.mapn->key, arr.m_data.mapn->val);
    } else {
      auto const inRange = arr_mapn_set(arr, fixedKey, val);
      return { std::move(arr), inRange && noThrow };
    }
  }

  not_reached();
}

std::pair<Type, bool> array_set(Type arr,
                                const Type& undisectedKey,
                                const Type& val) {
  assert(arr.subtypeOf(TArr));

  // Unless you know an array can't cow, you don't know if the TRef
  // will stay a TRef or turn back into a TInitCell.  Generally you
  // want a TInitGen.
  always_assert((val == TBottom || !val.subtypeOf(TRef)) &&
         "You probably don't want to put Ref types into arrays ...");

  auto const key = disect_array_key(undisectedKey);
  assert(key.type != TBottom);
  return array_like_set(std::move(arr), key, val);
}

std::pair<Type,Type> array_like_newelem(Type arr, const Type& val) {

  if (arr.m_bits & BOptKeyset) {
    auto const key = disect_strict_key(val);
    if (key.type == TBottom) return { TBottom, TInitCell };
    return { array_like_set(std::move(arr), key, key.type).first, val };
  }

  const bool maybeEmpty = arr.m_bits & BArrLikeE;
  const bool isVector = arr.m_bits & BOptVec;

  trep bits = combine_arr_like_bits(arr.m_bits, BArrLikeN);
  bits = static_cast<trep>(bits & ~BArrLikeE);

  if (!(arr.m_bits & BArrLikeN)) {
    assert(maybeEmpty);
    return { packed_impl(bits, { val }), ival(0) };
  }

  auto emptyHelper = [&] (const Type& inKey,
                          const Type& inVal) -> std::pair<Type,Type> {
    if (isVector) {
      assert(inKey.subtypeOf(TInt));
      return { packedn_impl(bits, union_of(inVal, val)), TInt };
    }

    return { mapn_impl(bits,
                       union_of(inKey, TInt),
                       union_of(inVal, val)), TInt };
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
          packed_impl(bits, std::move(d->elems)), val);
      }
      assert(!isVector);
      // We know its not packed, so this should always succeed.
      auto d = toDArrLikeMap(arr.m_data.aval);
      return array_like_newelem(map_impl(bits, std::move(d->map)), val);
    }

  case DataTag::ArrLikePacked:
    if (maybeEmpty) {
      return emptyHelper(TInt, packed_values(*arr.m_data.packed));
    } else {
      arr.m_bits = bits;
      auto len = arr.m_data.packed->elems.size();
      arr.m_data.packed.mutate()->elems.push_back(val);
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
    if (maybeEmpty) {
      auto mkv = map_key_values(*arr.m_data.map);
      return emptyHelper(mkv.first, mkv.second);
    } else {
      arr.m_bits = bits;
      auto const idx = arr_map_newelem(arr, val);
      return { std::move(arr), idx };
    }

  case DataTag::ArrLikeMapN:
    assert(!isVector);
    if (maybeEmpty) {
      return emptyHelper(arr.m_data.mapn->key, arr.m_data.mapn->val);
    }
    return { mapn_impl(bits,
                       union_of(arr.m_data.mapn->key, TInt),
                       union_of(arr.m_data.mapn->val, val)),
             TInt };
  }

  not_reached();
}

std::pair<Type,Type> array_newelem_key(const Type& arr, const Type& val) {
  assert(arr.subtypeOf(TArr));

  // Unless you know an array can't cow, you don't know if the TRef
  // will stay a TRef or turn back into a TInitCell.  Generally you
  // want a TInitGen.
  always_assert((val == TBottom || !val.subtypeOf(TRef)) &&
         "You probably don't want to put Ref types into arrays ...");

  return array_like_newelem(arr, val);
}

Type array_newelem(const Type& arr, const Type& val) {
  return array_newelem_key(arr, val).first;
}

std::pair<Type,Type> iter_types(const Type& iterable) {
  // Optional types are okay here because a null will not set any locals.
  if (!iterable.subtypeOfAny(TOptArr, TOptVec, TOptDict, TOptKeyset)) {
    return { TInitCell, TInitCell };
  }

  if (!is_specialized_array_like(iterable)) {
    if (iterable.subtypeOf(TOptSVec))    return { TInt, TInitUnc };
    if (iterable.subtypeOf(TOptSDict))   return { TUncArrKey, TInitUnc };
    if (iterable.subtypeOf(TOptSKeyset)) return { TUncArrKey, TUncArrKey };
    if (iterable.subtypeOf(TOptSArr))    return { TUncArrKey, TInitUnc };

    if (iterable.subtypeOf(TOptVec))     return { TInt, TInitCell };
    if (iterable.subtypeOf(TOptDict))    return { TArrKey, TInitCell };
    if (iterable.subtypeOf(TOptKeyset))  return { TArrKey, TArrKey };
    if (iterable.subtypeOf(TOptArr))     return { TArrKey, TInitCell };

    always_assert(false);
  }

  // Note: we don't need to handle possible emptiness explicitly,
  // because if the array was empty we won't ever pull anything out
  // while iterating.

  switch (iterable.m_dataTag) {
  case DataTag::None:
  case DataTag::Str:
  case DataTag::Obj:
  case DataTag::Int:
  case DataTag::Dbl:
  case DataTag::Cls:
  case DataTag::RefInner:
    always_assert(0);
  case DataTag::ArrLikeVal:
    return val_key_values(iterable.m_data.aval);
  case DataTag::ArrLikePacked:
    return { TInt, packed_values(*iterable.m_data.packed) };
  case DataTag::ArrLikePackedN:
    return { TInt, iterable.m_data.packedn->type };
  case DataTag::ArrLikeMap:
    return map_key_values(*iterable.m_data.map);
  case DataTag::ArrLikeMapN:
    return { iterable.m_data.mapn->key, iterable.m_data.mapn->val };
  }

  not_reached();
}

bool could_run_destructor(const Type& t) {
  if (t.couldBe(TObj)) return true;

  auto const couldBeArr =
    t.couldBe(TCArrN) || t.couldBe(TCVecN) || t.couldBe(TCDictN);

  if (t.couldBe(TRef)) {
    if (!couldBeArr && is_ref_with_inner(t)) {
      return could_run_destructor(*t.m_data.inner);
    }
    return true;
  }

  if (!couldBeArr) return false;

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
      if (could_run_destructor(e)) return true;
    }
    return false;
  case DataTag::ArrLikePackedN:
    return could_run_destructor(t.m_data.packedn->type);
  case DataTag::ArrLikeMap:
    for (auto const& kv : t.m_data.map->map) {
      if (could_run_destructor(kv.second)) return true;
    }
    return false;
  case DataTag::ArrLikeMapN:
    return could_run_destructor(t.m_data.mapn->val);
  }

  not_reached();
}

//////////////////////////////////////////////////////////////////////

ArrKey disect_vec_key(const Type& keyTy) {
  auto ret = ArrKey{};

  if (!keyTy.couldBe(TInt)) {
    ret.type = TBottom;
    ret.mayThrow = true;
    return ret;
  }

  // If the key is null, we'll throw, so we can assume its not for the effective
  // type (and mark it as potentially throwing). We check for this explicitly
  // here rather than falling through so we can take advantage of something like
  // ?Int=123.
  if (keyTy.subtypeOf(TOptInt)) {
    if (keyTy.m_dataTag == DataTag::Int) {
      ret.i = keyTy.m_data.ival;
      ret.type = ival(*ret.i);
    } else {
      ret.type = TInt;
    }
    ret.mayThrow = !keyTy.subtypeOf(TInt);
    return ret;
  }

  // Something else. Same reasoning as above. We can assume its an TInt because
  // it will throw otherwise.
  ret.type = TInt;
  ret.mayThrow = true;
  return ret;
}

std::pair<Type, bool> vec_elem(const Type& vec, const Type& undisectedKey) {
  auto const key = disect_vec_key(undisectedKey);
  if (key.type == TBottom) return {TBottom, false};
  return array_like_elem(vec, key);
}

std::pair<Type, bool>
vec_set(Type vec, const Type& undisectedKey, const Type& val) {
  if (!val.couldBe(TInitCell)) return {TBottom, false};

  auto const key = disect_vec_key(undisectedKey);
  if (key.type == TBottom) return {TBottom, false};

  return array_like_set(std::move(vec), key, val);
}

std::pair<Type,Type> vec_newelem(Type vec, const Type& val) {
  return array_like_newelem(std::move(vec),
                            val.subtypeOf(TInitCell) ? val : TInitCell);
}

//////////////////////////////////////////////////////////////////////

ArrKey disect_strict_key(const Type& keyTy) {
  auto ret = ArrKey{};

  if (!keyTy.couldBe(TArrKey)) {
    ret.type = TBottom;
    ret.mayThrow = true;
    return ret;
  }

  // If the key is null, we'll throw, so we can assume its not for the effective
  // type (but mark it as potentially throwing).
  if (keyTy.subtypeOf(TOptArrKey)) {
    if (keyTy.m_dataTag == DataTag::Int) {
      ret.i = keyTy.m_data.ival;
    } else if (keyTy.m_dataTag == DataTag::Str) {
      ret.s = keyTy.m_data.sval;
    }
    ret.type = is_opt(keyTy) ? unopt(keyTy) : keyTy;
    ret.mayThrow = !keyTy.subtypeOf(TArrKey);
    return ret;
  }

  // Something else. Same reasoning as above. We can assume its an TArrKey
  // because it will throw otherwise.
  ret.type = TArrKey;
  ret.mayThrow = true;
  return ret;
}

std::pair<Type, bool> dict_elem(const Type& dict, const Type& undisectedKey) {
  auto const key = disect_strict_key(undisectedKey);
  if (key.type == TBottom) return {TBottom, false};
  return array_like_elem(dict, key);
}

std::pair<Type, bool>
dict_set(Type dict, const Type& undisectedKey, const Type& val) {
  if (!val.couldBe(TInitCell)) return {TBottom, false};

  auto const key = disect_strict_key(undisectedKey);
  if (key.type == TBottom) return {TBottom, false};

  return array_like_set(std::move(dict), key, val);
}

std::pair<Type,Type> dict_newelem(Type dict, const Type& val) {
  return array_like_newelem(std::move(dict),
                            val.subtypeOf(TInitCell) ? val : TInitCell);
}

//////////////////////////////////////////////////////////////////////

std::pair<Type, bool>
keyset_elem(const Type& keyset, const Type& undisectedKey) {
  auto const key = disect_strict_key(undisectedKey);
  if (key.type == TBottom) return {TBottom, false};
  return array_like_elem(keyset, key);
}

std::pair<Type, bool> keyset_set(Type /*keyset*/, const Type&, const Type&) {
  // The set operation on keysets is not allowed.
  return {TBottom, false};
}

std::pair<Type,Type> keyset_newelem(Type keyset, const Type& val) {
  return array_like_newelem(std::move(keyset), val);
}

//////////////////////////////////////////////////////////////////////

RepoAuthType make_repo_type_arr(ArrayTypeTable::Builder& arrTable,
                                const Type& t) {
  auto const emptiness  = TArrE.couldBe(t) ? RepoAuthType::Array::Empty::Maybe
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
      // TODO(#4205897): we need to use this before it's worth putting
      // in the repo.
      if (false) {
        return arrTable.packedn(
          emptiness,
          make_repo_type(arrTable, t.m_data.packedn->type)
        );
      }
      return nullptr;
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
    if (t.subtypeOf(TSArr))    return RepoAuthType::Tag::SArr;
    if (t.subtypeOf(TArr))     return RepoAuthType::Tag::Arr;
    if (t.subtypeOf(TOptSArr)) return RepoAuthType::Tag::OptSArr;
    if (t.subtypeOf(TOptArr))  return RepoAuthType::Tag::OptArr;
    not_reached();
  }();

  return RepoAuthType { tag, arr };
}

RepoAuthType make_repo_type(ArrayTypeTable::Builder& arrTable, const Type& t) {
  assert(!t.couldBe(TCls));
  assert(!t.subtypeOf(TBottom));
  using T = RepoAuthType::Tag;

  if (t.strictSubtypeOf(TObj) || (is_opt(t) && t.strictSubtypeOf(TOptObj))) {
    auto const dobj = dobj_of(t);
    auto const tag =
      is_opt(t)
        ? (dobj.type == DObj::Exact ? T::OptExactObj : T::OptSubObj)
        : (dobj.type == DObj::Exact ? T::ExactObj    : T::SubObj);
    return RepoAuthType { tag, dobj.cls.name() };
  }

  if (t.strictSubtypeOf(TArr) || (is_opt(t) && t.strictSubtypeOf(TOptArr))) {
    return make_repo_type_arr(arrTable, t);
  }

#define X(x) if (t.subtypeOf(T##x)) return RepoAuthType{T::x};
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
  X(UncArrKey)
  X(ArrKey)
  X(OptUncArrKey)
  X(OptArrKey)
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

}}
