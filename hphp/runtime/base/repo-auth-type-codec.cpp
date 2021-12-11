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

#include "hphp/runtime/vm/func-emitter.h"
#include "hphp/runtime/vm/unit.h"
#include "hphp/runtime/vm/unit-emitter.h"
#include "hphp/runtime/vm/repo-global-data.h"

namespace HPHP {

//////////////////////////////////////////////////////////////////////

namespace {

template <typename LookupStr>
RepoAuthType decodeRATImpl(const unsigned char*& pc, LookupStr lookupStr) {
  using T = RepoAuthType::Tag;
  static_assert(sizeof(T) == sizeof(uint8_t));
  auto const tag = static_cast<T>(*pc++);

  if (RepoAuthType::tagHasArrData(tag)) {
    auto const id = static_cast<ArrayTypeTable::Id>(decode_iva(pc));
    return RepoAuthType{tag, globalArrayTypeTable().lookup(id)};
  }

  if (RepoAuthType::tagHasName(tag)) {
    auto const id = decode_iva(pc);
    return RepoAuthType{tag, lookupStr(id)};
  }

  return RepoAuthType{tag};
}

//////////////////////////////////////////////////////////////////////

}

RepoAuthType decodeRAT(const Unit* unit, const unsigned char*& pc) {
  return decodeRATImpl(
    pc,
    [&] (uint32_t id) { return unit->lookupLitstrId(id); }
  );
}

RepoAuthType decodeRAT(const UnitEmitter& ue, const unsigned char*& pc) {
  return decodeRATImpl(
    pc,
    [&] (uint32_t id) { return ue.lookupLitstr(id); }
  );
}

void encodeRAT(FuncEmitter& fe, RepoAuthType rat) {
  using T = RepoAuthType::Tag;
  static_assert(sizeof(T) == sizeof(uint8_t));

  fe.emitByte((uint8_t)rat.tag());
  if (auto const a = rat.array()) {
    fe.emitIVA(a->id());
  } else if (auto const n = rat.name()) {
    fe.emitIVA(fe.ue().mergeLitstr(n));
  }
}

size_t encodedRATSize(const unsigned char* pc) {
  using T = RepoAuthType::Tag;
  static_assert(sizeof(T) == sizeof(uint8_t));
  auto const tag = static_cast<T>(*pc++);

  if (!RepoAuthType::tagHasArrData(tag) && !RepoAuthType::tagHasName(tag)) {
    return 1;
  }

  auto const start = pc;
  decode_iva(pc);
  return
    reinterpret_cast<uintptr_t>(pc) - reinterpret_cast<uintptr_t>(start) + 1;
}

//////////////////////////////////////////////////////////////////////

}
