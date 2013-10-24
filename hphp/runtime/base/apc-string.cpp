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
#include "hphp/runtime/base/apc-string.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

APCHandle* APCString::MakeShared(DataType type, StringData* data) {
  auto len = data->size();

  auto const cap = static_cast<uint32_t>(len) + 1;
  auto const apcStr = new (cap) APCString(type);

  apcStr->m_data.m_lenAndCount = len;
  apcStr->m_data.m_cap         = cap;
  apcStr->m_data.m_data        = reinterpret_cast<char*>(apcStr + 1);

  apcStr->m_data.m_data[len] = 0;
  auto const mcret = memcpy(apcStr->m_data.m_data, data->data(), len);
  auto const ret   = reinterpret_cast<APCString*>(mcret) - 1;
  // Recalculating ret from mcret avoids a spill.

  ret->m_data.preCompute();

  assert(ret == apcStr);
  assert(apcStr->m_data.m_hash != 0);
  assert(ret->m_data.m_count == 0);
  assert(ret->m_data.isFlat());
  assert(ret->m_data.checkSane());
  return ret->getHandle();
}

NEVER_INLINE
StringData* StringData::MakeSVSlowPath(APCString* shared, uint32_t len) {
  auto const data = shared->getStringData();
  auto const hash = data->m_hash & STRHASH_MASK;
  auto const capAndHash = static_cast<uint64_t>(hash) << 32;

  auto const sd = static_cast<StringData*>(
      MM().smartMallocSize(sizeof(StringData) + sizeof(SharedPayload))
  );

  sd->m_data = const_cast<char*>(data->m_data);
  sd->m_lenAndCount = len;
  sd->m_capAndHash = capAndHash;

  sd->sharedPayload()->shared = shared;
  sd->enlist();
  shared->incRef();

  assert(sd->m_len == len);
  assert(sd->m_count == 0);
  assert(sd->m_cap == 0); // cap == 0 means shared
  assert(sd->m_hash == hash);
  assert(sd->checkSane());
  return sd;
}

StringData* StringData::Make(APCString* shared) {
  // No need to check if len > MaxSize, because if it were we'd never
  // have made the StringData in the APCVariant without throwing.
  assert(size_t(shared->getStringData()->size()) <= size_t(MaxSize));

  StringData *data = shared->getStringData();
  auto const len = data->size();
  if (UNLIKELY(len > SmallStringReserve)) {
    return MakeSVSlowPath(shared, len);
  }

  auto const psrc = data->data();
  auto const hash = data->m_hash & STRHASH_MASK;
  assert(hash != 0);

  auto const needed = static_cast<uint32_t>(sizeof(StringData) + len + 1);
  auto const cap = MemoryManager::smartSizeClass(needed);
  auto const sd = static_cast<StringData*>(MM().smartMallocSize(cap));
  auto const pdst = reinterpret_cast<char*>(sd + 1);

  auto const capAndHash = static_cast<uint64_t>(hash) << 32 |
                                            (cap - sizeof(StringData));

  sd->m_data = pdst;
  sd->m_lenAndCount = len;
  sd->m_capAndHash = capAndHash;

  pdst[len] = 0;
  auto const mcret = memcpy(pdst, psrc, len);
  auto const ret = reinterpret_cast<StringData*>(mcret) - 1;
  // Recalculating ret from mcret avoids a spill.

  // Note: this return value thing is doing a dead lea into %rsi in
  // the caller for some reason.

  assert(ret == sd);
  assert(ret->m_len == len);
  assert(ret->m_count == 0);
  assert(ret->m_cap == cap - sizeof(StringData));
  assert(ret->m_hash == hash);
  assert(ret->isFlat());
  assert(ret->checkSane());
  return ret;
}

APCHandle* StringData::getAPCHandle() const {
  if (isShared()) return sharedPayload()->shared->getHandle();
  return nullptr;
}

// Defined here for inlining into MakeSVSlowPath
ALWAYS_INLINE void StringData::enlist() {
 assert(isShared());
 auto& head = MM().m_strings;
 // insert after head
 auto const next = head.next;
 auto& payload = *sharedPayload();
 assert(uintptr_t(next) != kMallocFreeWord);
 payload.node.next = next;
 payload.node.prev = &head;
 next->prev = head.next = &payload.node;
}

///////////////////////////////////////////////////////////////////////////////
}
