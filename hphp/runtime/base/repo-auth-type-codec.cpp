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
#include "hphp/runtime/base/repo-auth-type-codec.h"

#include "hphp/runtime/vm/unit.h"
#include "hphp/runtime/vm/repo.h"
#include "hphp/runtime/vm/repo-global-data.h"

namespace HPHP {

//////////////////////////////////////////////////////////////////////

namespace {

template <class LookupStr, class LookupArrayType>
RepoAuthType decodeRATImpl(const unsigned char*& pc, LookupStr lookupStr,
                           LookupArrayType lookupArrayType) {
  using T = RepoAuthType::Tag;
  uint16_t permutatedTag = static_cast<uint8_t>(*pc++);
  if (permutatedTag == 0xff) {
    uint8_t tmp = static_cast<uint8_t>(*pc++);
    assertx(tmp != 0xff);
    permutatedTag = tmp + 0xff;
  }

  // Move the kRATPtrBit(0x4000) and kRATArrayDataBit(0x8000) bit back
  auto rawTag = (permutatedTag >> 2) | (permutatedTag << 14);
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
  case T::Cls:
  case T::OptCls:
  case T::ClsMeth:
  case T::OptClsMeth:
  case T::Record:
  case T::OptRecord:
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
    return RepoAuthType{tag};

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
      uint32_t id = decode_iva(pc);
      auto const arr = lookupArrayType(id);
      return RepoAuthType{tag, arr};
    }
    return RepoAuthType{tag};

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
  case T::SubRecord:
  case T::OptExactRecord:
  case T::OptSubRecord:
    assertx(!highBitSet);
    {
      uint32_t id = decode_iva(pc);
      const StringData* const name = lookupStr(id);
      return RepoAuthType{tag, name};
    }
  }
  not_reached();
}

}

RepoAuthType decodeRAT(const Unit* unit, const unsigned char*& pc) {
  return decodeRATImpl(
    pc,
    [&](uint32_t id) { return unit->lookupLitstrId(id); },
    [&](uint32_t id) { return unit->lookupArrayTypeId(id); }
  );
}

RepoAuthType decodeRAT(const UnitEmitter& ue, const unsigned char*& pc) {
  return decodeRATImpl(
    pc,
    [&](uint32_t id) { return ue.lookupLitstr(id); },
    [&](uint32_t id) { return ue.lookupArrayType(id); }
  );
}

void encodeRAT(FuncEmitter& fe, RepoAuthType rat) {
  auto rawTag = static_cast<uint16_t>(rat.tag());
  if (rat.hasArrData()) rawTag |= kRATArrayDataBit;

  // Move the kRATPtrBit(0x4000) and kRATArrayDataBit(0x8000) last
  uint16_t permutatedTag = (rawTag << 2) | (rawTag >> 14);
  if (permutatedTag >= 0xff) {
    // Write a 0xff signal byte
    fe.emitByte(static_cast<uint8_t>(0xff));
    permutatedTag -= 0xff;
  }
  assertx(permutatedTag < 0xff);
  fe.emitByte(static_cast<uint8_t>(permutatedTag));

  using T = RepoAuthType::Tag;
  switch (rat.tag()) {
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
  case T::Cls:
  case T::OptCls:
  case T::ClsMeth:
  case T::OptClsMeth:
  case T::Record:
  case T::OptRecord:
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
  case T::OptNum:
  case T::Num:
  case T::InitPrim:
  case T::InitUnc:
  case T::Unc:
  case T::NonNull:
  case T::InitCell:
  case T::Cell:
    break;

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
    if (rat.hasArrData()) {
      fe.emitIVA(rat.arrayId());
    }
    break;

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
    fe.emitIVA(fe.ue().mergeLitstr(rat.clsName()));
    break;

  case T::ExactRecord:
  case T::SubRecord:
  case T::OptExactRecord:
  case T::OptSubRecord:
    fe.emitIVA(fe.ue().mergeLitstr(rat.recordName()));
    break;

  }
}

//////////////////////////////////////////////////////////////////////

}
