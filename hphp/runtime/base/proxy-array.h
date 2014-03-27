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
#ifndef incl_HPHP_PROXY_ARRAY_H
#define incl_HPHP_PROXY_ARRAY_H

#include "hphp/runtime/vm/name-value-table.h"
#include "hphp/runtime/base/array-data.h"

namespace HPHP {

//////////////////////////////////////////////////////////////////////

/*
 * A simple proxy for an underlying ArrayData. This is needed, since some
 * operations will reseat the unerlying array, and some callers are unable to
 * deal with that because of non-indiraction APIs.
 *
 * The Zend compatibility layer needs this since functions like
 * zend_hash_update only take a pointer to the ArrayData and don't expect it to
 * change location.
 */
struct ProxyArray : public ArrayData {
  explicit ProxyArray(ArrayData* ad)
    : ArrayData(kProxyKind)
    , m_ad(ad)
  {}

  static ProxyArray* Make(ArrayData*);

public: // ArrayData implementation
  static void Release(ArrayData*);

  static size_t Vsize(const ArrayData*);
  static void NvGetKey(const ArrayData* ad, TypedValue* out, ssize_t pos);
  static const Variant& GetValueRef(const ArrayData*, ssize_t pos);

  static bool ExistsInt(const ArrayData* ad, int64_t k);
  static bool ExistsStr(const ArrayData* ad, const StringData* k);

  static TypedValue* NvGetInt(const ArrayData*, int64_t k);
  static TypedValue* NvGetStr(const ArrayData*, const StringData* k);

  static ArrayData* LvalInt(ArrayData*, int64_t k, Variant*& ret, bool copy);
  static ArrayData* LvalStr(ArrayData*, StringData* k, Variant*& ret,
                            bool copy);
  static ArrayData* LvalNew(ArrayData*, Variant*& ret, bool copy);

  static ArrayData* SetInt(ArrayData*, int64_t k, const Variant& v, bool copy);
  static ArrayData* SetStr(ArrayData*, StringData* k, const Variant& v, bool copy);
  static ArrayData* SetRefInt(ArrayData*, int64_t k, const Variant& v, bool copy);
  static ArrayData* SetRefStr(ArrayData*, StringData* k, const Variant& v, bool copy);
  static ArrayData* RemoveInt(ArrayData*, int64_t k, bool copy);
  static ArrayData* RemoveStr(ArrayData*, const StringData* k, bool copy);

  static ArrayData* Copy(const ArrayData* ad);

  static ArrayData* Append(ArrayData*, const Variant& v, bool copy);
  static ArrayData* AppendRef(ArrayData*, const Variant& v, bool copy);
  static ArrayData* AppendWithRef(ArrayData*, const Variant& v, bool copy);

  static ArrayData* PlusEq(ArrayData*, const ArrayData* elems);
  static ArrayData* Merge(ArrayData*, const ArrayData* elems);
  static ArrayData* Pop(ArrayData*, Variant &value);
  static ArrayData* Dequeue(ArrayData*, Variant &value);
  static ArrayData* Prepend(ArrayData*, const Variant& v, bool copy);
  static void Renumber(ArrayData*);
  static void OnSetEvalScalar(ArrayData*);
  static ArrayData* Escalate(const ArrayData* ad);

  static ssize_t IterBegin(const ArrayData*);
  static ssize_t IterEnd(const ArrayData*);
  static ssize_t IterAdvance(const ArrayData*, ssize_t prev);
  static ssize_t IterRewind(const ArrayData*, ssize_t prev);

  static bool ValidMArrayIter(const ArrayData*, const MArrayIter & fp);
  static bool AdvanceMArrayIter(ArrayData*, MArrayIter&);
  static bool IsVectorData(const ArrayData*);
  static APCHandle *GetAPCHandle(const ArrayData* ad);

  static ArrayData* EscalateForSort(ArrayData*);
  static void Ksort(ArrayData*, int sort_flags, bool ascending);
  static void Sort(ArrayData*, int sort_flags, bool ascending);
  static void Asort(ArrayData*, int sort_flags, bool ascending);
  static bool Uksort(ArrayData*, const Variant& cmp_function);
  static bool Usort(ArrayData*, const Variant& cmp_function);
  static bool Uasort(ArrayData*, const Variant& cmp_function);

  static ArrayData* ZSetInt(ArrayData* ad, int64_t k, RefData* v);
  static ArrayData* ZSetStr(ArrayData* ad, StringData* k, RefData* v);
  static ArrayData* ZAppend(ArrayData* ad, RefData* v);

  static ArrayData* CopyWithStrongIterators(const ArrayData*);
  static ArrayData* NonSmartCopy(const ArrayData*);

private:
  static ProxyArray* asProxyArray(ArrayData* ad);
  static const ProxyArray* asProxyArray(const ArrayData* ad);
  static ProxyArray* reseatable(ArrayData* oldArr, ArrayData* newArr);
  static ArrayData* innerArr(ArrayData* ad);
  static ArrayData* innerArr(const ArrayData* ad);

private:
  ArrayData* m_ad;
};

//////////////////////////////////////////////////////////////////////

}

#endif
