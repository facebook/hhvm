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
#include "hphp/runtime/base/tv-comparisons.h"

#include <type_traits>

#include "hphp/runtime/base/tv-conversions.h"
#include "hphp/runtime/base/comparisons.h"
#include "hphp/runtime/base/type-conversions.h"
#include "hphp/runtime/base/mixed-array.h"
#include "hphp/runtime/base/packed-array.h"
#include "hphp/runtime/base/set-array.h"
#include "hphp/runtime/ext/datetime/ext_datetime.h"

namespace HPHP {

//////////////////////////////////////////////////////////////////////
namespace collections {
extern bool equals(const ObjectData*, const ObjectData*);
}
//////////////////////////////////////////////////////////////////////

namespace {

//////////////////////////////////////////////////////////////////////

/*
 * Family of relative op functions.
 *
 * These are used to implement the common parts of the php operators
 * ==, <, and >.  They handle some of the php behavior with regard to
 * numeric-ish strings, and delegate to the 'op' functor to perform
 * the actual comparison on primitive types, and between complex php
 * types of the same type.
 *
 * See below for the implementations of the Op template parameter.
 */

template<class Op>
typename Op::RetType cellRelOp(Op op, Cell cell, bool val) {
  if (UNLIKELY(isVecType(cell.m_type))) {
    return op.vecVsNonVec();
  } else if (UNLIKELY(isDictType(cell.m_type))) {
    return op.dictVsNonDict();
  } else if (UNLIKELY(isKeysetType(cell.m_type))) {
    return op.keysetVsNonKeyset();
  } else {
    return op(cellToBool(cell), val);
  }
}

template<class Op>
typename Op::RetType cellRelOp(Op op, Cell cell, int64_t val) {
  assert(cellIsPlausible(cell));

  switch (cell.m_type) {
    case KindOfUninit:
    case KindOfNull:
      return op(false, !!val);

    case KindOfBoolean:
      return op(!!cell.m_data.num, val != 0);

    case KindOfInt64:
      return op(cell.m_data.num, val);

    case KindOfDouble:
      return op(cell.m_data.dbl, val);

    case KindOfPersistentString:
    case KindOfString: {
      auto const num = stringToNumeric(cell.m_data.pstr);
      return num.m_type == KindOfInt64  ? op(num.m_data.num, val) :
             num.m_type == KindOfDouble ? op(num.m_data.dbl, val) :
             op(0, val);
    }

    case KindOfPersistentVec:
    case KindOfVec:
      return op.vecVsNonVec();

    case KindOfPersistentDict:
    case KindOfDict:
      return op.dictVsNonDict();

    case KindOfPersistentKeyset:
    case KindOfKeyset:
      return op.keysetVsNonKeyset();

    case KindOfPersistentArray:
    case KindOfArray:
      return op(true, false);

    case KindOfObject:
      return cell.m_data.pobj->isCollection()
        ? op.collectionVsNonObj()
        : op(cell.m_data.pobj->toInt64(), val);

    case KindOfResource:
      return op(cell.m_data.pres->data()->o_toInt64(), val);

    case KindOfRef:
    case KindOfClass:
      break;
  }
  not_reached();
}

template<class Op>
typename Op::RetType cellRelOp(Op op, Cell cell, double val) {
  assert(cellIsPlausible(cell));

  switch (cell.m_type) {
    case KindOfUninit:
    case KindOfNull:
      return op(false, val != 0);

    case KindOfBoolean:
      return op(!!cell.m_data.num, val != 0);

    case KindOfInt64:
      return op(cell.m_data.num, val);

    case KindOfDouble:
      return op(cell.m_data.dbl, val);

    case KindOfPersistentString:
    case KindOfString: {
      auto const num = stringToNumeric(cell.m_data.pstr);
      return num.m_type == KindOfInt64  ? op(num.m_data.num, val) :
             num.m_type == KindOfDouble ? op(num.m_data.dbl, val) :
             op(0, val);
    }

    case KindOfPersistentVec:
    case KindOfVec:
      return op.vecVsNonVec();

    case KindOfPersistentDict:
    case KindOfDict:
      return op.dictVsNonDict();

    case KindOfPersistentKeyset:
    case KindOfKeyset:
      return op.keysetVsNonKeyset();

    case KindOfPersistentArray:
    case KindOfArray:
      return op(true, false);

    case KindOfObject:
      return cell.m_data.pobj->isCollection()
        ? op.collectionVsNonObj()
        : op(cell.m_data.pobj->toDouble(), val);

    case KindOfResource:
      return op(cell.m_data.pres->data()->o_toDouble(), val);

    case KindOfRef:
    case KindOfClass:
      break;
  }
  not_reached();
}

template<class Op>
typename Op::RetType cellRelOp(Op op, Cell cell, const StringData* val) {
  assert(cellIsPlausible(cell));

  switch (cell.m_type) {
    case KindOfUninit:
    case KindOfNull:
      return op(staticEmptyString(), val);

    case KindOfInt64: {
      auto const num = stringToNumeric(val);
      return num.m_type == KindOfInt64  ? op(cell.m_data.num, num.m_data.num) :
             num.m_type == KindOfDouble ? op(cell.m_data.num, num.m_data.dbl) :
             op(cell.m_data.num, 0);
    }
    case KindOfBoolean:
      return op(!!cell.m_data.num, toBoolean(val));

    case KindOfDouble: {
      auto const num = stringToNumeric(val);
      return num.m_type == KindOfInt64  ? op(cell.m_data.dbl, num.m_data.num) :
             num.m_type == KindOfDouble ? op(cell.m_data.dbl, num.m_data.dbl) :
             op(cell.m_data.dbl, 0);
    }

    case KindOfPersistentString:
    case KindOfString:
      return op(cell.m_data.pstr, val);

    case KindOfPersistentVec:
    case KindOfVec:
      return op.vecVsNonVec();

    case KindOfPersistentDict:
    case KindOfDict:
      return op.dictVsNonDict();

    case KindOfPersistentKeyset:
    case KindOfKeyset:
      return op.keysetVsNonKeyset();

    case KindOfPersistentArray:
    case KindOfArray:
      return op(true, false);

    case KindOfObject: {
      auto od = cell.m_data.pobj;
      if (od->isCollection()) return op.collectionVsNonObj();
      if (od->hasToString()) {
        String str(od->invokeToString());
        return op(str.get(), val);
      }
      return op(true, false);
    }

    case KindOfResource: {
      auto const rd = cell.m_data.pres;
      return op(rd->data()->o_toDouble(), val->toDouble());
    }

    case KindOfRef:
    case KindOfClass:
      break;
  }
  not_reached();
}

template<class Op>
typename Op::RetType cellRelOp(Op op, Cell cell, const ArrayData* ad) {
  assert(cellIsPlausible(cell));

  assert(ad->isPHPArray());

  switch (cell.m_type) {
    case KindOfUninit:
    case KindOfNull:
      return op(false, !ad->empty());

    case KindOfBoolean:
      return op(cell.m_data.num, !ad->empty());

    case KindOfInt64:
      return op(false, true);

    case KindOfDouble:
      return op(false, true);

    case KindOfPersistentString:
    case KindOfString:
      return op(false, true);

    case KindOfPersistentVec:
    case KindOfVec:
      return op.vecVsNonVec();

    case KindOfPersistentDict:
    case KindOfDict:
      return op.dictVsNonDict();

    case KindOfPersistentKeyset:
    case KindOfKeyset:
      return op.keysetVsNonKeyset();

    case KindOfPersistentArray:
    case KindOfArray:
      return op(cell.m_data.parr, ad);

    case KindOfObject: {
      auto const od = cell.m_data.pobj;
      return od->isCollection()
        ? op.collectionVsNonObj()
        : op(true, false);
    }

    case KindOfResource:
      return op(false, true);

    case KindOfRef:
    case KindOfClass:
      break;
  }
  not_reached();
}

template<class Op>
typename Op::RetType cellRelOp(Op op, Cell cell, const ObjectData* od) {
  assert(cellIsPlausible(cell));

  switch (cell.m_type) {
    case KindOfUninit:
    case KindOfNull:
      return op(false, true);

    case KindOfBoolean:
      return op(!!cell.m_data.num, od->toBoolean());

    case KindOfInt64:
      return od->isCollection() ? op.collectionVsNonObj()
                                : op(cell.m_data.num, od->toInt64());

    case KindOfDouble:
      return od->isCollection() ? op.collectionVsNonObj()
                                : op(cell.m_data.dbl, od->toDouble());

    case KindOfPersistentString:
    case KindOfString: {
      auto obj = const_cast<ObjectData*>(od);
      if (obj->isCollection()) return op.collectionVsNonObj();
      if (obj->hasToString()) {
        String str(obj->invokeToString());
        return op(cell.m_data.pstr, str.get());
      }
      return op(false, true);
    }

    case KindOfPersistentVec:
    case KindOfVec:
      return op.vecVsNonVec();

    case KindOfPersistentDict:
    case KindOfDict:
      return op.dictVsNonDict();

    case KindOfPersistentKeyset:
    case KindOfKeyset:
      return op.keysetVsNonKeyset();

    case KindOfPersistentArray:
    case KindOfArray:
        return od->isCollection() ? op.collectionVsNonObj() : op(false, true);

    case KindOfObject:
      return op(cell.m_data.pobj, od);

    case KindOfResource:
      return op(false, true);

    case KindOfRef:
    case KindOfClass:
      break;
  }
  not_reached();
}

template<class Op>
typename Op::RetType cellRelOp(Op op, Cell cell, const ResourceData* rd) {
  assert(cellIsPlausible(cell));

  switch (cell.m_type) {
    case KindOfUninit:
    case KindOfNull:
      return op(false, true);

    case KindOfBoolean:
      return op(!!cell.m_data.num, rd->o_toBoolean());

    case KindOfInt64:
      return op(cell.m_data.num, rd->o_toInt64());

    case KindOfDouble:
      return op(cell.m_data.dbl, rd->o_toDouble());

    case KindOfPersistentString:
    case KindOfString: {
      auto const str = cell.m_data.pstr;
      return op(str->toDouble(), rd->o_toDouble());
    }

    case KindOfPersistentVec:
    case KindOfVec:
      return op.vecVsNonVec();

    case KindOfPersistentDict:
    case KindOfDict:
      return op.dictVsNonDict();

    case KindOfPersistentKeyset:
    case KindOfKeyset:
      return op.keysetVsNonKeyset();

    case KindOfPersistentArray:
    case KindOfArray:
      return op(true, false);

    case KindOfObject:
      return op(true, false);

    case KindOfResource:
      return op(cell.m_data.pres->data(), rd);

    case KindOfRef:
    case KindOfClass:
      break;
  }
  not_reached();
}
template<class Op>
typename Op::RetType cellRelOp(Op op, Cell cell, const ResourceHdr* r) {
  return cellRelOp(op, cell, r->data());
}

template<class Op>
typename Op::RetType cellRelOpVec(Op op, Cell cell, const ArrayData* a) {
  assert(cellIsPlausible(cell));
  assert(a->isVecArray());

  if (UNLIKELY(!isVecType(cell.m_type))) {
    if (isDictType(cell.m_type)) return op.dictVsNonDict();
    if (isKeysetType(cell.m_type)) return op.keysetVsNonKeyset();
    return op.vecVsNonVec();
  }
  return op.vec(cell.m_data.parr, a);
}

template<class Op>
typename Op::RetType cellRelOpDict(Op op, Cell cell, const ArrayData* a) {
  assert(cellIsPlausible(cell));
  assert(a->isDict());

  if (UNLIKELY(!isDictType(cell.m_type))) {
    if (isVecType(cell.m_type)) return op.vecVsNonVec();
    if (isKeysetType(cell.m_type)) return op.keysetVsNonKeyset();
    return op.dictVsNonDict();
  }

  return op.dict(cell.m_data.parr, a);
}

template<class Op>
typename Op::RetType cellRelOpKeyset(Op op, Cell cell, const ArrayData* a) {
  assert(cellIsPlausible(cell));
  assert(a->isKeyset());

  if (UNLIKELY(!isKeysetType(cell.m_type))) {
    if (isVecType(cell.m_type)) return op.vecVsNonVec();
    if (isDictType(cell.m_type)) return op.dictVsNonDict();
    return op.keysetVsNonKeyset();
  }

  return op.keyset(cell.m_data.parr, a);
}

template<class Op>
typename Op::RetType cellRelOp(Op op, Cell c1, Cell c2) {
  assert(cellIsPlausible(c1));
  assert(cellIsPlausible(c2));

  switch (c2.m_type) {
  case KindOfUninit:
  case KindOfNull:
    return isStringType(c1.m_type) ? op(c1.m_data.pstr, staticEmptyString()) :
           c1.m_type == KindOfObject ? op(true, false) :
           cellRelOp(op, c1, false);

  case KindOfInt64:        return cellRelOp(op, c1, c2.m_data.num);
  case KindOfBoolean:      return cellRelOp(op, c1, !!c2.m_data.num);
  case KindOfDouble:       return cellRelOp(op, c1, c2.m_data.dbl);
  case KindOfPersistentString:
  case KindOfString:       return cellRelOp(op, c1, c2.m_data.pstr);
  case KindOfPersistentVec:
  case KindOfVec:          return cellRelOpVec(op, c1, c2.m_data.parr);
  case KindOfPersistentDict:
  case KindOfDict:         return cellRelOpDict(op, c1, c2.m_data.parr);
  case KindOfPersistentKeyset:
  case KindOfKeyset:       return cellRelOpKeyset(op, c1, c2.m_data.parr);
  case KindOfPersistentArray:
  case KindOfArray:        return cellRelOp(op, c1, c2.m_data.parr);
  case KindOfObject:       return cellRelOp(op, c1, c2.m_data.pobj);
  case KindOfResource:     return cellRelOp(op, c1, c2.m_data.pres);

  case KindOfRef:
  case KindOfClass:
    break;
  }
  not_reached();
}

template<class Op>
typename Op::RetType tvRelOp(Op op, TypedValue tv1, TypedValue tv2) {
  assert(tvIsPlausible(tv1));
  assert(tvIsPlausible(tv2));
  return cellRelOp(op, *tvToCell(&tv1), *tvToCell(&tv2));
}

/*
 * These relative ops helper function objects define operator() for
 * each primitive type, and for the case of a complex type being
 * compared with itself (that is obj with obj, string with string,
 * array with array).
 *
 * They must also define a function called collectionVsNonObj() which
 * is used when comparing collections with non-object types.  (The obj
 * vs obj function should handle the collection vs collection and
 * collection vs non-collection object cases.)  This is just to handle
 * that php operator == returns false in these cases, while the Lt/Gt
 * operators throw an exception.
 */

struct Eq {
  using RetType = bool;

