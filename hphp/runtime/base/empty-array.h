/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2013 Facebook, Inc. (http://www.facebook.com)     |
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
#ifndef incl_HPHP_EMPTY_ARRAY_H_
#define incl_HPHP_EMPTY_ARRAY_H_

#include <cstddef>
#include <cstdint>
#include <utility>
#include <sys/types.h>

namespace HPHP {

//////////////////////////////////////////////////////////////////////

struct Variant;
struct ArrayData;
struct RefData;
struct StringData;
struct TypedValue;
struct MArrayIter;
struct APCHandle;

//////////////////////////////////////////////////////////////////////

/*
 * Functions relating to the "empty array" kind.  These implement
 * entries in the array dispatch table for the global empty array.
 * Other arrays may also be empty in the sense that size() == 0, but
 * this one is dealt with commonly enough to deserve special handlers.
 */
struct EmptyArray {
  static void Release(ArrayData*);

  // TODO(#3983912): these helpers should be moved somewhere so we can
  // use them for other array shapes, too.
  static void* ReturnNull(...);
  static bool ReturnFalse(...);
  static bool ReturnTrue(...);
  static ArrayData* ReturnFirstArg(ArrayData*, ...);
  static ssize_t ReturnInvalidIndex(const ArrayData*);
  static void NoOp(...);
  static ArrayData* PopOrDequeue(ArrayData*, Variant&);

  static void NvGetKey(const ArrayData*, TypedValue* out, ssize_t pos);
  static ArrayData* SetInt(ArrayData*, int64_t k, const Variant& v, bool copy);
  static ArrayData* SetStr(ArrayData*, StringData* k, const Variant& v,
    bool copy);
  static const Variant& GetValueRef(const ArrayData* ad, ssize_t pos);
  static ArrayData* LvalInt(ArrayData*, int64_t k, Variant*& ret, bool copy);
  static ArrayData* LvalStr(ArrayData*, StringData* k, Variant*& ret,
                            bool copy);
  static ArrayData* LvalNew(ArrayData*, Variant*& ret, bool copy);
  static ArrayData* SetRefInt(ArrayData*, int64_t k, const Variant& v,
    bool copy);
  static ArrayData* SetRefStr(ArrayData*, StringData* k,
    const Variant& v, bool copy);
  static ssize_t IterAdvance(const ArrayData*, ssize_t prev);
  static ssize_t IterRewind(const ArrayData*, ssize_t prev);
  static bool ValidMArrayIter(const ArrayData*, const MArrayIter& fp);
  static bool AdvanceMArrayIter(ArrayData*, MArrayIter& fp);
  static ArrayData* Copy(const ArrayData* ad);
  static ArrayData* CopyWithStrongIterators(const ArrayData*);
  static ArrayData* NonSmartCopy(const ArrayData*);
  static ArrayData* ZSetInt(ArrayData* ad, int64_t k, RefData* v);
  static ArrayData* ZSetStr(ArrayData* ad, StringData* k, RefData* v);
  static ArrayData* ZAppend(ArrayData* ad, RefData* v);
  static ArrayData* Append(ArrayData*, const Variant& v, bool copy);
  static ArrayData* AppendRef(ArrayData*, const Variant& v, bool copy);
  static ArrayData* AppendWithRef(ArrayData*, const Variant& v, bool copy);
  static ArrayData* PlusEq(ArrayData*, const ArrayData* elems);
  static ArrayData* Merge(ArrayData*, const ArrayData* elems);
  static ArrayData* Prepend(ArrayData*, const Variant& v, bool copy);
  static void OnSetEvalScalar(ArrayData*);

private:
  static std::pair<ArrayData*,TypedValue*> MakePacked(TypedValue);
  static std::pair<ArrayData*,TypedValue*> MakePackedInl(TypedValue);
  static std::pair<ArrayData*,TypedValue*> MakeMixed(StringData*, TypedValue);
  static std::pair<ArrayData*,TypedValue*> MakeMixed(int64_t, TypedValue);

private:
  struct Initializer;
  static Initializer s_initializer;
};

//////////////////////////////////////////////////////////////////////

}

#endif
