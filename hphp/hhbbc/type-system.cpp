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
#include "hphp/hhbbc/type-system.h"

#include <type_traits>
#include <cmath>
#include <algorithm>
#include <iterator>

#include <folly/Optional.h>
#include <folly/Traits.h>
#include <folly/Hash.h>

#include "hphp/runtime/base/repo-auth-type.h"
#include "hphp/runtime/base/repo-auth-type-array.h"
#include "hphp/runtime/base/array-iterator.h"
#include "hphp/runtime/base/mixed-array.h"
#include "hphp/runtime/base/packed-array.h"
#include "hphp/runtime/base/set-array.h"
#include "hphp/hhbbc/index.h"

namespace HPHP { namespace HHBBC {

TRACE_SET_MOD(hhbbc);

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
  assert((a & B) == a || (a & OptB) == a);
  assert((b & B) == b || (b & OptB) == b);
  auto const combined = static_cast<trep>(a | b);
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

//////////////////////////////////////////////////////////////////////

/*
 * The following functions make DArr* structs out of static arrays, to
 * simplify implementing some of the type system operations on them.
 *
 * When they return folly::none it is not a conservative thing: it
 * implies the array is definitely not packed, packedN, struct-like,
 * etc (we use this to return false in couldBe).
 */

folly::Optional<DArrPacked> toDArrPacked(SArray ar) {
  assert(!ar->empty());

  std::vector<Type> elems;
  auto idx = size_t{0};
  for (ArrayIter iter(ar); iter; ++iter, ++idx) {
    auto const key = *iter.first().asTypedValue();
    if (key.m_type != KindOfInt64) return folly::none;
    if (key.m_data.num != idx)     return folly::none;
    elems.push_back(
      from_cell(*iter.secondRef().asTypedValue())
    );
  }

  return DArrPacked { std::move(elems) };
}

folly::Optional<DArrPackedN> toDArrPackedN(SArray ar) {
  assert(!ar->empty());

  auto t = TBottom;
  auto idx = size_t{0};
  for (ArrayIter iter(ar); iter; ++iter, ++idx) {
    auto const key = *iter.first().asTypedValue();
    if (key.m_type != KindOfInt64) return folly::none;
    if (key.m_data.num != idx)     return folly::none;
    t = union_of(t, from_cell(*iter.secondRef().asTypedValue()));
  }

  return DArrPackedN { std::move(t) };
}

folly::Optional<DArrStruct> toDArrStruct(SArray ar) {
  assert(!ar->empty());

  auto map = StructMap{};
  for (ArrayIter iter(ar); iter; ++iter) {
    auto const key = *iter.first().asTypedValue();
    if (!isStringType(key.m_type)) return folly::none;
    map[key.m_data.pstr] = from_cell(*iter.secondRef().asTypedValue());
  }

  return DArrStruct { std::move(map) };
}

DArrMapN toDArrMapN(SArray ar) {
  assert(!ar->empty());

  auto k = TBottom;
  auto v = TBottom;
  for (ArrayIter iter(ar); iter; ++iter) {
    auto const key = *iter.first().asTypedValue();
    k = union_of(k, from_cell(key));
    v = union_of(v, from_cell(*iter.secondRef().asTypedValue()));
  }

  return DArrMapN { k, v };
}

DVec toDVec(SArray ar) {
  assert(ar->isVecArray());
  assert(!ar->empty());

  auto v = TBottom;
  for (ArrayIter iter(ar); iter; ++iter) {
    v = union_of(v, from_cell(*iter.secondRef().asTypedValue()));
  }

  return DVec { v, int64_t(ar->size()) };
}

DDict toDDict(SArray ar) {
  assert(ar->isDict());
  assert(!ar->empty());

  auto k = TBottom;
  auto v = TBottom;
  for (ArrayIter iter(ar); iter; ++iter) {
    auto const key = *iter.first().asTypedValue();
    k = union_of(k, from_cell(key));
    v = union_of(v, from_cell(*iter.secondRef().asTypedValue()));
  }

  return DDict { k, v };
}

DKeyset toDKeyset(SArray ar) {
  assert(ar->isKeyset());
  assert(!ar->empty());

  auto kv = TBottom;
  for (ArrayIter iter(ar); iter; ++iter) {
    auto const key = *iter.first().asTypedValue();
    kv = union_of(kv, from_cell(key));
  }

  return DKeyset { kv };
}

//////////////////////////////////////////////////////////////////////

bool subtypePacked(const DArrPacked& a, const DArrPacked& b) {
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

bool subtypeStruct(const DArrStruct& a, const DArrStruct& b) {
  if (a.map.size() != b.map.size()) return false;
  auto aIt = begin(a.map);
  auto bIt = begin(b.map);
  for (; aIt != end(a.map); ++aIt, ++bIt) {
    if (aIt->first != bIt->first) return false;
    if (!aIt->second.subtypeOf(bIt->second)) return false;
  }
  return true;
}

bool couldBePacked(const DArrPacked& a, const DArrPacked& b) {
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

bool couldBeStruct(const DArrStruct& a, const DArrStruct& b) {
  if (a.map.size() != b.map.size()) return false;
  auto aIt = begin(a.map);
  auto bIt = begin(b.map);
  for (; aIt != end(a.map); ++aIt, ++bIt) {
    if (aIt->first != bIt->first) return false;
    if (!aIt->second.couldBe(bIt->second)) return false;
  }
  return true;
}

Type struct_values(const DArrStruct& a) {
  auto ret = TBottom;
  for (auto& kv : a.map) ret = union_of(ret, kv.second);
  return ret;
}

Type packed_values(const DArrPacked& a) {
  auto ret = TBottom;
  for (auto& e : a.elems) ret = union_of(ret, e);
  return ret;
}

//////////////////////////////////////////////////////////////////////

/*
 * Helper for dealing with disjointDataFn's---most of them are commutative.
 * This shuffles values to the right in a canonical order to need less
 * overloads.
 */
template<class InnerFn>
struct Commute : InnerFn {
  using result_type = typename InnerFn::result_type;

  using InnerFn::operator();

  template<class B>
  result_type operator()(SArray a, const B& b) const {
    return InnerFn::operator()(b, a);
  }

  template<class B>
  result_type operator()(const DArrStruct& a, const B& b) const {
    return InnerFn::operator()(b, a);
  }

  template<class B>
  result_type operator()(const DArrPackedN& a, const B& b) const {
    return InnerFn::operator()(b, a);
  }

  template<class B>
  result_type operator()(const DArrMapN& a, const B& b) const {
    return InnerFn::operator()(b, a);
  }
};

struct DisjointEqImpl {
  using result_type = bool;

  bool operator()() const { return false; }

  bool operator()(const DArrPacked& a, SArray b) const {
    if (a.elems.size() != b->size()) return false;
    auto const p = toDArrPacked(b);
    return p && a.elems == p->elems;
  }

  bool operator()(const DArrStruct& a, SArray b) const {
    if (a.map.size() != b->size()) return false;
    auto const p = toDArrStruct(b);
    return p && a.map == p->map;
  }

  bool operator()(const DArrPackedN& a, SArray b) const {
    return false;
  }
  bool operator()(const DArrMapN& a, SArray b) const {
    return false;
  }
  bool operator()(const DArrPacked& a, const DArrPackedN& b) const {
    return false;
  }
  bool operator()(const DArrPacked& a, const DArrStruct& b) const {
    return false;
  }
  bool operator()(const DArrPacked&, const DArrMapN&) const {
    return false;
  }
  bool operator()(const DArrPackedN& a, const DArrStruct& b) const {
    return false;
  }
  bool operator()(const DArrPackedN&, const DArrMapN&) const {
    return false;
  }
  bool operator()(const DArrStruct&, const DArrMapN&) const {
    return false;
  }
};

struct DisjointCouldBeImpl {
  using result_type = bool;

  bool operator()() const { return false; }

  bool operator()(const DArrPacked& a, SArray b) const {
    if (a.elems.size() != b->size()) return false;
    auto const p = toDArrPacked(b);
    return p && couldBePacked(a, *p);
  }

  bool operator()(const DArrStruct& a, SArray b) const {
    if (a.map.size() != b->size()) return false;
    auto const p = toDArrStruct(b);
    return p && couldBeStruct(a, *p);
  }

  bool operator()(const DArrPackedN& a, SArray b) const {
    auto const p = toDArrPackedN(b);
    return p && a.type.couldBe(p->type);
  }

  bool operator()(const DArrMapN& a, SArray b) const {
    auto const p = toDArrMapN(b);
    return p.key.couldBe(a.key) && p.val.couldBe(a.val);
  }

  bool operator()(const DArrPacked& a, const DArrPackedN& b) const {
    for (auto& t : a.elems) {
      if (!t.couldBe(b.type)) return false;
    }
    return true;
  }

  bool operator()(const DArrPackedN& a, const DArrMapN& b) const {
    return TInt.couldBe(b.key) && a.type.couldBe(b.val);
  }

  bool operator()(const DArrStruct& a, const DArrMapN& b) const {
    if (!TSStr.couldBe(b.key)) return false;
    for (auto& kv : a.map) {
      if (!kv.second.couldBe(b.val)) return false;
    }
    return true;
  }

  bool operator()(const DArrPacked&, const DArrStruct&) const {
    return false;
  }
  bool operator()(const DArrPacked&, const DArrMapN&) const {
    return false;
  }
  bool operator()(const DArrPackedN&, const DArrStruct&) const {
    return false;
  }
};

// The countedness or possible-emptiness of the arrays is handled
// outside of this function, so it's ok to just return TArr from all
// of these here.

struct ArrUnionImpl {
  using result_type = Type;

  Type operator()() const { not_reached(); }

  Type operator()(const DArrPacked& a, const DArrPacked& b) const {
    if (a.elems.size() != b.elems.size()) {
      return arr_packedn(union_of(packed_values(a), packed_values(b)));
    }
    auto ret = a.elems;
    for (auto i = size_t{0}; i < a.elems.size(); ++i) {
      ret[i] = union_of(ret[i], b.elems[i]);
    }
    return arr_packed(std::move(ret));
  }

  Type operator()(const DArrPacked& a, const DArrPackedN& b) const {
    return arr_packedn(union_of(packed_values(a), b.type));
  }

  Type operator()(const DArrStruct& a, const DArrStruct& b) const {
    auto to_map = [&] {
      auto all = union_of(struct_values(a), struct_values(b));
      return arr_mapn(TSStr, std::move(all));
    };

    /*
     * With the current meaning of structs, if the keys are different, we can't
     * do anything better than going to a map type.  The reason for this is
     * that our struct types currently are implying the presence of all the
     * keys in the struct (it might be worth adding some more types for struct
     * subtyping to handle this better.)
     */
    if (a.map.size() != b.map.size()) return to_map();

    auto retStruct = StructMap{};
    auto aIt = begin(a.map);
    auto bIt = begin(b.map);
    for (; aIt != end(a.map); ++aIt, ++bIt) {
      if (aIt->first != bIt->first) return to_map();
      retStruct[aIt->first] = union_of(aIt->second, bIt->second);
    }
    return arr_struct(std::move(retStruct));
  }

  Type operator()(SArray a, SArray b) const {
    assert(a != b); // Should've been handled earlier in union_of.
    {
      auto const p1 = toDArrPacked(a);
      auto const p2 = toDArrPacked(b);
      if (p1 && p2) {
        return (*this)(*p1, *p2);
      }
      if (p1) {
        if (auto const p2n = toDArrPackedN(b)) {
          return (*this)(*p1, *p2n);
        }
      }
      if (p2) {
        if (auto const p1n = toDArrPackedN(a)) {
          return (*this)(*p2, *p1n);
        }
      }
    }
    {
      auto const s1 = toDArrStruct(a);
      auto const s2 = toDArrStruct(b);
      if (s1 && s2) return (*this)(*s1, *s2);
    }
    return (*this)(toDArrMapN(a), b);
  }

  Type operator()(const DArrPacked& a, SArray b) const {
    auto p = toDArrPacked(b);
    if (!p) return TArr;
    return (*this)(a, *p);
  }

  Type operator()(const DArrPackedN& a, SArray b) const {
    auto const p = toDArrPackedN(b);
    if (!p) return TArr;
    return arr_packedn(union_of(a.type, p->type));
  }

  Type operator()(const DArrStruct& a, SArray b) const {
    auto const p = toDArrStruct(b);
    if (!p) return TArr;
    return (*this)(a, *p);
  }

  Type operator()(const DArrMapN& a, SArray b) const {
    auto const p = toDArrMapN(b);
    return arr_mapn(union_of(a.key, p.key), union_of(a.val, p.val));
  }

  Type operator()(const DArrPacked& a, const DArrStruct& b) const {
    return arr_mapn(union_of(TInt, TSStr),
                    union_of(packed_values(a), struct_values(b)));
  }

  Type operator()(const DArrPacked& a, const DArrMapN& b) const {
    return arr_mapn(union_of(b.key, TInt),
                    union_of(packed_values(a), b.val));
  }

  Type operator()(const DArrPackedN& a, const DArrStruct& b) const {
    return arr_mapn(union_of(TInt, TSStr),
                    union_of(a.type, struct_values(b)));
  }

  Type operator()(const DArrPackedN& a, const DArrMapN& b) const {
    return arr_mapn(union_of(TInt, b.key),
                    union_of(a.type, b.val));
  }

  Type operator()(const DArrStruct& a, const DArrMapN& b) const {
    return arr_mapn(union_of(TSStr, b.key),
                    union_of(struct_values(a), b.val));
  }
};

/*
 * Subtype is not a commutative relation, so this is the only
 * disjointDataFn helper that doesn't use Commute<>.
 */
struct DisjointSubtype {
  using result_type = bool;

  bool operator()() const { return false; }

  bool operator()(const DArrStruct& a, SArray b) const {
    if (a.map.size() != b->size()) return false;
    auto const p = toDArrStruct(b);
    return p && subtypeStruct(a, *p);
  }

  bool operator()(SArray a, const DArrStruct& b) const {
    if (a->size() != b.map.size()) return false;
    auto const p = toDArrStruct(a);
    return p && subtypeStruct(*p, b);
  }

  bool operator()(SArray a, const DArrPacked& b) const {
    if (a->size() != b.elems.size()) return false;
    auto const p = toDArrPacked(a);
    return p && subtypePacked(*p, b);
  }

  bool operator()(const DArrPacked& a, SArray b) const {
    if (a.elems.size() != b->size()) return false;
    auto const p = toDArrPacked(b);
    return p && subtypePacked(a, *p);
  }

  bool operator()(const DArrPackedN& a, const DArrMapN& b) const {
    return TInt.subtypeOf(b.key) && a.type.subtypeOf(b.val);
  }

  bool operator()(const DArrPacked& a, const DArrMapN& b) const {
    if (!TInt.subtypeOf(b.key)) return false;
    for (auto& v : a.elems) {
      if (!v.subtypeOf(b.val)) return false;
    }
    return true;
  }

  bool operator()(const DArrStruct& a, const DArrMapN& b) const {
    if (!TSStr.subtypeOf(b.key)) return false;
    for (auto& kv : a.map) {
      if (!kv.second.subtypeOf(b.val)) return false;
    }
    return true;
  }

  bool operator()(SArray a, const DArrMapN& b) const {
    auto const p = toDArrMapN(a);
    return p.key.subtypeOf(b.key) && p.val.subtypeOf(b.val);
  }

  bool operator()(const DArrPacked& a, const DArrPackedN& b) const {
    for (auto& t : a.elems) {
      if (!t.subtypeOf(b.type)) return false;
    }
    return true;
  }

  bool operator()(SArray a, const DArrPackedN& b) const {
    auto p = toDArrPackedN(a);
    return p && p->type.subtypeOf(b.type);
  }

  bool operator()(const DArrPackedN& a, const DArrPacked& b) const {
    return false;
  }
  bool operator()(const DArrPackedN& a, SArray b) const {
    return false;
  }
  bool operator()(const DArrStruct& a, const DArrPacked& b) const {
    return false;
  }
  bool operator()(const DArrStruct& a, const DArrPackedN& b) const {
    return false;
  }
  bool operator()(const DArrPacked& a, const DArrStruct& b) const {
    return false;
  }
  bool operator()(const DArrPackedN& a, const DArrStruct& b) const {
    return false;
  }
  bool operator()(const DArrMapN&, const DArrPackedN&) const {
    return false;
  }
  bool operator()(const DArrMapN&, const DArrPacked&) const {
    return false;
  }
  bool operator()(const DArrMapN&, const DArrStruct&) const {
    return false;
  }
  bool operator()(const DArrMapN&, SArray) const {
    return false;
  }
};

using DisjointEq      = Commute<DisjointEqImpl>;
using DisjointCouldBe = Commute<DisjointCouldBeImpl>;
using ArrUnion        = Commute<ArrUnionImpl>;

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
  case DataTag::Str:    m_data.sval = o.m_data.sval; return;
  case DataTag::VecVal:
  case DataTag::DictVal:
  case DataTag::KeysetVal:
  case DataTag::ArrVal: m_data.aval = o.m_data.aval; return;
  case DataTag::Int:    m_data.ival = o.m_data.ival; return;
  case DataTag::Dbl:    m_data.dval = o.m_data.dval; return;
  case DataTag::Obj:    new (&m_data.dobj) DObj(o.m_data.dobj); return;
  case DataTag::Cls:    new (&m_data.dcls) DCls(o.m_data.dcls); return;
  case DataTag::RefInner:
    new (&m_data.inner) copy_ptr<Type>(o.m_data.inner);
    return;
  case DataTag::ArrPacked:
    new (&m_data.apacked) copy_ptr<DArrPacked>(o.m_data.apacked);
    return;
  case DataTag::ArrPackedN:
    new (&m_data.apackedn) copy_ptr<DArrPackedN>(o.m_data.apackedn);
    return;
  case DataTag::ArrStruct:
    new (&m_data.astruct) copy_ptr<DArrStruct>(o.m_data.astruct);
    return;
  case DataTag::ArrMapN:
    new (&m_data.amapn) copy_ptr<DArrMapN>(o.m_data.amapn);
    return;
  case DataTag::Vec:
    new (&m_data.vec) copy_ptr<DVec>(o.m_data.vec);
    return;
  case DataTag::Dict:
    new (&m_data.dict) copy_ptr<DDict>(o.m_data.dict);
    return;
  case DataTag::Keyset:
    new (&m_data.keyset) copy_ptr<DKeyset>(o.m_data.keyset);
    return;
  }
  not_reached();
}

Type::Type(Type&& o) noexcept
  : m_bits(o.m_bits)
  , m_dataTag(o.m_dataTag)
{
  SCOPE_EXIT { assert(checkInvariants());
               assert(o.checkInvariants()); };
  using std::move;
  o.m_dataTag = DataTag::None;
  switch (m_dataTag) {
  case DataTag::None:   return;
  case DataTag::Str:    m_data.sval = o.m_data.sval; return;
  case DataTag::VecVal:
  case DataTag::DictVal:
  case DataTag::KeysetVal:
  case DataTag::ArrVal: m_data.aval = o.m_data.aval; return;
  case DataTag::Int:    m_data.ival = o.m_data.ival; return;
  case DataTag::Dbl:    m_data.dval = o.m_data.dval; return;
  case DataTag::Obj:    new (&m_data.dobj) DObj(move(o.m_data.dobj)); return;
  case DataTag::Cls:    new (&m_data.dcls) DCls(move(o.m_data.dcls)); return;
  case DataTag::RefInner:
    new (&m_data.inner) copy_ptr<Type>(move(o.m_data.inner));
    return;
  case DataTag::ArrPacked:
    new (&m_data.apacked) copy_ptr<DArrPacked>(move(o.m_data.apacked));
    return;
  case DataTag::ArrPackedN:
    new (&m_data.apackedn) copy_ptr<DArrPackedN>(move(o.m_data.apackedn));
    return;
  case DataTag::ArrStruct:
    new (&m_data.astruct) copy_ptr<DArrStruct>(move(o.m_data.astruct));
    return;
  case DataTag::ArrMapN:
    new (&m_data.amapn) copy_ptr<DArrMapN>(move(o.m_data.amapn));
    return;
  case DataTag::Vec:
    new (&m_data.vec) copy_ptr<DVec>(move(o.m_data.vec));
    return;
  case DataTag::Dict:
    new (&m_data.dict) copy_ptr<DDict>(move(o.m_data.dict));
    return;
  case DataTag::Keyset:
    new (&m_data.keyset) copy_ptr<DKeyset>(move(o.m_data.keyset));
    return;
  }
  not_reached();
}

Type& Type::operator=(const Type& o) noexcept {
  SCOPE_EXIT { assert(checkInvariants()); };
  this->~Type();
  new (this) Type(o);
  return *this;
}

Type& Type::operator=(Type&& o) noexcept {
  SCOPE_EXIT { assert(checkInvariants());
               assert(o.checkInvariants()); };
  this->~Type();
  new (this) Type(std::move(o));
  return *this;
}

Type::~Type() noexcept {
  assert(checkInvariants());

  switch (m_dataTag) {
  case DataTag::None:
  case DataTag::Str:
  case DataTag::VecVal:
  case DataTag::DictVal:
  case DataTag::KeysetVal:
  case DataTag::ArrVal:
  case DataTag::Int:
  case DataTag::Dbl:
    return;
  case DataTag::RefInner:
    m_data.inner.~copy_ptr<Type>();
    return;
  case DataTag::Obj:
    m_data.dobj.~DObj();
    return;
  case DataTag::Cls:
    m_data.dcls.~DCls();
    return;
  case DataTag::ArrPacked:
    m_data.apacked.~copy_ptr<DArrPacked>();
    return;
  case DataTag::ArrPackedN:
    m_data.apackedn.~copy_ptr<DArrPackedN>();
    return;
  case DataTag::ArrStruct:
    m_data.astruct.~copy_ptr<DArrStruct>();
    return;
  case DataTag::ArrMapN:
    m_data.amapn.~copy_ptr<DArrMapN>();
    return;
  case DataTag::Vec:
    m_data.vec.~copy_ptr<DVec>();
    return;
  case DataTag::Dict:
    m_data.dict.~copy_ptr<DDict>();
    return;
  case DataTag::Keyset:
    m_data.keyset.~copy_ptr<DKeyset>();
    return;
  }
  not_reached();
}

//////////////////////////////////////////////////////////////////////

template<class Ret, class T, class Function>
struct Type::DJHelperFn {
  template<class Y> Ret operator()(const Y& y) const { return f(t, y); }
  Ret operator()(const T& t) const { not_reached(); }
  Ret operator()() const { return f(); }
  Function f;
  const T& t;
};

template<class Ret, class T, class Function>
Type::DJHelperFn<Ret,T,Function> Type::djbind(const Function& f,
                                              const T& t) const {
  return { f, t };
}

// Dispatcher for the second argument for disjointDataFn.
template<class Ret, class T, class Function>
Ret Type::dj2nd(const Type& o, DJHelperFn<Ret,T,Function> f) const {
  switch (o.m_dataTag) {
  case DataTag::None:       not_reached();
  case DataTag::Str:        return f();
  case DataTag::Obj:        return f();
  case DataTag::Int:        return f();
  case DataTag::Dbl:        return f();
  case DataTag::Cls:        return f();
  case DataTag::RefInner:   return f();
  case DataTag::VecVal:     return f();
  case DataTag::DictVal:    return f();
  case DataTag::KeysetVal:  return f();
  case DataTag::Vec:        return f();
  case DataTag::Dict:       return f();
  case DataTag::Keyset:     return f();
  case DataTag::ArrVal:     return f(o.m_data.aval);
  case DataTag::ArrPacked:  return f(*o.m_data.apacked);
  case DataTag::ArrPackedN: return f(*o.m_data.apackedn);
  case DataTag::ArrStruct:  return f(*o.m_data.astruct);
  case DataTag::ArrMapN:    return f(*o.m_data.amapn);
  }
  not_reached();
}

/*
 * Dispatch an operation on data when this->m_dataTag is different
 * from o.m_dataTag.  Right now these operations only need to do work
 * for array shapes, so the default case (zero-arg call) is applied to
 * most other types.
 *
 * See the disjoint data function objects above.
 */
template<class Function>
typename Function::result_type
Type::disjointDataFn(const Type& o, Function f) const {
  assert(m_dataTag != o.m_dataTag);
  using R = typename Function::result_type;
  switch (m_dataTag) {
  case DataTag::None:       not_reached();
  case DataTag::Str:        return f();
  case DataTag::Obj:        return f();
  case DataTag::Int:        return f();
  case DataTag::Dbl:        return f();
  case DataTag::Cls:        return f();
  case DataTag::RefInner:   return f();
  case DataTag::VecVal:     return f();
  case DataTag::DictVal:    return f();
  case DataTag::KeysetVal:  return f();
  case DataTag::Vec:        return f();
  case DataTag::Dict:       return f();
  case DataTag::Keyset:     return f();
  case DataTag::ArrVal:     return dj2nd(o, djbind<R>(f, m_data.aval));
  case DataTag::ArrPacked:  return dj2nd(o, djbind<R>(f, *m_data.apacked));
  case DataTag::ArrPackedN: return dj2nd(o, djbind<R>(f, *m_data.apackedn));
  case DataTag::ArrStruct:  return dj2nd(o, djbind<R>(f, *m_data.astruct));
  case DataTag::ArrMapN:    return dj2nd(o, djbind<R>(f, *m_data.amapn));
  }
  not_reached();
}

//////////////////////////////////////////////////////////////////////

bool Type::hasData() const {
  return m_dataTag != DataTag::None;
}

bool Type::equivData(const Type& o) const {
  if (m_dataTag != o.m_dataTag) {
    return disjointDataFn(o, DisjointEq{});
  }

  switch (m_dataTag) {
  case DataTag::None:
    not_reached();
  case DataTag::Str:
    return m_data.sval == o.m_data.sval;
  case DataTag::VecVal:
  case DataTag::DictVal:
  case DataTag::KeysetVal:
  case DataTag::ArrVal:
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
  case DataTag::ArrPacked:
    return m_data.apacked->elems == o.m_data.apacked->elems;
  case DataTag::ArrPackedN:
    return m_data.apackedn->type == o.m_data.apackedn->type;
  case DataTag::ArrStruct:
    return m_data.astruct->map == o.m_data.astruct->map;
  case DataTag::ArrMapN:
    return m_data.amapn->key == o.m_data.amapn->key &&
           m_data.amapn->val == o.m_data.amapn->val;
  case DataTag::Vec:
    return m_data.vec->val == o.m_data.vec->val &&
           m_data.vec->len == o.m_data.vec->len;
  case DataTag::Dict:
    return m_data.dict->key == o.m_data.dict->key &&
           m_data.dict->val == o.m_data.dict->val;
  case DataTag::Keyset:
    return m_data.keyset->keyval == o.m_data.keyset->keyval;
  }
  not_reached();
}

bool Type::subtypeData(const Type& o) const {
  if (m_dataTag != o.m_dataTag) {
    return disjointDataFn(o, DisjointSubtype{});
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
  case DataTag::VecVal:
  case DataTag::DictVal:
  case DataTag::KeysetVal:
  case DataTag::ArrVal:
  case DataTag::Int:
  case DataTag::Dbl:
  case DataTag::None:
    return equivData(o);
  case DataTag::RefInner:
    return m_data.inner->subtypeOf(*o.m_data.inner);
  case DataTag::ArrPacked:
    return subtypePacked(*m_data.apacked, *o.m_data.apacked);
  case DataTag::ArrPackedN:
    return m_data.apackedn->type.subtypeOf(o.m_data.apackedn->type);
  case DataTag::ArrStruct:
    return subtypeStruct(*m_data.astruct, *o.m_data.astruct);
  case DataTag::ArrMapN:
    return m_data.amapn->key.subtypeOf(o.m_data.amapn->key) &&
           m_data.amapn->val.subtypeOf(o.m_data.amapn->val);
  case DataTag::Vec:
    return m_data.vec->val.subtypeOf(o.m_data.vec->val) &&
           m_data.vec->len == o.m_data.vec->len;
  case DataTag::Dict:
    return m_data.dict->key.subtypeOf(o.m_data.dict->key) &&
           m_data.dict->val.subtypeOf(o.m_data.dict->val);
  case DataTag::Keyset:
    return m_data.keyset->keyval.subtypeOf(o.m_data.keyset->keyval);
  }
  not_reached();
}

bool Type::couldBeData(const Type& o) const {
  if (m_dataTag != o.m_dataTag) {
    return disjointDataFn(o, DisjointCouldBe{});
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
  case DataTag::VecVal:
  case DataTag::DictVal:
  case DataTag::KeysetVal:
  case DataTag::ArrVal:
    return m_data.aval == o.m_data.aval;
  case DataTag::Str:
    return m_data.sval == o.m_data.sval;
  case DataTag::Int:
    return m_data.ival == o.m_data.ival;
  case DataTag::Dbl:
    return m_data.dval == o.m_data.dval;
  case DataTag::ArrPacked:
    return couldBePacked(*m_data.apacked, *o.m_data.apacked);
  case DataTag::ArrPackedN:
    return m_data.apackedn->type.couldBe(o.m_data.apackedn->type);
  case DataTag::ArrStruct:
    return couldBeStruct(*m_data.astruct, *o.m_data.astruct);
  case DataTag::ArrMapN:
    return m_data.amapn->key.couldBe(o.m_data.amapn->key) &&
           m_data.amapn->val.couldBe(o.m_data.amapn->val);
  case DataTag::Vec:
    return m_data.vec->val.couldBe(o.m_data.vec->val) &&
      (!m_data.vec->len || !o.m_data.vec->len ||
       *m_data.vec->len == *o.m_data.vec->len);
  case DataTag::Dict:
    return m_data.dict->key.couldBe(o.m_data.dict->key) &&
           m_data.dict->val.couldBe(o.m_data.dict->val);
  case DataTag::Keyset:
    return m_data.keyset->keyval.couldBe(o.m_data.keyset->keyval);
  }
  not_reached();
}

bool Type::operator==(const Type& o) const {
  assert(checkInvariants());
  assert(o.checkInvariants());

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

bool Type::subtypeOf(Type o) const {
  assert(checkInvariants());
  assert(o.checkInvariants());

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

bool Type::strictSubtypeOf(Type o) const {
  assert(checkInvariants());
  assert(o.checkInvariants());
  return *this != o && subtypeOf(o);
}

bool Type::couldBe(Type o) const {
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
    m_data.inner->checkInvariants();
    assert(!m_data.inner->couldBe(TRef));
    break;
  case DataTag::Cls:    break;
  case DataTag::Obj:
    if (auto t = m_data.dobj.whType.get()) {
      t->checkInvariants();
    }
    break;
  case DataTag::VecVal:
  case DataTag::DictVal:
  case DataTag::KeysetVal:
  case DataTag::ArrVal:
    assert(m_dataTag != DataTag::VecVal || m_data.aval->isVecArray());
    assert(m_dataTag != DataTag::DictVal || m_data.aval->isDict());
    assert(m_dataTag != DataTag::KeysetVal || m_data.aval->isKeyset());
    assert(m_data.aval->isStatic());
    assert(!m_data.aval->empty());
    break;
  case DataTag::ArrPacked:
    assert(!m_data.apacked->elems.empty());
    for (DEBUG_ONLY auto& v : m_data.apacked->elems) {
      assert(v.subtypeOf(TInitGen));
    }
    break;
  case DataTag::ArrStruct:
    assert(!m_data.astruct->map.empty());
    for (DEBUG_ONLY auto& kv : m_data.astruct->map) {
      assert(kv.second.subtypeOf(TInitGen));
    }
    break;
  case DataTag::ArrPackedN:
    assert(m_data.apackedn->type.subtypeOf(TInitGen));
    break;
  case DataTag::ArrMapN:
    assert(m_data.amapn->key.subtypeOf(TInitCell));
    assert(m_data.amapn->val.subtypeOf(TInitGen));
    break;
  case DataTag::Vec:
    assert(m_data.vec->val.subtypeOf(TInitCell));
    break;
  case DataTag::Dict:
    assert(m_data.dict->key.subtypeOf(TInitCell));
    assert(m_data.dict->val.subtypeOf(TInitCell));
    break;
  case DataTag::Keyset:
    assert(m_data.keyset->keyval.subtypeOf(TInitCell));
    break;
  }
  return true;
}

//////////////////////////////////////////////////////////////////////

Type wait_handle(const Index& index, Type inner) {
  auto const rwh = index.builtin_class(s_WaitHandle.get());
  auto t = subObj(rwh);
  t.m_data.dobj.whType = make_copy_ptr<Type>(std::move(inner));
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
  new (&ret.m_data.dobj) DObj(wh.m_data.dobj.type, wh.m_data.dobj.cls);
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
  r.m_dataTag   = DataTag::ArrVal;
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
  r.m_dataTag   = DataTag::VecVal;
  return r;
}

Type vec_empty()         { return Type { BSVecE }; }
Type counted_vec_empty() { return Type { BCVecE }; }
Type some_vec_empty()    { return Type { BVecE }; }

template<trep t>
Type vec_n_impl(Type ty, folly::Optional<int64_t> count) {
  if (count && !*count) return vec_empty();
  auto ret = Type { t };
  new (&ret.m_data.vec) copy_ptr<DVec>(
    make_copy_ptr<DVec>(std::move(ty), count)
  );
  ret.m_dataTag = DataTag::Vec;
  return ret;
}

Type vec_n(Type ty, folly::Optional<int64_t> count) {
  return vec_n_impl<BVecN>(std::move(ty), std::move(count));
}

Type cvec_n(Type ty, folly::Optional<int64_t> count) {
  return vec_n_impl<BCVecN>(std::move(ty), std::move(count));
}

Type svec_n(Type ty, folly::Optional<int64_t> count) {
  return vec_n_impl<BSVecN>(std::move(ty), std::move(count));
}

Type dict_val(SArray val) {
  assert(val->isStatic());
  assert(val->isDict());
  if (val->empty()) return dict_empty();
  auto r        = Type { BSDictN };
  r.m_data.aval = val;
  r.m_dataTag   = DataTag::DictVal;
  return r;
}

Type dict_empty()         { return Type { BSDictE }; }
Type counted_dict_empty() { return Type { BCDictE }; }
Type some_dict_empty()    { return Type { BDictE }; }

template<trep t>
Type dict_n_impl(Type k, Type v) {
  auto ret = Type { t };
  new (&ret.m_data.dict) copy_ptr<DDict>(
    make_copy_ptr<DDict>(std::move(k), std::move(v))
  );
  ret.m_dataTag = DataTag::Dict;
  return ret;
}

Type dict_n(Type k, Type v) {
  return dict_n_impl<BDictN>(std::move(k), std::move(v));
}

Type cdict_n(Type k, Type v) {
  return dict_n_impl<BCDictN>(std::move(k), std::move(v));
}

Type sdict_n(Type k, Type v) {
  return dict_n_impl<BSDictN>(std::move(k), std::move(v));
}

Type keyset_val(SArray val) {
  assert(val->isStatic());
  assert(val->isKeyset());
  if (val->empty()) return keyset_empty();
  auto r        = Type { BSKeysetN };
  r.m_data.aval = val;
  r.m_dataTag   = DataTag::KeysetVal;
  return r;
}

Type keyset_empty()         { return Type { BSKeysetE }; }
Type counted_keyset_empty() { return Type { BCKeysetE }; }
Type some_keyset_empty()    { return Type { BKeysetE }; }

struct StrictKey {
  folly::Optional<int64_t> i;
  folly::Optional<SString> s;
  Type type;
};

template<trep t>
Type keyset_n_impl(Type kv) {
  kv = disect_strict_key(kv).type;
  if (kv == TBottom) return TKeyset;
  auto ret = Type { t };
  new (&ret.m_data.keyset) copy_ptr<DKeyset>(
    make_copy_ptr<DKeyset>(std::move(kv))
  );
  ret.m_dataTag = DataTag::Keyset;
  return ret;
}

Type keyset_n(Type kv) {
  return keyset_n_impl<BKeysetN>(std::move(kv));
}

Type ckeyset_n(Type kv) {
  return keyset_n_impl<BCKeysetN>(std::move(kv));
}

Type skeyset_n(Type kv) {
  return keyset_n_impl<BSKeysetN>(std::move(kv));
}

Type toDVecType(Type ty) {
  assert(ty.m_dataTag == DataTag::VecVal);
  auto dv = toDVec(ty.m_data.aval);
  auto r = vec_n(std::move(dv.val), dv.len);
  r.m_bits = ty.m_bits;
  return r;
}

Type toDDictType(Type ty) {
  assert(ty.m_dataTag == DataTag::DictVal);
  auto dv = toDDict(ty.m_data.aval);
  auto r = dict_n(std::move(dv.key), std::move(dv.val));
  r.m_bits = ty.m_bits;
  return r;
}

Type toDKeysetType(Type ty) {
  assert(ty.m_dataTag == DataTag::KeysetVal);
  auto kv = toDKeyset(ty.m_data.aval);
  auto r = keyset_n(std::move(kv.keyval));
  r.m_bits = ty.m_bits;
  return r;
}

Type subObj(res::Class val) {
  auto r = Type { BObj };
  new (&r.m_data.dobj) DObj(
    val.couldBeOverriden() ? DObj::Sub : DObj::Exact,
    val
  );
  r.m_dataTag = DataTag::Obj;
  return r;
}

Type objExact(res::Class val) {
  auto r = Type { BObj };
  new (&r.m_data.dobj) DObj(DObj::Exact, val);
  r.m_dataTag = DataTag::Obj;
  return r;
}

Type subCls(res::Class val) {
  auto r        = Type { BCls };
  new (&r.m_data.dcls) DCls {
    val.couldBeOverriden() ? DCls::Sub : DCls::Exact,
    val
  };
  r.m_dataTag = DataTag::Cls;
  return r;
}

Type clsExact(res::Class val) {
  auto r        = Type { BCls };
  new (&r.m_data.dcls) DCls { DCls::Exact, val };
  r.m_dataTag   = DataTag::Cls;
  return r;
}


Type ref_to(Type t) {
  assert(t.subtypeOf(TInitCell));
  auto r = Type{BRef};
  new (&r.m_data.inner) copy_ptr<Type> {make_copy_ptr<Type>(std::move(t))};
  r.m_dataTag = DataTag::RefInner;
  return r;
}

bool is_ref_with_inner(const Type& t) {
  return t.m_dataTag == DataTag::RefInner;
}

bool is_specialized_array(const Type& t) {
  switch (t.m_dataTag) {
  case DataTag::None:
  case DataTag::Str:
  case DataTag::Obj:
  case DataTag::Int:
  case DataTag::Dbl:
  case DataTag::Cls:
  case DataTag::RefInner:
  case DataTag::VecVal:
  case DataTag::DictVal:
  case DataTag::KeysetVal:
  case DataTag::Vec:
  case DataTag::Dict:
  case DataTag::Keyset:
    return false;
  case DataTag::ArrVal:
  case DataTag::ArrPacked:
  case DataTag::ArrPackedN:
  case DataTag::ArrStruct:
  case DataTag::ArrMapN:
    return true;
  }
  not_reached();
}

bool is_specialized_vec(const Type& t) {
  return t.m_dataTag == DataTag::Vec || t.m_dataTag == DataTag::VecVal;
}

bool is_specialized_dict(const Type& t) {
  return t.m_dataTag == DataTag::Dict || t.m_dataTag == DataTag::DictVal;
}

bool is_specialized_keyset(const Type& t) {
  return t.m_dataTag == DataTag::Keyset || t.m_dataTag == DataTag::KeysetVal;
}

Type spec_vec_union(Type specVecA, Type vecB) {
  auto const bits = combine_vec_bits(specVecA.m_bits, vecB.m_bits);
  if (!is_specialized_vec(vecB)) {
    if (vecB.subtypeOf(TOptVecE)) {
      specVecA.m_bits = bits;
      return specVecA;
    }
    return Type { bits };
  }

  if (specVecA.m_dataTag == DataTag::VecVal) {
    if (vecB.m_dataTag == DataTag::VecVal) {
      if (specVecA.m_data.aval == vecB.m_data.aval) {
        specVecA.m_bits = bits;
        return specVecA;
      }
      vecB = toDVecType(vecB);
    }
    specVecA = toDVecType(specVecA);
  } else if (vecB.m_dataTag == DataTag::VecVal) {
    vecB = toDVecType(vecB);
  }
  assert(specVecA.m_dataTag == DataTag::Vec);
  assert(vecB.m_dataTag == DataTag::Vec);

  auto& lenA = specVecA.m_data.vec->len;
  auto& valA = specVecA.m_data.vec->val;
  if (!lenA || !vecB.m_data.vec->len || *lenA != *vecB.m_data.vec->len) {
    lenA = folly::none;
  }
  valA = union_of(valA, vecB.m_data.vec->val);
  specVecA.m_bits = bits;

  return specVecA;
}

Type spec_dict_union(Type specDictA, Type dictB) {
  auto const bits = combine_dict_bits(specDictA.m_bits, dictB.m_bits);
  if (!is_specialized_dict(dictB)) {
    if (dictB.subtypeOf(TOptDictE)) {
      specDictA.m_bits = bits;
      return specDictA;
    }
    return Type { bits };
  }

  if (specDictA.m_dataTag == DataTag::DictVal) {
    if (dictB.m_dataTag == DataTag::DictVal) {
      if (specDictA.m_data.aval == dictB.m_data.aval) {
        specDictA.m_bits = bits;
        return specDictA;
      }
      dictB = toDDictType(dictB);
    }
    specDictA = toDDictType(specDictA);
  } else if (dictB.m_dataTag == DataTag::DictVal) {
    dictB = toDDictType(dictB);
  }
  assert(specDictA.m_dataTag == DataTag::Dict);
  assert(dictB.m_dataTag == DataTag::Dict);

  auto& keyA = specDictA.m_data.dict->key;
  auto& valA = specDictA.m_data.dict->val;
  keyA = union_of(keyA, dictB.m_data.dict->key);
  valA = union_of(valA, dictB.m_data.dict->val);
  specDictA.m_bits = bits;

  return specDictA;
}

Type spec_keyset_union(Type specKeysetA, Type keysetB) {
  auto const bits = combine_keyset_bits(specKeysetA.m_bits, keysetB.m_bits);
  if (!is_specialized_keyset(keysetB)) {
    if (keysetB.subtypeOf(TOptKeysetE)) {
      specKeysetA.m_bits = bits;
      return specKeysetA;
    }
    return Type { bits };
  }

  if (specKeysetA.m_dataTag == DataTag::KeysetVal) {
    if (keysetB.m_dataTag == DataTag::KeysetVal) {
      if (specKeysetA.m_data.aval == keysetB.m_data.aval) {
        specKeysetA.m_bits = bits;
        return specKeysetA;
      }
      keysetB = toDKeysetType(keysetB);
    }
    specKeysetA = toDKeysetType(specKeysetA);
  } else if (keysetB.m_dataTag == DataTag::KeysetVal) {
    keysetB = toDKeysetType(keysetB);
  }
  assert(specKeysetA.m_dataTag == DataTag::Keyset);
  assert(keysetB.m_dataTag == DataTag::Keyset);

  auto& kvA = specKeysetA.m_data.keyset->keyval;
  kvA = union_of(kvA, keysetB.m_data.keyset->keyval);
  specKeysetA.m_bits = bits;

  return specKeysetA;
}

Type arr_packed(std::vector<Type> elems) {
  assert(!elems.empty());
  auto r = Type { BArrN };
  new (&r.m_data.apacked) copy_ptr<DArrPacked>(
    make_copy_ptr<DArrPacked>(std::move(elems))
  );
  r.m_dataTag = DataTag::ArrPacked;
  return r;
}

Type sarr_packed(std::vector<Type> elems) {
  auto t = arr_packed(std::move(elems));
  t.m_bits = BSArrN;
  return t;
}

Type carr_packed(std::vector<Type> elems) {
  auto t = arr_packed(std::move(elems));
  t.m_bits = BCArrN;
  return t;
}

Type arr_packedn(Type t) {
  auto r = Type { BArrN };
  new (&r.m_data.apackedn) copy_ptr<DArrPackedN>(
    make_copy_ptr<DArrPackedN>(std::move(t))
  );
  r.m_dataTag = DataTag::ArrPackedN;
  return r;
}

Type sarr_packedn(Type t) {
  auto r = arr_packedn(std::move(t));
  r.m_bits = BSArrN;
  return r;
}

Type carr_packedn(Type t) {
  auto r = arr_packedn(std::move(t));
  r.m_bits = BCArrN;
  return r;
}

Type arr_struct(StructMap m) {
  assert(!m.empty());
  auto r = Type { BArrN };
  new (&r.m_data.astruct) copy_ptr<DArrStruct>(
    make_copy_ptr<DArrStruct>(std::move(m))
  );
  r.m_dataTag = DataTag::ArrStruct;
  return r;
}

Type sarr_struct(StructMap m) {
  auto r = arr_struct(std::move(m));
  r.m_bits = BSArrN;
  return r;
}

Type carr_struct(StructMap m) {
  auto r = arr_struct(std::move(m));
  r.m_bits = BCArrN;
  return r;
}

Type arr_mapn(Type k, Type v) {
  auto r = Type { BArrN };
  new (&r.m_data.amapn) copy_ptr<DArrMapN>(
    make_copy_ptr<DArrMapN>(std::move(k), std::move(v))
  );
  r.m_dataTag = DataTag::ArrMapN;
  return r;
}

Type sarr_mapn(Type k, Type v) {
  auto r = arr_mapn(k, v);
  r.m_bits = BSArrN;
  return r;
}

Type carr_mapn(Type k, Type v) {
  auto r = arr_mapn(std::move(k), std::move(v));
  r.m_bits = BCArrN;
  return r;
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

bool is_opt(Type t) {
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

folly::Optional<Cell> tv(Type t) {
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
    case DataTag::VecVal:
    case DataTag::DictVal:
    case DataTag::KeysetVal:
    case DataTag::ArrVal:
      if ((t.m_bits & BArrN) == t.m_bits) {
        return make_tv<KindOfPersistentArray>(
          const_cast<ArrayData*>(t.m_data.aval)
        );
      } else if ((t.m_bits & BVecN) == t.m_bits) {
        return make_tv<KindOfPersistentVec>(
          const_cast<ArrayData*>(t.m_data.aval)
        );
      } else if ((t.m_bits & BDictN) == t.m_bits) {
        return make_tv<KindOfPersistentDict>(
          const_cast<ArrayData*>(t.m_data.aval)
        );
      } else if ((t.m_bits & BKeysetN) == t.m_bits) {
        return make_tv<KindOfPersistentKeyset>(
          const_cast<ArrayData*>(t.m_data.aval)
        );
      }
      break;
    case DataTag::ArrStruct:
    case DataTag::ArrPacked:
      // TODO(#3696042): we could materialize a static array here if
      // we check if a whole specialized array is constants, and if it
      // can't be empty.
      break;
    case DataTag::RefInner:
    case DataTag::ArrPackedN:
    case DataTag::ArrMapN:
    case DataTag::Obj:
    case DataTag::Cls:
    case DataTag::None:
    case DataTag::Vec:
    case DataTag::Dict:
    case DataTag::Keyset:
      break;
    }
  }

  return folly::none;
}

Type type_of_istype(IsTypeOp op) {
  switch (op) {
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

  case KindOfClass:
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
  case KindOfClass:
    break;
  }
  always_assert(0 && "dt in from_DataType didn't satisfy preconditions");
}

Type from_hni_constraint(SString s) {
  if (!s) return TGen;

  auto p   = s->data();
  auto ret = TBottom;

  if (*p == '?') {
    ret = union_of(ret, TInitNull);
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
  if (!strcasecmp(p, "array"))        return union_of(ret, TArr);
  if (!strcasecmp(p, "HH\\mixed"))    return TInitGen;

  // It might be an object, or we might want to support type aliases in HNI at
  // some point.  For now just be conservative.
  return TGen;
}

Type Type::unionArr(const Type& a, const Type& b) {
  assert(!a.subtypeOf(b));
  assert(!b.subtypeOf(a));
  assert(is_specialized_array(a));
  assert(is_specialized_array(b));

  auto ret = Type{};
  auto const newBits = combine_arr_bits(a.m_bits, b.m_bits);

  if (a.m_dataTag != b.m_dataTag) {
    ret = a.disjointDataFn(b, ArrUnion{});
    ret.m_bits = newBits;
    return ret;
  }

  switch (a.m_dataTag) {
  case DataTag::Vec:
  case DataTag::Dict:
  case DataTag::Keyset:
  case DataTag::VecVal:
  case DataTag::DictVal:
  case DataTag::KeysetVal:
  case DataTag::None:
  case DataTag::Str:
  case DataTag::Obj:
  case DataTag::Int:
  case DataTag::Dbl:
  case DataTag::Cls:
  case DataTag::RefInner:
    not_reached();
  case DataTag::ArrVal:
    {
      ret = ArrUnion{}(a.m_data.aval, b.m_data.aval);
      ret.m_bits = newBits;
      return ret;
    }
  case DataTag::ArrPacked:
    {
      ret = ArrUnion{}(*a.m_data.apacked, *b.m_data.apacked);
      ret.m_bits = newBits;
      return ret;
    }
  case DataTag::ArrPackedN:
    {
      ret = a;
      ret.m_bits = newBits;
      ret.m_data.apackedn->type = union_of(a.m_data.apackedn->type,
                                           b.m_data.apackedn->type);
      return ret;
    }
  case DataTag::ArrStruct:
    {
      ret = ArrUnion{}(*a.m_data.astruct, *b.m_data.astruct);
      ret.m_bits = newBits;
      return ret;
    }
  case DataTag::ArrMapN:
    {
      ret = arr_mapn(union_of(a.m_data.amapn->key, b.m_data.amapn->key),
                     union_of(a.m_data.amapn->val, b.m_data.amapn->val));
      ret.m_bits = newBits;
      return ret;
    }
  }
  not_reached();
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
      *a.m_data.dobj.whType = union_of(
        *a.m_data.dobj.whType,
        *b.m_data.dobj.whType
      );
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
    if (is_specialized_array(b)) {
      DEBUG_ONLY auto const shouldBeOpt = is_opt(a) || is_opt(b);
      auto const t = Type::unionArr(a, b);
      if (shouldBeOpt) assert(is_opt(t));
      return t;
    }
    if (b.subtypeOf(TOptArrE)) {
      // Keep a's data.
      a.m_bits = combine_arr_bits(a.m_bits, b.m_bits);
      return a;
    }
    if (b.subtypeOf(TOptArr)) {
      // We can't keep a's data, since it contains unknown non-empty
      // arrays.  (If it were specialized we'd be in the unionArr
      // path, which handles trying to keep as much data as we can.)
      return Type { combine_arr_bits(a.m_bits, b.m_bits) };
    }
  } else {
    if (is_specialized_array(b)) {
      // Flip args and do the above.
      return union_of(b, a);
    }
  }

  if (is_specialized_vec(a) && b.subtypeOf(TOptVec)) {
    return spec_vec_union(a, b);
  }

  if (is_specialized_dict(a) && b.subtypeOf(TOptDict)) {
    return spec_dict_union(a, b);
  }

  if (is_specialized_keyset(a) && b.subtypeOf(TOptKeyset)) {
    return spec_keyset_union(a, b);
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


  /*
   * Merging option types tries to preserve subtype information where it's
   * possible.  E.g. if you union InitNull and Obj<=Foo, we want OptObj<=Foo to
   * be the result.
   */
  if (a == TInitNull && canBeOptional(b.m_bits)) return opt(b);
  if (b == TInitNull && canBeOptional(a.m_bits)) return opt(a);

  // Optional types where the non-Null part is already a union or can have a
  // value need to be manually tried (e.g. if we are merging TOptTrue and TOptFalse,
  // we want TOptBool, or merging TOptInt=1 and TOptInt=2 should give us TOptInt).
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

  // Currently the only types in our typesystem that have infinitely
  // growing chains of union_of are specialized arrays.
  if (!is_specialized_array(a) || !is_specialized_array(b)) {
    return union_of(a, b);
  }

  // This (throwing away the data) is overly conservative, but works
  // for now.
  auto const newBits = combine_arr_bits(a.m_bits, b.m_bits);
  return Type { newBits };
}

Type stack_flav(Type a) {
  if (a.subtypeOf(TUninit))   return TUninit;
  if (a.subtypeOf(TInitCell)) return TInitCell;
  if (a.subtypeOf(TRef))      return TRef;
  if (a.subtypeOf(TGen))      return TGen;
  if (a.subtypeOf(TCls))      return TCls;
  always_assert(0 && "stack_flav passed invalid type");
}

Type loosen_statics(Type a) {
  // TODO(#3696042): this should be modified to keep specialized array
  // information, including whether the array is possibly empty.
  if (a.couldBe(TSStr)) a = union_of(a, TStr);
  if (a.couldBe(TSArr)) a = union_of(a, TArr);
  if (a.couldBe(TSVec)) a = union_of(a, TVec);
  if (a.couldBe(TSDict)) a = union_of(a, TDict);
  if (a.couldBe(TSKeyset)) a = union_of(a, TKeyset);
  return a;
}

Type loosen_values(Type a) {
  return a.strictSubtypeOf(TInt) ? TInt :
         a.strictSubtypeOf(TDbl) ? TDbl :
         a.strictSubtypeOf(TBool) ? TBool :
         a.strictSubtypeOf(TSStr) ? TSStr :
         a.strictSubtypeOf(TSArr) ? TSArr :
         a.strictSubtypeOf(TSVec) ? TSVec :
         a.strictSubtypeOf(TSDict) ? TSDict :
         a.strictSubtypeOf(TSKeyset) ? TSKeyset :
         a == TOptFalse || a == TOptTrue ? TOptBool :
         a;
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
 * For known strings that are strictly integers, we'll set both the
 * known integer and string keys, so generally the int case should be
 * checked first below.
 *
 * For keys that could be strings, we have to assume they could be
 * strictly-integer strings.  After disection, the effective type we
 * can assume for the array key is in `type'.
 */

struct ArrKey {
  folly::Optional<int64_t> i;
  folly::Optional<SString> s;
  Type type;
};

ArrKey disect_array_key(const Type& keyTy) {
  auto ret = ArrKey{};

  if (keyTy.strictSubtypeOf(TInt)) {
    ret.i = keyTy.m_data.ival;
    ret.type = keyTy;
    return ret;
  }
  if (keyTy.strictSubtypeOf(TStr) && keyTy.m_dataTag == DataTag::Str) {
    ret.s = keyTy.m_data.sval;
    ret.type = keyTy;
    int64_t i;
    if (keyTy.m_data.sval->isStrictlyInteger(i)) {
      ret.i = i;
      ret.type = TInt;
    }
    return ret;
  }
  if (keyTy.strictSubtypeOf(TDbl)) {
    ret.i = static_cast<int64_t>(keyTy.m_data.dval);
    ret.type = TInt;
    return ret;
  }
  if (keyTy.subtypeOf(TNum)) {
    ret.type = TInt;
    return ret;
  }

  if (keyTy.subtypeOf(TNull)) {
    ret.s = s_empty.get();
    ret.type = sempty();
    return ret;
  }
  if (keyTy.subtypeOf(TRes)) {
    ret.type = TInt;
    return ret;
  }

  if (keyTy.subtypeOf(TTrue)) {
    ret.i = 1;
    ret.type = TInt;
    return ret;
  }
  if (keyTy.subtypeOf(TFalse)) {
    ret.i = 0;
    ret.type = TInt;
    return ret;
  }
  if (keyTy.subtypeOf(TBool)) {
    ret.type = TInt;
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
      return ret;
    }
  }

  // Nothing we can do in other cases that could be strings (without
  // statically known values)---they may behave like integers at
  // runtime.

  // TODO(#3774082): We should be able to set this to a Str|Int type.
  ret.type = TInitCell;
  return ret;
}

Type array_elem(const Type& arr, const Type& undisectedKey) {
  assert(arr.subtypeOf(TArr));

  auto const key = disect_array_key(undisectedKey);
  auto ty = [&]() -> Type {
    switch (arr.m_dataTag) {
    case DataTag::Str:
    case DataTag::Obj:
    case DataTag::Int:
    case DataTag::Dbl:
    case DataTag::Cls:
    case DataTag::RefInner:
    case DataTag::Vec:
    case DataTag::Dict:
    case DataTag::Keyset:
    case DataTag::VecVal:
    case DataTag::DictVal:
    case DataTag::KeysetVal:
      not_reached();

    case DataTag::None:
      return arr.subtypeOf(TSArr) ? TInitUnc : TInitCell;

    case DataTag::ArrVal:
      if (key.i) {
        if (auto const r = arr.m_data.aval->nvGet(*key.i)) {
          return from_cell(*r);
        }
        return TInitNull;
      }
      if (key.s) {
        if (auto const r = arr.m_data.aval->nvGet(*key.s)) {
          return from_cell(*r);
        }
        return TInitNull;
      }
      return arr.subtypeOf(TSArr) ? TInitUnc : TInitCell;

    /*
     * In the following cases, note that if you get an elem out of an
     * array that doesn't exist, php semantics are to return null (after
     * a warning).  So in cases where we don't know the key statically
     * we need to union TInitNull into the result.
     */

    case DataTag::ArrPacked:
      if (!key.i) {
        return union_of(packed_values(*arr.m_data.apacked), TInitNull);
      }
      if (*key.i >= 0 && *key.i < arr.m_data.apacked->elems.size()) {
        return arr.m_data.apacked->elems[*key.i];
      }
      return TInitNull;

    case DataTag::ArrPackedN:
      return union_of(arr.m_data.apackedn->type, TInitNull);

    case DataTag::ArrStruct:
      if (key.s) {
        auto it = arr.m_data.astruct->map.find(*key.s);
        return it != end(arr.m_data.astruct->map)
          ? it->second
          : TInitNull;
      }
      return union_of(struct_values(*arr.m_data.astruct), TInitNull);

    case DataTag::ArrMapN:
      return union_of(arr.m_data.amapn->val, TInitNull);
    }
    not_reached();
  }();

  if (!ty.subtypeOf(TInitCell)) {
    ty = TInitCell;
  } else if (arr.couldBe(TArrE) || arr.couldBe(TVecE) || arr.couldBe(TDictE) ||
             arr.couldBe(TKeysetE)) {
    ty = union_of(std::move(ty), TInitNull);
  }
  return ty;
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

// Do the effects of array_set but without handling possibly emptiness
// of `arr'.
Type arrayN_set(Type arr, const Type& undisectedKey, const Type& val) {
  auto const key = disect_array_key(undisectedKey);

  auto ensure_counted = [&] {
    // TODO(#3696042): this same logic should be in loosen_statics.
    if (arr.m_bits & BCArr) return;
    if (arr.m_bits == BSArrN) {
      arr.m_bits = combine_arr_bits(arr.m_bits, BCArrN);
    } else if (arr.m_bits == BSArrE) {
      arr.m_bits = combine_arr_bits(arr.m_bits, BCArrE);
    } else {
      arr.m_bits = combine_arr_bits(arr.m_bits, BCArr);
    }
  };

  switch (arr.m_dataTag) {
  case DataTag::Str:
  case DataTag::Obj:
  case DataTag::Int:
  case DataTag::Dbl:
  case DataTag::Cls:
  case DataTag::RefInner:
  case DataTag::VecVal:
  case DataTag::DictVal:
  case DataTag::KeysetVal:
  case DataTag::Vec:
  case DataTag::Dict:
  case DataTag::Keyset:
    not_reached();

  case DataTag::None:
    ensure_counted();
    return arr;

  case DataTag::ArrVal:
    {
      if (auto d = toDArrStruct(arr.m_data.aval)) {
        return arrayN_set(arr_struct(std::move(d->map)), undisectedKey, val);
      }
      if (auto d = toDArrPacked(arr.m_data.aval)) {
        return arrayN_set(arr_packed(std::move(d->elems)), undisectedKey,
                          val);
      }
      if (auto d = toDArrPackedN(arr.m_data.aval)) {
        return arrayN_set(arr_packedn(d->type), undisectedKey, val);
      }
      auto d = toDArrMapN(arr.m_data.aval);
      return arrayN_set(arr_mapn(d.key, d.val), undisectedKey, val);
    }

  case DataTag::ArrPacked:
    ensure_counted();
    if (key.i) {
      if (*key.i >= 0 && *key.i < arr.m_data.apacked->elems.size()) {
        auto& current = arr.m_data.apacked->elems[*key.i];
        if (current.subtypeOf(TInitCell)) {
          current = val;
        }
        return arr;
      }
      if (*key.i == arr.m_data.apacked->elems.size()) {
        arr.m_data.apacked->elems.push_back(val);
        return arr;
      }
    }
    return arr_mapn(union_of(TInt, key.type),
                    union_of(packed_values(*arr.m_data.apacked), val));

  case DataTag::ArrPackedN:
    ensure_counted();
    return arr_mapn(union_of(TInt, key.type),
                    union_of(arr.m_data.apackedn->type, val));

  case DataTag::ArrStruct:
    ensure_counted();
    {
      auto& map = arr.m_data.astruct->map;
      if (key.s) {
        auto it = map.find(*key.s);
        if (it == end(map)) {
          map[*key.s] = val;
        } else {
          if (it->second.subtypeOf(TInitCell)) {
            it->second = val;
          }
        }
        return arr;
      }
      return arr_mapn(union_of(key.type, TSStr),
                      union_of(struct_values(*arr.m_data.astruct), val));
    }

  case DataTag::ArrMapN:
    ensure_counted();
    return arr_mapn(union_of(key.type, arr.m_data.amapn->key),
                    union_of(val, arr.m_data.amapn->val));
  }

  not_reached();
}

Type array_set(Type arr, const Type& undisectedKey, const Type& val) {
  assert(arr.subtypeOf(TArr));

  // Unless you know an array can't cow, you don't know if the TRef
  // will stay a TRef or turn back into a TInitCell.  Generally you
  // want a TInitGen.
  always_assert(!val.subtypeOf(TRef) &&
         "You probably don't want to put Ref types into arrays ...");

  auto nonEmptyPart =
    arr.couldBe(TArrN) ? arrayN_set(arr, undisectedKey, val)
                       : TBottom;
  if (!arr.couldBe(TArrE)) {
    assert(nonEmptyPart != TBottom);
    return nonEmptyPart;
  }

  // Union in the effects of doing the set if the array was empty.
  auto const key = disect_array_key(undisectedKey);
  auto emptyPart = [&] {
    if (key.s && !key.i) {
      auto map = StructMap{};
      map[*key.s] = val;
      return arr_struct(std::move(map));
    }
    if (key.i && *key.i == 0) return arr_packed({val});
    // Keeping the ArrE for now just because we would need to check
    // the key.type is not an invalid key (array or object) to prove
    // it's non-empty now.
    return union_of(arr_mapn(key.type, val), arr);
  }();
  return union_of(std::move(nonEmptyPart), std::move(emptyPart));
}

// Do the same as array_newelem_key, but ignoring possible emptiness
// of `arr'.
std::pair<Type,Type> arrayN_newelem_key(const Type& arr, const Type& val) {
  switch (arr.m_dataTag) {
  case DataTag::Str:
  case DataTag::Obj:
  case DataTag::Int:
  case DataTag::Dbl:
  case DataTag::Cls:
  case DataTag::RefInner:
  case DataTag::VecVal:
  case DataTag::DictVal:
  case DataTag::KeysetVal:
  case DataTag::Vec:
  case DataTag::Dict:
  case DataTag::Keyset:
    not_reached();

  case DataTag::None:
    return { TArrN, TInt };

  case DataTag::ArrVal:
    {
      if (auto d = toDArrStruct(arr.m_data.aval)) {
        return array_newelem_key(arr_struct(std::move(d->map)), val);
      }
      if (auto d = toDArrPacked(arr.m_data.aval)) {
        return array_newelem_key(arr_packed(std::move(d->elems)), val);
      }
      if (auto d = toDArrPackedN(arr.m_data.aval)) {
        return array_newelem_key(arr_packedn(d->type), val);
      }
      auto d = toDArrMapN(arr.m_data.aval);
      return array_newelem_key(arr_mapn(d.key, d.val), val);
    }

  case DataTag::ArrPacked:
    {
      auto v = arr.m_data.apacked->elems;
      v.push_back(val);
      return { arr_packed(std::move(v)), ival(v.size() - 1) };
    }

  case DataTag::ArrPackedN:
    return { arr_packedn(union_of(arr.m_data.apackedn->type, val)), TInt };

  case DataTag::ArrStruct:
    return { arr_mapn(union_of(TSStr, TInt),
                      union_of(struct_values(*arr.m_data.astruct), val)),
             TInt };

  case DataTag::ArrMapN:
    return { arr_mapn(union_of(arr.m_data.amapn->key, TInt),
                      union_of(arr.m_data.amapn->val, TInitNull)),
             TInt };
  }

  not_reached();
}

std::pair<Type,Type> array_newelem_key(const Type& arr, const Type& val) {
  assert(arr.subtypeOf(TArr));

  // Unless you know an array can't cow, you don't know if the TRef
  // will stay a TRef or turn back into a TInitCell.  Generally you
  // want a TInitGen.
  always_assert(!val.subtypeOf(TRef) &&
         "You probably don't want to put Ref types into arrays ...");

  // Inserting in an empty array creates a packed array of size one.
  auto emptyPart = arr.couldBe(TArrE)
    ? std::make_pair(arr_packed({val}), ival(0))
    : std::make_pair(TBottom, TBottom);

  auto nonEmptyPart = arr.couldBe(TArrN)
    ? arrayN_newelem_key(arr, val)
    : std::make_pair(TBottom, TBottom);

  return {
    union_of(std::move(emptyPart.first), std::move(nonEmptyPart.first)),
    union_of(std::move(emptyPart.second), std::move(nonEmptyPart.second))
  };
}

Type array_newelem(const Type& arr, const Type& val) {
  return array_newelem_key(arr, val).first;
}

std::pair<Type,Type> iter_types(const Type& iterable) {
  if (!iterable.subtypeOf(TArr) || !is_specialized_array(iterable)) {
    return { TInitCell, TInitCell };
  }

  // Note: we don't need to handle possible emptiness explicitly,
  // because if the array was empty we won't ever pull anything out
  // while iterating.

  switch (iterable.m_dataTag) {
  case DataTag::None:
    if (iterable.subtypeOf(TSArr)) {
      return { TInitUnc, TInitUnc };
    }
    return { TInitCell, TInitCell };
  case DataTag::Str:
  case DataTag::Obj:
  case DataTag::Int:
  case DataTag::Dbl:
  case DataTag::Cls:
  case DataTag::RefInner:
    always_assert(0);
  case DataTag::Vec:
    return { TInt, iterable.m_data.vec->val };
  case DataTag::Dict:
    return { iterable.m_data.dict->key, iterable.m_data.dict->val };
  case DataTag::Keyset:
    return { iterable.m_data.keyset->keyval, iterable.m_data.keyset->keyval };
  case DataTag::VecVal:
  case DataTag::DictVal:
  case DataTag::ArrVal:
  case DataTag::KeysetVal:
    {
      auto const mn = toDArrMapN(iterable.m_data.aval);
      return { mn.key, mn.val };
    }
  case DataTag::ArrPacked:
    return { TInt, packed_values(*iterable.m_data.apacked) };
  case DataTag::ArrPackedN:
    return { TInt, iterable.m_data.apackedn->type };
  case DataTag::ArrStruct:
    return { TSStr, struct_values(*iterable.m_data.astruct) };
  case DataTag::ArrMapN:
    return { iterable.m_data.amapn->key, iterable.m_data.amapn->val };
  }

  not_reached();
}

//////////////////////////////////////////////////////////////////////

struct VecKey {
  folly::Optional<int64_t> i;
  Type type;
};

VecKey disect_vec_key(const Type& keyTy) {
  auto ret = VecKey{};
  if (!keyTy.couldBe(TInt)) return {folly::none, TBottom};
  if (keyTy.strictSubtypeOf(TInt)) {
    ret.i = keyTy.m_data.ival;
  }
  ret.type = TInt;
  return ret;
}

std::pair<Type, bool> vec_elem(const Type& vec, const Type& undisectedKey) {
  auto const key = disect_vec_key(undisectedKey);
  if (key.type == TBottom) return {TBottom, false};
  if ((vec.m_bits & BVecE) == vec.m_bits) return {TBottom, false};

  const bool maybeEmpty = vec.m_bits & BVecE;

  switch (vec.m_dataTag) {
  case DataTag::ArrVal:
  case DataTag::ArrPacked:
  case DataTag::ArrPackedN:
  case DataTag::ArrStruct:
  case DataTag::ArrMapN:
  case DataTag::DictVal:
  case DataTag::Dict:
  case DataTag::KeysetVal:
  case DataTag::Keyset:
  case DataTag::Str:
  case DataTag::Obj:
  case DataTag::Int:
  case DataTag::Dbl:
  case DataTag::Cls:
  case DataTag::RefInner:
    not_reached();

  case DataTag::None:
    return {TInitCell, false};

  case DataTag::VecVal: {
    auto const ad = vec.m_data.aval;
    if (key.i) {
      if (auto const r = ad->nvGet(*key.i)) {
        return {from_cell(*r), !maybeEmpty};
      }
      return {TBottom, false};
    }
    return {TInitCell, false};
  }
  case DataTag::Vec:
    if (key.i && vec.m_data.vec->len) {
      if (0 > *key.i || *key.i >= *vec.m_data.vec->len) return {TBottom, false};
      return {vec.m_data.vec->val, !maybeEmpty};
    }
    return {vec.m_data.vec->val, false};
  }
  not_reached();
}

std::pair<Type, bool>
vec_set(Type vec, const Type& undisectedKey, const Type& val) {
  auto const key = disect_vec_key(undisectedKey);
  if (key.type == TBottom) return {TBottom, false};
  if ((vec.m_bits & BVecE) == vec.m_bits) return {TBottom, false};

  const bool maybeEmpty = vec.m_bits & BVecE;
  if (!(vec.m_bits & BCVec)) {
    vec.m_bits = vec.m_bits == BSVecN ? BVecN : BVec;
  }

  switch (vec.m_dataTag) {
  case DataTag::ArrVal:
  case DataTag::ArrPacked:
  case DataTag::ArrPackedN:
  case DataTag::ArrStruct:
  case DataTag::ArrMapN:
  case DataTag::DictVal:
  case DataTag::Dict:
  case DataTag::KeysetVal:
  case DataTag::Keyset:
  case DataTag::Str:
  case DataTag::Obj:
  case DataTag::Int:
  case DataTag::Dbl:
  case DataTag::Cls:
  case DataTag::RefInner:
    not_reached();

  case DataTag::None:
    return {std::move(vec), false};

  case DataTag::VecVal: {
    vec = toDVecType(vec);
    /* fallthrough */
  }
  case DataTag::Vec:
    auto& ty = vec.m_data.vec->val;
    ty = union_of(std::move(ty), val);
    if (key.i && vec.m_data.vec->len) {
      if (0 > *key.i || *key.i >= *vec.m_data.vec->len) return {TBottom, false};
      return {vec, !maybeEmpty};
    }
    return {std::move(vec), false};
  }
  not_reached();
}

Type vec_newelem(Type vec, const Type& val) {
  vec.m_bits = !(vec.m_bits & BSVec) ? BCVecN : BVecN;

  switch (vec.m_dataTag) {
  case DataTag::ArrVal:
  case DataTag::ArrPacked:
  case DataTag::ArrPackedN:
  case DataTag::ArrStruct:
  case DataTag::ArrMapN:
  case DataTag::DictVal:
  case DataTag::Dict:
  case DataTag::KeysetVal:
  case DataTag::Keyset:
  case DataTag::Str:
  case DataTag::Obj:
  case DataTag::Int:
  case DataTag::Dbl:
  case DataTag::Cls:
  case DataTag::RefInner:
    not_reached();

  case DataTag::None:
    return vec;

  case DataTag::VecVal: {
    vec = toDVecType(vec);
    /* fallthrough */
  }
  case DataTag::Vec:
    auto& ty = vec.m_data.vec->val;
    ty = union_of(std::move(ty), val);
    if (vec.m_data.vec->len) (*vec.m_data.vec->len)++;
    return vec;
  }
  not_reached();
}

//////////////////////////////////////////////////////////////////////

StrictKey disect_strict_key(const Type& keyTy) {
  auto ret = StrictKey{};
  if (!keyTy.couldBe(TInt) && !keyTy.couldBe(TStr)) {
    ret.type = TBottom;
    return ret;
  }

  if (keyTy.strictSubtypeOf(TInt)) {
    ret.i = keyTy.m_data.ival;
    ret.type = keyTy;
    return ret;
  }

  if (keyTy.strictSubtypeOf(TStr) && keyTy.m_dataTag == DataTag::Str) {
    ret.s = keyTy.m_data.sval;
    ret.type = keyTy;
    return ret;
  }

  if (!keyTy.subtypeOf(TInt) && !keyTy.subtypeOf(TStr)) {
    ret.type = TInitCell;
    return ret;
  }
  ret.type = keyTy;
  return ret;
}

std::pair<Type, bool> dict_elem(const Type& dict, const Type& undisectedKey) {
  auto const key = disect_strict_key(undisectedKey);
  if (key.type == TBottom) return {TBottom, false};
  if ((dict.m_bits & BDictE) == dict.m_bits) return {TBottom, false};

  const bool maybeEmpty = dict.m_bits & BDictE;

  switch (dict.m_dataTag) {
  case DataTag::ArrVal:
  case DataTag::ArrPacked:
  case DataTag::ArrPackedN:
  case DataTag::ArrStruct:
  case DataTag::ArrMapN:
  case DataTag::VecVal:
  case DataTag::Vec:
  case DataTag::KeysetVal:
  case DataTag::Keyset:
  case DataTag::Str:
  case DataTag::Obj:
  case DataTag::Int:
  case DataTag::Dbl:
  case DataTag::Cls:
  case DataTag::RefInner:
    not_reached();

  case DataTag::None:
    return {TInitCell, false};

  case DataTag::DictVal: {
    auto ad = dict.m_data.aval;
    if (key.i) {
      if (auto const r = ad->nvGet(*key.i)) {
        return {from_cell(*r), !maybeEmpty};
      }
      return {TBottom, false};
    } else if (key.s) {
      if (auto const r = ad->nvGet(*key.s)) {
        return {from_cell(*r), !maybeEmpty};
      }
      return {TBottom, false};
    }
    return {TInitCell, false};
  }
  case DataTag::Dict:
    if (!key.type.couldBe(dict.m_data.dict->key)) return {TBottom, false};
    return {dict.m_data.dict->val, false};
  }
  not_reached();
}

std::pair<Type, bool>
dict_set(Type dict, const Type& undisectedKey, const Type& val) {
  auto const key = disect_strict_key(undisectedKey);
  if (key.type == TBottom) return {TBottom, false};

  const bool validKey = key.type.subtypeOf(TInt) || key.type.subtypeOf(TStr);
  dict.m_bits = !(dict.m_bits & BSDict) ? BCDictN : BDictN;

  switch (dict.m_dataTag) {
  case DataTag::ArrVal:
  case DataTag::ArrPacked:
  case DataTag::ArrPackedN:
  case DataTag::ArrStruct:
  case DataTag::ArrMapN:
  case DataTag::VecVal:
  case DataTag::Vec:
  case DataTag::KeysetVal:
  case DataTag::Keyset:
  case DataTag::Str:
  case DataTag::Obj:
  case DataTag::Int:
  case DataTag::Dbl:
  case DataTag::Cls:
  case DataTag::RefInner:
    not_reached();

  case DataTag::None:
    return {std::move(dict), validKey};

  case DataTag::DictVal: {
    dict = toDDictType(dict);
    /* fallthrough */
  }
  case DataTag::Dict:
    auto& kt = dict.m_data.dict->key;
    auto& vt = dict.m_data.dict->val;
    kt = union_of(std::move(kt), key.type);
    vt = union_of(std::move(vt), val);
    return {std::move(dict), validKey};
  }
  not_reached();
}

Type dict_newelem(Type dict, const Type& val) {
  dict.m_bits = !(dict.m_bits & BSDict) ? BCDictN : BDictN;

  switch (dict.m_dataTag) {
  case DataTag::ArrVal:
  case DataTag::ArrPacked:
  case DataTag::ArrPackedN:
  case DataTag::ArrStruct:
  case DataTag::ArrMapN:
  case DataTag::VecVal:
  case DataTag::Vec:
  case DataTag::KeysetVal:
  case DataTag::Keyset:
  case DataTag::Str:
  case DataTag::Obj:
  case DataTag::Int:
  case DataTag::Dbl:
  case DataTag::Cls:
  case DataTag::RefInner:
    not_reached();

  case DataTag::None:
    return dict;

  case DataTag::DictVal: {
    dict = toDDictType(dict);
    /* fallthrough */
  }
  case DataTag::Dict:
    auto& kt = dict.m_data.dict->key;
    auto& vt = dict.m_data.dict->val;
    kt = union_of(std::move(kt), TInt);
    vt = union_of(std::move(vt), val);
    return dict;
  }
  not_reached();
}

//////////////////////////////////////////////////////////////////////

std::pair<Type, bool>
keyset_elem(const Type& keyset, const Type& undisectedKey) {
  auto const key = disect_strict_key(undisectedKey);
  if (key.type == TBottom) return {TBottom, false};
  if ((keyset.m_bits & BKeysetE) == keyset.m_bits) return {TBottom, false};

  const bool maybeEmpty = keyset.m_bits & BKeysetE;

  switch (keyset.m_dataTag) {
  case DataTag::ArrVal:
  case DataTag::ArrPacked:
  case DataTag::ArrPackedN:
  case DataTag::ArrStruct:
  case DataTag::ArrMapN:
  case DataTag::DictVal:
  case DataTag::Dict:
  case DataTag::VecVal:
  case DataTag::Vec:
  case DataTag::Str:
  case DataTag::Obj:
  case DataTag::Int:
  case DataTag::Dbl:
  case DataTag::Cls:
  case DataTag::RefInner:
    not_reached();

  case DataTag::None:
    return {TInitCell, false};

  case DataTag::KeysetVal: {
    auto ad = keyset.m_data.aval;
    if (key.i) {
      if (auto const r = ad->nvGet(*key.i)) {
        return {from_cell(*r), !maybeEmpty};
      }
      return {TBottom, false};
    } else if (key.s) {
      if (auto const r = ad->nvGet(*key.s)) {
        return {from_cell(*r), !maybeEmpty};
      }
      return {TBottom, false};
    }
    return {TInitCell, false};
  }
  case DataTag::Keyset:
    if (!key.type.couldBe(keyset.m_data.keyset->keyval)) {
      return {TBottom, false};
    }
    return {keyset.m_data.keyset->keyval, false};
  }
  not_reached();
}

std::pair<Type, bool>
keyset_set(Type keyset, const Type&, const Type&) {
  // The set operation on keysets is not allowed.
  assert(keyset.m_dataTag == DataTag::None ||
         keyset.m_dataTag == DataTag::KeysetVal ||
         keyset.m_dataTag == DataTag::Keyset);
  return {TBottom, false};
}

Type keyset_newelem(Type keyset, const Type& val) {
  keyset.m_bits = !(keyset.m_bits & BSKeyset) ? BCKeysetN : BKeysetN;

  switch (keyset.m_dataTag) {
  case DataTag::ArrVal:
  case DataTag::ArrPacked:
  case DataTag::ArrPackedN:
  case DataTag::ArrStruct:
  case DataTag::ArrMapN:
  case DataTag::DictVal:
  case DataTag::Dict:
  case DataTag::VecVal:
  case DataTag::Vec:
  case DataTag::Str:
  case DataTag::Obj:
  case DataTag::Int:
  case DataTag::Dbl:
  case DataTag::Cls:
  case DataTag::RefInner:
    not_reached();

  case DataTag::None:
    return keyset;

  case DataTag::KeysetVal: {
    keyset = toDKeysetType(keyset);
    /* fallthrough */
  }
  case DataTag::Keyset:
    auto& ty = keyset.m_data.keyset->keyval;
    ty = union_of(std::move(ty), val);
    return keyset;
  }
  not_reached();
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
    case DataTag::Vec:
    case DataTag::Dict:
    case DataTag::Keyset:
    case DataTag::VecVal:
    case DataTag::DictVal:
    case DataTag::KeysetVal:
    case DataTag::ArrVal:
    case DataTag::ArrStruct:
    case DataTag::ArrMapN:
      return nullptr;
    case DataTag::ArrPackedN:
      // TODO(#4205897): we need to use this before it's worth putting
      // in the repo.
      if (false) {
        return arrTable.packedn(
          emptiness,
          make_repo_type(arrTable, t.m_data.apackedn->type)
        );
      }
      return nullptr;
    case DataTag::ArrPacked:
      {
        std::vector<RepoAuthType> repoTypes;
        std::transform(
          begin(t.m_data.apacked->elems), end(t.m_data.apacked->elems),
          std::back_inserter(repoTypes),
          [&] (const Type& t) { return make_repo_type(arrTable, t); }
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

  if (t.strictSubtypeOf(TArr) ||
      // TODO(#4205897): optional array types.
      (false && is_opt(t) && t.strictSubtypeOf(TOptArr))) {
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