  template<class T, class U>
  typename std::enable_if<
    !std::is_pointer<T>::value &&
    !std::is_pointer<U>::value,
    bool
  >::type operator()(T t, U u) const { return t == u; }

  bool operator()(const StringData* sd1, const StringData* sd2) const {
    return sd1->equal(sd2);
  }

  bool operator()(const ArrayData* ad1, const ArrayData* ad2) const {
    assert(ad1->isPHPArray());
    assert(ad2->isPHPArray());
    return ArrayData::Equal(ad1, ad2);
  }

  bool operator()(const ObjectData* od1, const ObjectData* od2) const {
    assert(od1);
    assert(od2);
    return od1->equal(*od2);
  }

  bool operator()(const ResourceData* od1, const ResourceData* od2) const {
    return od1 == od2;
  }
  bool operator()(const ResourceHdr* od1, const ResourceHdr* od2) const {
    return od1 == od2;
  }

  bool vec(const ArrayData* ad1, const ArrayData* ad2) const {
    assert(ad1->isVecArray());
    assert(ad2->isVecArray());
    return PackedArray::VecEqual(ad1, ad2);
  }
  bool dict(const ArrayData* ad1, const ArrayData* ad2) const {
    assert(ad1->isDict());
    assert(ad2->isDict());
    return MixedArray::DictEqual(ad1, ad2);
  }
  bool keyset(const ArrayData* ad1, const ArrayData* ad2) const {
    assert(ad1->isKeyset());
    assert(ad2->isKeyset());
    return SetArray::Equal(ad1, ad2);
  }

