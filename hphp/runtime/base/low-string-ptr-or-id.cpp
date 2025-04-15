/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-present Facebook, Inc. (http://www.facebook.com)  |
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

#include "hphp/runtime/base/low-string-ptr-or-id.h"
#include "hphp/runtime/vm/unit.h"
#include "hphp/runtime/vm/unit-emitter.h"
#include "hphp/util/assertions.h"
#include "hphp/util/low-ptr.h"

namespace HPHP {

namespace details {
  bool is_low(const StringData* str) {
    LowStringPtrOrId::storage_type ones = ~0;
    auto ptr = reinterpret_cast<uintptr_t>(str);
    return (ptr & ones) == ptr;
  }

  bool is_ptr(LowStringPtrOrId::storage_type s) {
    return (s & 0x1) == 0;
  }

  inline LowStringPtrOrId::storage_type to_low(const StringData* str) {
    assertx(is_low(str));
    auto s = (LowStringPtrOrId::storage_type)(reinterpret_cast<uintptr_t>(str));
    assertx(is_ptr(s));
    return s;
  }
}

LowStringPtrOrId::LowStringPtrOrId(const StringData* str) {
  m_s.store(details::to_low(str), std::memory_order_relaxed);
}

LowStringPtrOrId::LowStringPtrOrId(Id id) {
  assertx(id >= 0);
  m_s.store(((storage_type)id << 1) | 0x1, std::memory_order_relaxed);
}

LowStringPtrOrId::LowStringPtrOrId(const LowStringPtrOrId& other) {
  m_s.store(other.m_s.load(std::memory_order_relaxed));
}

LowStringPtrOrId& LowStringPtrOrId::operator=(const StringData* str) {
  m_s.store(details::to_low(str), std::memory_order_relaxed);
  return *this;
}

LowStringPtrOrId& LowStringPtrOrId::operator=(const LowStringPtrOrId& other) {
  m_s.store(other.m_s.load(std::memory_order_relaxed));
  return *this;
}

Id LowStringPtrOrId::id() const {
  auto s = m_s.load(std::memory_order_relaxed);
  if (!details::is_ptr(s)) {
    return Id(s >> 1);
  }
  return kInvalidId;
}

const StringData* LowStringPtrOrId::rawPtr() const {
  auto s = m_s.load(std::memory_order_relaxed);
  assertx(details::is_ptr(s));
  return reinterpret_cast<const StringData*>(s);
}

const StringData* LowStringPtrOrId::ptr(const Unit* unit) const {
  auto i = id();
  if (i != kInvalidId) {
    auto str = unit->lookupLitstrId(i);
    m_s.store(details::to_low(str), std::memory_order_relaxed);
    return str;
  }
  return reinterpret_cast<const StringData*>(m_s.load(std::memory_order_relaxed));
}

const StringData* LowStringPtrOrId::ptr(const UnitEmitter& ue) const {
  auto i = id();
  if (i != kInvalidId) {
    auto str = ue.lookupLitstrId(i);
    m_s.store(details::to_low(str), std::memory_order_relaxed);
    return str;
  }
  return reinterpret_cast<const StringData*>(m_s.load(std::memory_order_relaxed));
}

void BlobEncoderHelper<LowStringPtrOrId>::serde(BlobEncoder& encoder,
                                                const LowStringPtrOrId& s) {
  assertx(!BlobEncoderHelper<const StringData*>::tl_unit);
  if (auto const ue = BlobEncoderHelper<const StringData*>::tl_unitEmitter) {
    auto const sd = s.ptr(*ue);
    encoder(sd);
    return;
  }
  auto const sd = s.rawPtr();
  encoder(sd);
}

void BlobEncoderHelper<LowStringPtrOrId>::serde(BlobDecoder& decoder,
                                                LowStringPtrOrId& s) {
  if (BlobEncoderHelper<const StringData*>::tl_unitEmitter ||
      BlobEncoderHelper<const StringData*>::tl_unit) {
    Id id;
    decoder(id);
    s = LowStringPtrOrId(id);
    return;
  }

  LowStringPtr sd;
  decoder(sd);
  s = LowStringPtrOrId(sd);
}

}
