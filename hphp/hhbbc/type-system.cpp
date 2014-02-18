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
#include "hphp/hhbbc/type-system.h"

#include <type_traits>
#include <cmath>
#include <algorithm>

#include "folly/Optional.h"
#include "folly/Traits.h"

#include "hphp/runtime/base/array-iterator.h"
#include "hphp/hhbbc/index.h"

namespace HPHP { namespace HHBBC {

TRACE_SET_MOD(hhbbc);

namespace {

//////////////////////////////////////////////////////////////////////

const StaticString s_WaitHandle("WaitHandle");

//////////////////////////////////////////////////////////////////////

// Legal to call with !isPredefined(bits)
bool mayHaveData(trep bits) {
  switch (bits) {
  case BSStr:    case BObj:    case BInt:    case BDbl:
  case BOptSStr: case BOptObj: case BOptInt: case BOptDbl:
  case BCls:
  case BArr:    case BSArr:    case BCArr:
  case BOptArr: case BOptSArr: case BOptCArr:
    return true;
  default:
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
  case BSArr:
  case BCArr:
  case BObj:
  case BRes:
  case BCls:
  case BRef:
  case BNull:
  case BNum:
  case BBool:
  case BStr:
  case BArr:
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
  case BOptSArr:
  case BOptCArr:
  case BOptArr:
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
  case BSArr:
  case BCArr:
  case BObj:
  case BRes:
    return true;

  case BNull:
  case BNum:
  case BBool:
  case BStr:
  case BArr:
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
  case BOptSArr:
  case BOptCArr:
  case BOptArr:
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

//////////////////////////////////////////////////////////////////////

/*
 * The following functions make DArr* structs out of static arrays, to
 * simplify implementing some of the type system operations on them.
 *
 * When they return folly::none it is not a conservative thing: it
 * implies the array is definitely not packed, packedN, struct-like,
 * etc (we use this to return false in couldBe).
 *
 * This also means they should check for the empty array and return
 * none in that case to add some false cases for couldBe.
 */

folly::Optional<DArrPacked> toDArrPacked(SArray ar) {
  if (!ar->size()) return folly::none;

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
  if (!ar->size()) return folly::none;

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
  if (!ar->size()) return folly::none;

  auto map = StructMap{};
  for (ArrayIter iter(ar); iter; ++iter) {
    auto const key = *iter.first().asTypedValue();
    if (!IS_STRING_TYPE(key.m_type)) return folly::none;
    map[key.m_data.pstr] = from_cell(*iter.secondRef().asTypedValue());
  }

  return DArrStruct { std::move(map) };
}

folly::Optional<DArrMapN> toDArrMapN(SArray ar) {
  if (!ar->size()) return folly::none;

  auto k = TBottom;
  auto v = TBottom;
  for (ArrayIter iter(ar); iter; ++iter) {
    auto const key = *iter.first().asTypedValue();
    k = union_of(k, from_cell(key));
    v = union_of(v, from_cell(*iter.secondRef().asTypedValue()));
  }

  return DArrMapN { k, v };
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

//////////////////////////////////////////////////////////////////////

/*
 * Helper for dealing with disjointDataFn's---most of them are
 * commutative.  This shuffles values to the right in a canonical
 * order to need less overloads.
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
    return p && p->key.couldBe(a.key) && p->val.couldBe(a.val);
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

// The countedness of the arrays is handled outside of this function,
// so it's ok to just return TArr from all of these here.
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
    return TArr;
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
    if (!p) {
      // We could return an array type that knows it can't contain
      // refs (but could be an empty array), but we don't have one
      // yet.
      return TArr;
    }
    return arr_mapn(union_of(a.key, p->key), union_of(a.val, p->val));
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
    return p && p->key.subtypeOf(b.key) && p->val.subtypeOf(b.val);
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
  switch (m_dataTag) {
  case DataTag::None:   return;
  case DataTag::Str:    m_data.sval = o.m_data.sval; return;
  case DataTag::ArrVal: m_data.aval = o.m_data.aval; return;
  case DataTag::Int:    m_data.ival = o.m_data.ival; return;
  case DataTag::Dbl:    m_data.dval = o.m_data.dval; return;
  case DataTag::Obj:    new (&m_data.dobj) DObj(o.m_data.dobj); return;
  case DataTag::Cls:    new (&m_data.dcls) DCls(o.m_data.dcls); return;
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
  }
  not_reached();
}

Type::Type(Type&& o) noexcept
  : m_bits(o.m_bits)
  , m_dataTag(o.m_dataTag)
{
  using std::move;
  o.m_dataTag = DataTag::None;
  switch (m_dataTag) {
  case DataTag::None:   return;
  case DataTag::Str:    m_data.sval = o.m_data.sval; return;
  case DataTag::ArrVal: m_data.aval = o.m_data.aval; return;
  case DataTag::Int:    m_data.ival = o.m_data.ival; return;
  case DataTag::Dbl:    m_data.dval = o.m_data.dval; return;
  case DataTag::Obj:    new (&m_data.dobj) DObj(move(o.m_data.dobj)); return;
  case DataTag::Cls:    new (&m_data.dcls) DCls(move(o.m_data.dcls)); return;
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
  }
  not_reached();
}

Type& Type::operator=(const Type& o) noexcept {
  this->~Type();
  new (this) Type(o);
  return *this;
}

Type& Type::operator=(Type&& o) noexcept {
  this->~Type();
  new (this) Type(std::move(o));
  return *this;
}

Type::~Type() noexcept {
  assert(checkInvariants());

  switch (m_dataTag) {
  case DataTag::None:
  case DataTag::Str:
  case DataTag::ArrVal:
  case DataTag::Int:
  case DataTag::Dbl:
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
Type::DJHelperFn<Ret,T,Function> djbind(const Function& f, const T& t) {
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
  case DataTag::ArrVal:
    return m_data.aval == o.m_data.aval;
  case DataTag::Int:
    return m_data.ival == o.m_data.ival;
  case DataTag::Dbl:
    // For purposes of Type equivalence, NaNs are equal.
    return m_data.dval == o.m_data.dval ||
           (std::isnan(m_data.dval) && std::isnan(o.m_data.dval));
  case DataTag::Obj:
    return m_data.dobj.type == o.m_data.dobj.type &&
           m_data.dobj.cls.same(o.m_data.dobj.cls);
  case DataTag::Cls:
    return m_data.dcls.type == o.m_data.dcls.type &&
           m_data.dcls.cls.same(o.m_data.dcls.cls);
  case DataTag::ArrPacked:
    return m_data.apacked->elems == o.m_data.apacked->elems;
  case DataTag::ArrPackedN:
    return m_data.apackedn->type == o.m_data.apackedn->type;
  case DataTag::ArrStruct:
    return m_data.astruct->map == o.m_data.astruct->map;
  case DataTag::ArrMapN:
    return m_data.amapn->key == o.m_data.amapn->key &&
           m_data.amapn->val == o.m_data.amapn->val;
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
  case DataTag::ArrVal:
  case DataTag::Int:
  case DataTag::Dbl:
  case DataTag::None:
    return equivData(o);
  case DataTag::ArrPacked:
    return subtypePacked(*m_data.apacked, *o.m_data.apacked);
  case DataTag::ArrPackedN:
    return m_data.apackedn->type.subtypeOf(o.m_data.apackedn->type);
  case DataTag::ArrStruct:
    return subtypeStruct(*m_data.astruct, *o.m_data.astruct);
  case DataTag::ArrMapN:
    return m_data.amapn->key.subtypeOf(o.m_data.amapn->key) &&
           m_data.amapn->val.subtypeOf(o.m_data.amapn->val);
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
  }
  not_reached();
}

bool Type::operator==(Type o) const {
  assert(checkInvariants());
  assert(o.checkInvariants());
  if (m_bits != o.m_bits) return false;
  if (hasData() != o.hasData()) return false;
  if (!hasData() && !o.hasData()) return true;
  return equivData(o);
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
  case DataTag::ArrVal: assert(m_data.aval->isStatic()); break;
  case DataTag::Dbl:    break;
  case DataTag::Int:    break;
  case DataTag::Cls:    break;
  case DataTag::Obj:
    if (auto t = m_data.dobj.whType.get()) {
      t->checkInvariants();
    }
    break;
  case DataTag::ArrPacked:
    assert(!m_data.apacked->elems.empty());
    break;
  case DataTag::ArrStruct:
    assert(!m_data.astruct->map.empty());
    break;
  case DataTag::ArrPackedN:
  case DataTag::ArrMapN:
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
  r.m_dataTag   = DataTag::Str;
  r.m_data.sval = val;
  return r;
}

Type ival(int64_t val) {
  auto r        = Type { BInt };
  r.m_dataTag   = DataTag::Int;
  r.m_data.ival = val;
  return r;
}

Type dval(double val) {
  auto r        = Type { BDbl };
  r.m_dataTag   = DataTag::Dbl;
  r.m_data.dval = val;
  return r;
}

Type aval(SArray val) {
  assert(val->isStatic());
  auto r        = Type { BSArr };
  r.m_dataTag   = DataTag::ArrVal;
  r.m_data.aval = val;
  return r;
}

Type subObj(res::Class val) {
  auto r        = Type { BObj };
  r.m_dataTag   = DataTag::Obj;
  new (&r.m_data.dobj) DObj(
    val.couldBeOverriden() ? DObj::Sub : DObj::Exact,
    val
  );
  return r;
}

Type objExact(res::Class val) {
  auto r        = Type { BObj };
  r.m_dataTag   = DataTag::Obj;
  new (&r.m_data.dobj) DObj(DObj::Exact, val);
  return r;
}

Type subCls(res::Class val) {
  auto r        = Type { BCls };
  r.m_dataTag   = DataTag::Cls;
  r.m_data.dcls = DCls {
    val.couldBeOverriden() ? DCls::Sub : DCls::Exact,
    val
  };
  return r;
}

Type clsExact(res::Class val) {
  auto r        = Type { BCls };
  r.m_dataTag   = DataTag::Cls;
  r.m_data.dcls = DCls { DCls::Exact, val };
  return r;
}

bool is_specialized_array(const Type& t) {
  switch (t.m_dataTag) {
  case DataTag::None:
  case DataTag::Str:
  case DataTag::Obj:
  case DataTag::Int:
  case DataTag::Dbl:
  case DataTag::Cls:
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

Type arr_packed(std::vector<Type> elems) {
  assert(!elems.empty());
  auto r      = Type { BArr };
  r.m_dataTag = DataTag::ArrPacked;
  new (&r.m_data.apacked) copy_ptr<DArrPacked>(
    make_copy_ptr<DArrPacked>(std::move(elems))
  );
  return r;
}

Type sarr_packed(std::vector<Type> elems) {
  auto t = arr_packed(std::move(elems));
  t.m_bits = BSArr;
  return t;
}

Type carr_packed(std::vector<Type> elems) {
  auto t = arr_packed(std::move(elems));
  t.m_bits = BCArr;
  return t;
}

Type arr_packedn(Type t) {
  auto r      = Type { BArr };
  r.m_dataTag = DataTag::ArrPackedN;
  new (&r.m_data.apackedn) copy_ptr<DArrPackedN>(
    make_copy_ptr<DArrPackedN>(std::move(t))
  );
  return r;
}

Type sarr_packedn(Type t) {
  auto r = arr_packedn(std::move(t));
  r.m_bits = BSArr;
  return r;
}

Type carr_packedn(Type t) {
  auto r = arr_packedn(std::move(t));
  r.m_bits = BCArr;
  return r;
}

Type arr_struct(StructMap m) {
  assert(!m.empty());
  auto r      = Type { BArr };
  r.m_dataTag = DataTag::ArrStruct;
  new (&r.m_data.astruct) copy_ptr<DArrStruct>(
    make_copy_ptr<DArrStruct>(std::move(m))
  );
  return r;
}

Type sarr_struct(StructMap m) {
  auto r = arr_struct(std::move(m));
  r.m_bits = BSArr;
  return r;
}

Type carr_struct(StructMap m) {
  auto r = arr_struct(std::move(m));
  r.m_bits = BCArr;
  return r;
}

Type arr_mapn(Type k, Type v) {
  auto r      = Type { BArr };
  r.m_dataTag = DataTag::ArrMapN;
  new (&r.m_data.amapn) copy_ptr<DArrMapN>(
    make_copy_ptr<DArrMapN>(std::move(k), std::move(v))
  );
  return r;
}

Type sarr_mapn(Type k, Type v) {
  auto r = arr_mapn(k, v);
  r.m_bits = BSArr;
  return r;
}

Type carr_mapn(Type k, Type v) {
  auto r = arr_mapn(std::move(k), std::move(v));
  r.m_bits = BCArr;
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

bool is_specialized_obj(Type t) {
  return t.strictSubtypeOf(TObj) ||
    (is_opt(t) && unopt(t).strictSubtypeOf(TObj));
}

Type objcls(Type t) {
  assert(t.subtypeOf(TObj));
  if (t.strictSubtypeOf(TObj)) {
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
  default:
    if (is_opt(t)) {
      break;
    }
    switch (t.m_dataTag) {
    case DataTag::Int:    return make_tv<KindOfInt64>(t.m_data.ival);
    case DataTag::Dbl:    return make_tv<KindOfDouble>(t.m_data.dval);
    case DataTag::Str:    return make_tv<KindOfStaticString>(t.m_data.sval);
    case DataTag::ArrVal: return make_tv<KindOfArray>(
                            const_cast<ArrayData*>(t.m_data.aval)
                          );
    case DataTag::ArrStruct:
    case DataTag::ArrPacked:
      // TODO(#3696042): we could materialize a static array here if
      // we check if a whole specialized array is constants.
      break;
    case DataTag::ArrPackedN:
    case DataTag::ArrMapN:
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
  case IsTypeOp::Null:   return TNull;
  case IsTypeOp::Bool:   return TBool;
  case IsTypeOp::Int:    return TInt;
  case IsTypeOp::Dbl:    return TDbl;
  case IsTypeOp::Str:    return TStr;
  case IsTypeOp::Arr:    return TArr;
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
  assert(t.strictSubtypeOf(TCls));
  assert(t.m_dataTag == DataTag::Cls);
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

  case KindOfStaticString:
  case KindOfString:
    always_assert(cell.m_data.pstr->isStatic());
    return sval(cell.m_data.pstr);

  case KindOfArray:
    always_assert(cell.m_data.parr->isStatic());
    return aval(cell.m_data.parr);

  case KindOfRef:
  case KindOfObject:
  case KindOfResource:
  default:
    break;
  }
  always_assert(0 && "reference counted type in from_cell");
}

Type from_DataType(DataType dt) {
  switch (dt) {
  case KindOfUninit:   return TUninit;
  case KindOfNull:     return TInitNull;
  case KindOfBoolean:  return TBool;
  case KindOfInt64:    return TInt;
  case KindOfDouble:   return TDbl;
  case KindOfStaticString:
  case KindOfString:   return TStr;
  case KindOfArray:    return TArr;
  case KindOfRef:      return TRef;
  case KindOfObject:   return TObj;
  case KindOfResource: return TRes;
  default:
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

  if (!strcmp(p, "resource")) return union_of(ret, TRes);
  if (!strcmp(p, "bool"))     return union_of(ret, TBool);
  if (!strcmp(p, "int"))      return union_of(ret, TInt);
  if (!strcmp(p, "float"))    return union_of(ret, TDbl);
  if (!strcmp(p, "num"))      return union_of(ret, TNum);
  if (!strcmp(p, "string"))   return union_of(ret, TStr);
  if (!strcmp(p, "array"))    return union_of(ret, TArr);
  if (!strcmp(p, "mixed"))    return TInitGen;

  // It might be an object, or we might want to support type aliases
  // in HNI at some point.  For now just be conservative.
  return TGen;
}

Type Type::unionArr(const Type& a, const Type& b) {
  assert(!a.subtypeOf(b));
  assert(!b.subtypeOf(a));
  assert(is_specialized_array(a));
  assert(is_specialized_array(b));

  auto ret = Type{};
  auto const newBits = static_cast<trep>(a.m_bits | b.m_bits);

  if (a.m_dataTag != b.m_dataTag) {
    ret = a.disjointDataFn(b, ArrUnion{});
    ret.m_bits = newBits;
    return ret;
  }

  switch (a.m_dataTag) {
  case DataTag::None:
  case DataTag::Str:
  case DataTag::Obj:
  case DataTag::Int:
  case DataTag::Dbl:
  case DataTag::Cls:
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
   * We need to check this before specialized objects, including the
   * case where one of them was TInitNull, because otherwise we'll go
   * down the is_specialized_obj paths and lose the wait handle
   * information.
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

  if (is_specialized_array(a) && is_specialized_array(b)) {
    DEBUG_ONLY auto const shouldBeOpt = is_opt(a) || is_opt(b);
    auto const t = Type::unionArr(a, b);
    if (shouldBeOpt) assert(is_opt(t));
    return t;
  }

#define X(y) if (a.subtypeOf(y) && b.subtypeOf(y)) return y;
  X(TInt)
  X(TDbl)
  X(TSStr)
  X(TCStr)
  X(TSArr)
  X(TCArr)
  X(TObj)
  X(TCls)
  X(TNull)
  X(TBool)
  X(TNum)
  X(TStr)
  X(TArr)

  /*
   * Merging option types tries to preserve subtype information where
   * it's possible.  E.g. if you union InitNull and Obj<=Foo, we want
   * OptObj<=Foo to be the result.
   */
  if (a == TInitNull && canBeOptional(b.m_bits)) return opt(b);
  if (b == TInitNull && canBeOptional(a.m_bits)) return opt(a);

  // A few optional unions still need to be checked despite the above
  // (e.g. if we are merging TOptTrue and TOptFalse, we want TOptBool,
  // but neither was TInitNull).
  X(TOptBool)
  X(TOptNum)
  X(TOptStr)
  X(TOptArr)

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

Type stack_flav(Type a) {
  if (a.subtypeOf(TUninit))   return TUninit;
  if (a.subtypeOf(TInitCell)) return TInitCell;
  if (a.subtypeOf(TRef))      return TRef;
  if (a.subtypeOf(TGen))      return TGen;
  if (a.subtypeOf(TCls))      return TCls;
  always_assert(0 && "stack_flav passed invalid type");
}

Type loosen_statics(Type a) {
  if (a.couldBe(TSStr)) a = union_of(a, TStr);
  if (a.couldBe(TSArr)) a = union_of(a, TArr);
  return a;
}

Type loosen_values(Type a) {
  return a.strictSubtypeOf(TInt) ? TInt :
         a.strictSubtypeOf(TDbl) ? TDbl :
         a.strictSubtypeOf(TBool) ? TBool :
         a.strictSubtypeOf(TSStr) ? TSStr :
         a.strictSubtypeOf(TSArr) ? TSArr :
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

}}