  bool vecVsNonVec() const { return false; }
  bool dictVsNonDict() const { return false; }
  bool keysetVsNonKeyset() const { return false; }
  bool collectionVsNonObj() const { return false; }
};

struct Lt {
  using RetType = bool;

  template<class T, class U>
  typename std::enable_if<
    !std::is_pointer<T>::value &&
    !std::is_pointer<U>::value,
    bool
  >::type operator()(T t, U u) const { return t < u; }

  bool operator()(const StringData* sd1, const StringData* sd2) const {
    return sd1->compare(sd2) < 0;
  }

  bool operator()(const ArrayData* ad1, const ArrayData* ad2) const {
    assert(ad1->isPHPArray());
    assert(ad2->isPHPArray());
    return ArrayData::Lt(ad1, ad2);
  }

  bool operator()(const ObjectData* od1, const ObjectData* od2) const {
    assert(od1);
    assert(od2);
    return od1->less(*od2);
  }

  bool operator()(const ResourceData* rd1, const ResourceData* rd2) const {
    return rd1->o_toInt64() < rd2->o_toInt64();
  }
  bool operator()(const ResourceHdr* rd1, const ResourceHdr* rd2) const {
    return rd1->data()->o_toInt64() < rd2->data()->o_toInt64();
  }

  bool vec(const ArrayData* ad1, const ArrayData* ad2) const {
    assert(ad1->isVecArray());
    assert(ad2->isVecArray());
    return PackedArray::VecLt(ad1, ad2);
  }
  bool dict(const ArrayData* ad1, const ArrayData* ad2) const {
    assert(ad1->isDict());
    assert(ad2->isDict());
    throw_dict_compare_exception();
  }
  bool keyset(const ArrayData* ad1, const ArrayData* ad2) const {
    assert(ad1->isKeyset());
    assert(ad2->isKeyset());
    throw_keyset_compare_exception();
  }

