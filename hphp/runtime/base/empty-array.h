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
#ifndef incl_HPHP_EMPTY_ARRAY_H_
#define incl_HPHP_EMPTY_ARRAY_H_

#include <cstddef>
#include <cstdint>
#include <utility>
#include <sys/types.h>

#include "hphp/runtime/base/array-common.h"

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
  static constexpr auto NvGetInt =
    reinterpret_cast<TypedValue* (*)(const ArrayData*, int64_t)>(
      ArrayCommon::ReturnNull
    );
  static constexpr auto NvGetStr =
    reinterpret_cast<TypedValue* (*)(const ArrayData*, const StringData*)>(
      ArrayCommon::ReturnNull
    );
  static void NvGetKey(const ArrayData*, TypedValue* out, ssize_t pos);
  static ArrayData* SetInt(ArrayData*, int64_t k, const Variant& v, bool copy);
  static ArrayData* SetStr(ArrayData*, StringData* k, const Variant& v,
    bool copy);
  static constexpr auto RemoveInt =
    reinterpret_cast<ArrayData* (*)(ArrayData*, int64_t, bool)>(
      ArrayCommon::ReturnFirstArg
    );
  static constexpr auto RemoveStr =
    reinterpret_cast<ArrayData* (*)(ArrayData*, const StringData*, bool)>(
      ArrayCommon::ReturnFirstArg
    );
  static size_t Vsize(const ArrayData*);
  static const Variant& GetValueRef(const ArrayData* ad, ssize_t pos);
  static constexpr auto IsVectorData =
    reinterpret_cast<bool (*)(const ArrayData*)>(
      ArrayCommon::ReturnTrue
    );
  static constexpr auto ExistsInt =
    reinterpret_cast<bool (*)(const ArrayData*, int64_t)>(
      ArrayCommon::ReturnFalse
    );
  static constexpr auto ExistsStr =
    reinterpret_cast<bool (*)(const ArrayData*, const StringData*)>(
      ArrayCommon::ReturnFalse
    );
  static ArrayData* LvalInt(ArrayData*, int64_t k, Variant*& ret, bool copy);
  static ArrayData* LvalStr(ArrayData*, StringData* k, Variant*& ret,
                            bool copy);
  static ArrayData* LvalNew(ArrayData*, Variant*& ret, bool copy);
  static ArrayData* SetRefInt(ArrayData*, int64_t k, Variant& v, bool copy);
  static ArrayData* SetRefStr(ArrayData*, StringData* k, Variant& v,
    bool copy);
  static constexpr auto AddInt = &SetInt;
  static constexpr auto AddStr = &SetStr;
  static constexpr auto IterBegin = &ArrayCommon::ReturnInvalidIndex;
  static constexpr auto IterEnd = &ArrayCommon::ReturnInvalidIndex;
  static ssize_t IterAdvance(const ArrayData*, ssize_t prev);
  static ssize_t IterRewind(const ArrayData*, ssize_t prev);
  static constexpr auto ValidMArrayIter =
    reinterpret_cast<bool (*)(const ArrayData*, const MArrayIter&)>(
      ArrayCommon::ReturnFalse
    );
  static bool AdvanceMArrayIter(ArrayData*, MArrayIter& fp);
  static constexpr auto EscalateForSort =
    reinterpret_cast<ArrayData* (*)(ArrayData*)>(
      ArrayCommon::ReturnFirstArg
    );
  static constexpr auto Ksort =
    reinterpret_cast<void (*)(ArrayData*, int, bool)>(
      ArrayCommon::NoOp
    );
  static constexpr auto Sort =
    reinterpret_cast<void (*)(ArrayData*, int, bool)>(
      ArrayCommon::NoOp
    );
  static constexpr auto Asort =
    reinterpret_cast<void (*)(ArrayData*, int, bool)>(
      ArrayCommon::NoOp
    );
  static constexpr auto Uksort =
    reinterpret_cast<bool (*)(ArrayData*, const Variant&)>(
      ArrayCommon::ReturnTrue
    );
  static constexpr auto Usort =
    reinterpret_cast<bool (*)(ArrayData*, const Variant&)>(
      ArrayCommon::ReturnTrue
    );
  static constexpr auto Uasort =
    reinterpret_cast<bool (*)(ArrayData*, const Variant&)>(
      ArrayCommon::ReturnTrue
    );
  static ArrayData* PopOrDequeue(ArrayData*, Variant&);
  static constexpr auto Pop = &PopOrDequeue;
  static constexpr auto Dequeue = &PopOrDequeue;
  static ArrayData* Copy(const ArrayData* ad);
  static ArrayData* CopyWithStrongIterators(const ArrayData*);
  static ArrayData* NonSmartCopy(const ArrayData*);
  static ArrayData* ZSetInt(ArrayData* ad, int64_t k, RefData* v);
  static ArrayData* ZSetStr(ArrayData* ad, StringData* k, RefData* v);
  static ArrayData* ZAppend(ArrayData* ad, RefData* v);
  static ArrayData* Append(ArrayData*, const Variant& v, bool copy);
  static ArrayData* AppendRef(ArrayData*, Variant& v, bool copy);
  static ArrayData* AppendWithRef(ArrayData*, const Variant& v, bool copy);
  static ArrayData* PlusEq(ArrayData*, const ArrayData* elems);
  static ArrayData* Merge(ArrayData*, const ArrayData* elems);
  static ArrayData* Prepend(ArrayData*, const Variant& v, bool copy);
  static constexpr auto Renumber =
    reinterpret_cast<void (*)(ArrayData*)>(
      ArrayCommon::NoOp
    );
  static void OnSetEvalScalar(ArrayData*);
  static constexpr auto Escalate =
    reinterpret_cast<ArrayData* (*)(const ArrayData*)>(
      ArrayCommon::ReturnFirstArg
    );
  static constexpr auto GetAPCHandle =
    reinterpret_cast<APCHandle* (*)(const ArrayData*)>(
      ArrayCommon::ReturnNull
    );

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
