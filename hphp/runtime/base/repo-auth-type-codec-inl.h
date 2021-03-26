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
#pragma once

namespace HPHP {

//////////////////////////////////////////////////////////////////////

/*
 * We set the high bit in the tag if there's going to be a specialized
 * array id.  This should only be found when the tag is an array type.
 */
constexpr uint16_t kRATArrayDataBit = 0x8000;

//////////////////////////////////////////////////////////////////////

ALWAYS_INLINE
size_t encodedRATSize(const unsigned char* pc) {
  using T = RepoAuthType::Tag;
  uint16_t permutatedTag = static_cast<uint8_t>(*pc);
  auto nextPcVal = *(pc + 1);
  if (permutatedTag == 0xff) {
    uint8_t tmp = static_cast<uint8_t>(nextPcVal);
    assertx(tmp != 0xff);
    permutatedTag = tmp + 0xff;
    nextPcVal = *(pc + 2);
  }
  size_t tagSize = permutatedTag < 0xff ? 1 : 2;

  // Move the kRATPtrBit(0x4000) and kRATArrayDataBit(0x8000) bit back
  uint16_t rawTag = (permutatedTag >> 2) | (permutatedTag << 14);
  bool const highBitSet = rawTag & kRATArrayDataBit;
  auto const tag = static_cast<T>(rawTag & ~kRATArrayDataBit);
  switch (tag) {
  case T::Uninit:
  case T::InitNull:
  case T::Null:
  case T::Int:
  case T::OptInt:
  case T::UninitInt:
  case T::Dbl:
  case T::OptDbl:
  case T::Res:
  case T::OptRes:
  case T::Bool:
  case T::OptBool:
  case T::UninitBool:
  case T::SStr:
  case T::OptSStr:
  case T::UninitSStr:
  case T::Str:
  case T::OptStr:
  case T::UninitStr:
  case T::Obj:
  case T::OptObj:
  case T::UninitObj:
  case T::Func:
  case T::OptFunc:
  case T::ClsMeth:
  case T::OptClsMeth:
  case T::Record:
  case T::OptRecord:
  case T::Cls:
  case T::OptCls:
  case T::LazyCls:
  case T::OptLazyCls:
  case T::UncArrKey:
  case T::ArrKey:
  case T::OptUncArrKey:
  case T::OptArrKey:
  case T::UncStrLike:
  case T::StrLike:
  case T::OptUncStrLike:
  case T::OptStrLike:
  case T::UncArrKeyCompat:
  case T::ArrKeyCompat:
  case T::OptUncArrKeyCompat:
  case T::OptArrKeyCompat:
  case T::Num:
  case T::OptNum:
  case T::InitPrim:
  case T::InitUnc:
  case T::Unc:
  case T::NonNull:
  case T::InitCell:
  case T::Cell:
    assertx(!highBitSet);
    return tagSize;
  case T::SVec:
  case T::OptSVec:
  case T::Vec:
  case T::OptVec:
  case T::SDict:
  case T::OptSDict:
  case T::Dict:
  case T::OptDict:
  case T::SKeyset:
  case T::OptSKeyset:
  case T::Keyset:
  case T::OptKeyset:
  case T::SArrLike:
  case T::ArrLike:
  case T::OptSArrLike:
  case T::OptArrLike:
  case T::VecCompat:
  case T::OptVecCompat:
  case T::ArrLikeCompat:
  case T::OptArrLikeCompat:
    if (highBitSet) {
      return ((int8_t(nextPcVal) < 0) ? 4 : 1) + tagSize;
    }
    return tagSize;
  case T::ExactObj:
  case T::SubObj:
  case T::OptExactObj:
  case T::OptSubObj:
  case T::UninitExactObj:
  case T::UninitSubObj:
  case T::ExactCls:
  case T::SubCls:
  case T::OptExactCls:
  case T::OptSubCls:
  case T::ExactRecord:
  case T::OptExactRecord:
  case T::SubRecord:
  case T::OptSubRecord:
    assertx(!highBitSet);
    return ((int8_t(nextPcVal) < 0) ? 4 : 1) + tagSize;
  }
  not_reached();
}

//////////////////////////////////////////////////////////////////////

}