  bool vecVsNonVec() const {
    throw_vec_compare_exception();
  }
  bool dictVsNonDict() const {
    throw_dict_compare_exception();
  }
  bool keysetVsNonKeyset() const {
    throw_keyset_compare_exception();
  }
  bool collectionVsNonObj() const {
    throw_collection_compare_exception();
  }
};

struct Gt {
  using RetType = bool;

  template<class T, class U>
  typename std::enable_if<
    !std::is_pointer<T>::value &&
    !std::is_pointer<U>::value,
    bool
  >::type operator()(T t, U u) const { return t > u; }

  bool operator()(const StringData* sd1, const StringData* sd2) const {
    return sd1->compare(sd2) > 0;
  }

  bool operator()(const ArrayData* ad1, const ArrayData* ad2) const {
    assert(ad1->isPHPArray());
    assert(ad2->isPHPArray());
    return ArrayData::Gt(ad1, ad2);
  }

  bool operator()(const ObjectData* od1, const ObjectData* od2) const {
    assert(od1);
    assert(od2);
    return od1->more(*od2);
  }

  bool operator()(const ResourceData* rd1, const ResourceData* rd2) const {
    return rd1->o_toInt64() > rd2->o_toInt64();
  }
  bool operator()(const ResourceHdr* rd1, const ResourceHdr* rd2) const {
    return rd1->data()->o_toInt64() > rd2->data()->o_toInt64();
  }

