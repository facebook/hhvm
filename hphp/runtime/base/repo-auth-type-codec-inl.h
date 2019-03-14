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
#ifndef incl_HPHP_REPO_AUTH_TYPE_CODEC_INL_H_
#define incl_HPHP_REPO_AUTH_TYPE_CODEC_INL_H_

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
  case T::Dbl:
  case T::OptDbl:
  case T::Res:
  case T::OptRes:
  case T::Bool:
  case T::OptBool:
  case T::SStr:
  case T::OptSStr:
  case T::Str:
  case T::OptStr:
  case T::Obj:
  case T::OptObj:
  case T::Func:
  case T::OptFunc:
  case T::ClsMeth:
  case T::OptClsMeth:
  case T::Cls:
  case T::OptCls:
  case T::UncArrKey:
  case T::ArrKey:
  case T::OptUncArrKey:
  case T::OptArrKey:
  case T::UncStrLike:
  case T::StrLike:
  case T::OptUncStrLike:
  case T::OptStrLike:
  case T::InitUnc:
  case T::Unc:
  case T::InitCell:
  case T::Cell:
  case T::Ref:
  case T::InitGen:
  case T::Gen:
    assertx(!highBitSet);
    return tagSize;
  case T::SArr:
  case T::OptSArr:
  case T::Arr:
  case T::OptArr:
  case T::SVArr:
  case T::OptSVArr:
  case T::VArr:
  case T::OptVArr:
  case T::SDArr:
  case T::OptSDArr:
  case T::DArr:
  case T::OptDArr:
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
    if (highBitSet) {
      return ((int8_t(nextPcVal) < 0) ? 4 : 1) + tagSize;
    }
    return tagSize;
  case T::ExactObj:
  case T::SubObj:
  case T::OptExactObj:
  case T::OptSubObj:
    assertx(!highBitSet);
    return ((int8_t(nextPcVal) < 0) ? 4 : 1) + tagSize;
  }
  not_reached();
}

//////////////////////////////////////////////////////////////////////

}

#endif
