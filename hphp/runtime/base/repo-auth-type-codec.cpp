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

template<class LookupStr>
RepoAuthType decodeRATImpl(const unsigned char*& pc, LookupStr lookupStr) {
  using T = RepoAuthType::Tag;
  auto const rawTag = *pc++;
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
  case T::InitUnc:
  case T::Unc:
  case T::InitCell:
  case T::Cell:
  case T::Ref:
  case T::InitGen:
  case T::Gen:
    assert(!highBitSet);
    return RepoAuthType{tag};

  case T::SArr:
  case T::OptSArr:
  case T::Arr:
  case T::OptArr:
    if (highBitSet) {
      uint32_t id;
      std::memcpy(&id, pc, sizeof id);
      pc += sizeof id;
      auto const arr = Repo::get().global().arrayTypeTable.lookup(id);
      return RepoAuthType{tag, arr};
    }
    return RepoAuthType{tag};

  case T::ExactObj:
  case T::SubObj:
  case T::OptExactObj:
  case T::OptSubObj:
    assert(!highBitSet);
    {
      uint32_t id;
      std::memcpy(&id, pc, sizeof id);
      pc += sizeof id;
      const StringData* const clsName = lookupStr(id);
      return RepoAuthType{tag, clsName};
    }
  }
  not_reached();
}

}

RepoAuthType decodeRAT(const Unit* unit, const unsigned char*& pc) {
  return decodeRATImpl(pc, [&] (uint32_t id) {
    return unit->lookupLitstrId(id);
  });
}

RepoAuthType decodeRAT(const UnitEmitter& ue, const unsigned char*& pc) {
  return decodeRATImpl(pc, [&] (uint32_t id) {
    return ue.lookupLitstr(id);
  });
}

void encodeRAT(UnitEmitter& ue, RepoAuthType rat) {
  using T = RepoAuthType::Tag;
  switch (rat.tag()) {
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
  case T::InitUnc:
  case T::Unc:
  case T::InitCell:
  case T::Cell:
  case T::Ref:
  case T::InitGen:
  case T::Gen:
    ue.emitByte(static_cast<uint8_t>(rat.tag()));
    break;

  case T::SArr:
  case T::OptSArr:
  case T::Arr:
  case T::OptArr:
    {
      auto tagByte = static_cast<uint8_t>(rat.tag());
      auto const arr = rat.array();
      if (arr) tagByte |= kRATArrayDataBit;
      ue.emitByte(tagByte);
      if (arr) {
        ue.emitInt32(arr->id());
      }
      break;
    }

  case T::ExactObj:
  case T::SubObj:
  case T::OptExactObj:
  case T::OptSubObj:
    ue.emitByte(static_cast<uint8_t>(rat.tag()));
    ue.emitInt32(ue.mergeLitstr(rat.clsName()));
    break;
  }
}

//////////////////////////////////////////////////////////////////////

}