  bool vec(const ArrayData* ad1, const ArrayData* ad2) const {
    assert(ad1->isVecArray());
    assert(ad2->isVecArray());
    return PackedArray::VecGt(ad1, ad2);
  }
  bool dict(const ArrayData* ad1, const ArrayData* ad2) const {
    assert(ad1->isDict());
    assert(ad2->isDict());
    throw_dict_compare_exception();
  }
  bool keyset(const ArrayData* ad1, const ArrayData* ad2) const {
    assert(ad1->isKeyset());
    assert(ad2->isKeyset());
    throw_keyset_compare_exception();
  }

  bool vecVsNonVec() const {
    throw_vec_compare_exception();
  }
  bool dictVsNonDict() const {
    throw_dict_compare_exception();
  }
  bool keysetVsNonKeyset() const {
    throw_keyset_compare_exception();
  }
  bool collectionVsNonObj() const {
    throw_collection_compare_exception();
  }
};

struct Cmp {
  using RetType = int64_t;

  template<class T, class U>
  typename std::enable_if<
    !std::is_pointer<T>::value &&
    !std::is_pointer<U>::value,
    int64_t
  >::type operator()(T t, U u) const {
    // This ordering is required so that -1 is returned for NaNs (to match PHP7
    // behavior).
    return (t == u) ? 0 : ((t > u) ? 1 : -1);
  }

  int64_t operator()(const StringData* sd1, const StringData* sd2) const {
    return HPHP::compare(sd1, sd2);
  }

  int64_t operator()(const ArrayData* ad1, const ArrayData* ad2) const {
    assert(ad1->isPHPArray());
    assert(ad2->isPHPArray());
    return ArrayData::Compare(ad1, ad2);
  }

  int64_t operator()(const ObjectData* od1, const ObjectData* od2) const {
    return HPHP::compare(od1, od2);
  }

  int64_t operator()(const ResourceData* rd1, const ResourceData* rd2) const {
    return HPHP::compare(rd1->hdr(), rd2->hdr());
  }
  int64_t operator()(const ResourceHdr* rd1, const ResourceHdr* rd2) const {
    return HPHP::compare(rd1, rd2);
  }

  int64_t vec(const ArrayData* ad1, const ArrayData* ad2) const {
    assert(ad1->isVecArray());
    assert(ad2->isVecArray());
    return PackedArray::VecCmp(ad1, ad2);
  }
  int64_t dict(const ArrayData* ad1, const ArrayData* ad2) const {
    assert(ad1->isDict());
    assert(ad2->isDict());
    throw_dict_compare_exception();
  }
  int64_t keyset(const ArrayData* ad1, const ArrayData* ad2) const {
    assert(ad1->isKeyset());
    assert(ad2->isKeyset());
    throw_keyset_compare_exception();
  }

  bool vecVsNonVec() const {
    throw_vec_compare_exception();
  }
  bool dictVsNonDict() const {
    throw_dict_compare_exception();
  }
  bool keysetVsNonKeyset() const {
    throw_keyset_compare_exception();
  }
  int64_t collectionVsNonObj() const {
    throw_collection_compare_exception();
  }
};

//////////////////////////////////////////////////////////////////////

}

bool cellSame(Cell c1, Cell c2) {
  assert(cellIsPlausible(c1));
  assert(cellIsPlausible(c2));

  bool const null1 = isNullType(c1.m_type);
  bool const null2 = isNullType(c2.m_type);
  if (null1 && null2) return true;
  if (null1 || null2) return false;

  switch (c1.m_type) {
    case KindOfBoolean:
    case KindOfInt64:
      if (c2.m_type != c1.m_type) return false;
      return c1.m_data.num == c2.m_data.num;

    case KindOfDouble:
      if (c2.m_type != c1.m_type) return false;
      return c1.m_data.dbl == c2.m_data.dbl;

    case KindOfPersistentString:
    case KindOfString:
      if (!isStringType(c2.m_type)) return false;
      return c1.m_data.pstr->same(c2.m_data.pstr);

    case KindOfPersistentVec:
    case KindOfVec:
      if (!isVecType(c2.m_type)) return false;
      return PackedArray::VecSame(c1.m_data.parr, c2.m_data.parr);

    case KindOfPersistentDict:
    case KindOfDict:
      if (!isDictType(c2.m_type)) return false;
      return MixedArray::DictSame(c1.m_data.parr, c2.m_data.parr);

    case KindOfPersistentKeyset:
    case KindOfKeyset:
      if (!isKeysetType(c2.m_type)) return false;
      return SetArray::Same(c1.m_data.parr, c2.m_data.parr);

    case KindOfPersistentArray:
    case KindOfArray:
      if (!isArrayType(c2.m_type)) return false;
      return ArrayData::Same(c1.m_data.parr, c2.m_data.parr);

    case KindOfObject:
      return c2.m_type == KindOfObject &&
        c1.m_data.pobj == c2.m_data.pobj;

    case KindOfResource:
      return c2.m_type == KindOfResource &&
        c1.m_data.pres == c2.m_data.pres;

    case KindOfUninit:
    case KindOfNull:
    case KindOfRef:
    case KindOfClass:
      break;
  }
  not_reached();
}

bool tvSame(TypedValue tv1, TypedValue tv2) {
  assert(tvIsPlausible(tv1));
  assert(tvIsPlausible(tv2));
  return cellSame(*tvToCell(&tv1), *tvToCell(&tv2));
}

//////////////////////////////////////////////////////////////////////

bool cellEqual(Cell cell, bool val) {
  return cellRelOp(Eq(), cell, val);
}

bool cellEqual(Cell cell, int64_t val) {
  return cellRelOp(Eq(), cell, val);
}

bool cellEqual(Cell cell, double val) {
  return cellRelOp(Eq(), cell, val);
}

bool cellEqual(Cell cell, const StringData* val) {
  return cellRelOp(Eq(), cell, val);
}

bool cellEqual(Cell cell, const ArrayData* val) {
  if (val->isPHPArray()) return cellRelOp(Eq(), cell, val);
  if (val->isVecArray()) return cellRelOpVec(Eq(), cell, val);
  if (val->isDict()) return cellRelOpDict(Eq(), cell, val);
  if (val->isKeyset()) return cellRelOpKeyset(Eq(), cell, val);
  not_reached();
}

bool cellEqual(Cell cell, const ObjectData* val) {
  return cellRelOp(Eq(), cell, val);
}

bool cellEqual(Cell cell, const ResourceData* val) {
  return cellRelOp(Eq(), cell, val);
}
bool cellEqual(Cell cell, const ResourceHdr* val) {
  return cellRelOp(Eq(), cell, val);
}

bool cellEqual(Cell c1, Cell c2) {
  return cellRelOp(Eq(), c1, c2);
}

bool tvEqual(TypedValue tv1, TypedValue tv2) {
  return tvRelOp(Eq(), tv1, tv2);
}

bool cellLess(Cell cell, bool val) {
  return cellRelOp(Lt(), cell, val);
}

bool cellLess(Cell cell, int64_t val) {
  return cellRelOp(Lt(), cell, val);
}

bool cellLess(Cell cell, double val) {
  return cellRelOp(Lt(), cell, val);
}

bool cellLess(Cell cell, const StringData* val) {
  return cellRelOp(Lt(), cell, val);
}

bool cellLess(Cell cell, const ArrayData* val) {
  if (val->isPHPArray()) return cellRelOp(Lt(), cell, val);
  if (val->isVecArray()) return cellRelOpVec(Lt(), cell, val);
  if (val->isDict()) return cellRelOpDict(Lt(), cell, val);
  if (val->isKeyset()) return cellRelOpKeyset(Lt(), cell, val);
  not_reached();
}

bool cellLess(Cell cell, const ObjectData* val) {
  return cellRelOp(Lt(), cell, val);
}

bool cellLess(Cell cell, const ResourceData* val) {
  return cellRelOp(Lt(), cell, val);
}
bool cellLess(Cell cell, const ResourceHdr* val) {
  return cellRelOp(Lt(), cell, val);
}

bool cellLess(Cell c1, Cell c2) {
  return cellRelOp(Lt(), c1, c2);
}

bool tvLess(TypedValue tv1, TypedValue tv2) {
  return tvRelOp(Lt(), tv1, tv2);
}

bool cellGreater(Cell cell, bool val) {
  return cellRelOp(Gt(), cell, val);
}

bool cellGreater(Cell cell, int64_t val) {
  return cellRelOp(Gt(), cell, val);
}

bool cellGreater(Cell cell, double val) {
  return cellRelOp(Gt(), cell, val);
}

bool cellGreater(Cell cell, const StringData* val) {
  return cellRelOp(Gt(), cell, val);
}

bool cellGreater(Cell cell, const ArrayData* val) {
  if (val->isPHPArray()) return cellRelOp(Gt(), cell, val);
  if (val->isVecArray()) return cellRelOpVec(Gt(), cell, val);
  if (val->isDict()) return cellRelOpDict(Gt(), cell, val);
  if (val->isKeyset()) return cellRelOpKeyset(Gt(), cell, val);
  not_reached();
}

bool cellGreater(Cell cell, const ObjectData* val) {
  return cellRelOp(Gt(), cell, val);
}

bool cellGreater(Cell cell, const ResourceData* val) {
  return cellRelOp(Gt(), cell, val);
}
bool cellGreater(Cell cell, const ResourceHdr* val) {
  return cellRelOp(Gt(), cell, val);
}

bool cellGreater(Cell c1, Cell c2) {
  return cellRelOp(Gt(), c1, c2);
}

bool tvGreater(TypedValue tv1, TypedValue tv2) {
  return tvRelOp(Gt(), tv1, tv2);
}

//////////////////////////////////////////////////////////////////////

int64_t cellCompare(Cell cell, bool val) {
  return cellRelOp(Cmp(), cell, val);
}

int64_t cellCompare(Cell cell, int64_t val) {
  return cellRelOp(Cmp(), cell, val);
}

int64_t cellCompare(Cell cell, double val) {
  return cellRelOp(Cmp(), cell, val);
}

int64_t cellCompare(Cell cell, const StringData* val) {
  return cellRelOp(Cmp(), cell, val);
}

int64_t cellCompare(Cell cell, const ArrayData* val) {
  if (val->isPHPArray()) return cellRelOp(Cmp(), cell, val);
  if (val->isVecArray()) return cellRelOpVec(Cmp(), cell, val);
  if (val->isDict()) return cellRelOpDict(Cmp(), cell, val);
  if (val->isKeyset()) return cellRelOpKeyset(Cmp(), cell, val);
  not_reached();
}

int64_t cellCompare(Cell cell, const ObjectData* val) {
  return cellRelOp(Cmp(), cell, val);
}

int64_t cellCompare(Cell cell, const ResourceData* val) {
  return cellRelOp(Cmp(), cell, val);
}
int64_t cellCompare(Cell cell, const ResourceHdr* val) {
  return cellRelOp(Cmp(), cell, val);
}

int64_t cellCompare(Cell c1, Cell c2) {
  return cellRelOp(Cmp(), c1, c2);
}

int64_t tvCompare(TypedValue tv1, TypedValue tv2) {
  return tvRelOp(Cmp(), tv1, tv2);
}

//////////////////////////////////////////////////////////////////////

bool cellLessOrEqual(Cell c1, Cell c2) {
  assert(cellIsPlausible(c1));
  assert(cellIsPlausible(c2));

  if ((isArrayLikeType(c1.m_type) && isArrayLikeType(c2.m_type)) ||
      (c1.m_type == KindOfObject && c2.m_type == KindOfObject) ||
      (c1.m_type == KindOfResource && c2.m_type == KindOfResource)) {
    return cellLess(c1, c2) || cellEqual(c1, c2);
  }

  // We have to treat NaN specially: NAN <= NAN is false, for example, so we
  // can't just say !(NAN > NAN).
  if ((c1.m_type == KindOfDouble && std::isnan(c1.m_data.dbl)) ||
      (c2.m_type == KindOfDouble && std::isnan(c2.m_data.dbl))) {
    return cellLess(c1, c2) || cellEqual(c1, c2);
  }
  return !cellGreater(c1, c2);
}

bool cellGreaterOrEqual(Cell c1, Cell c2) {
  assert(cellIsPlausible(c1));
  assert(cellIsPlausible(c2));

  if ((isArrayLikeType(c1.m_type) && isArrayLikeType(c2.m_type)) ||
      (c1.m_type == KindOfObject && c2.m_type == KindOfObject) ||
      (c1.m_type == KindOfResource && c2.m_type == KindOfResource)) {
    return cellGreater(c1, c2) || cellEqual(c1, c2);
  }
  if ((c1.m_type == KindOfDouble && std::isnan(c1.m_data.dbl)) ||
      (c2.m_type == KindOfDouble && std::isnan(c2.m_data.dbl))) {
    return cellGreater(c1, c2) || cellEqual(c1, c2);
  }
  return !cellLess(c1, c2);
}

//////////////////////////////////////////////////////////////////////

}
