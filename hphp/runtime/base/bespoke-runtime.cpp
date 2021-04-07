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

#include "hphp/runtime/base/bespoke-runtime.h"
#include "hphp/runtime/base/array-data-defs.h"

#include "hphp/runtime/base/bespoke/struct-dict.h"

#include <algorithm>

namespace HPHP {

TRACE_SET_MOD(bespoke);

//////////////////////////////////////////////////////////////////////////////

using bespoke::StructDict;

namespace {
using StrToRSMap = folly::F14FastMap<const StringData*, RuntimeStruct*>;

StrToRSMap s_runtimeStrMap;
folly::SharedMutex s_mapLock;
}

RuntimeStruct::RuntimeStruct(
    const StringData* stableIdentifier, const FieldIndexVector& fields)
  : m_profile(nullptr)
  , m_stableIdentifier(stableIdentifier)
  , m_assignedLayout(nullptr)
  , m_fields()
  , m_fieldSlots()
{
  assertx(stableIdentifier->isStatic());

  auto maxId = 0;
  for (auto const [idx, _] : fields) {
    if (idx > maxId) maxId = idx;
  }

  auto fieldKeys = FieldKeys(maxId + 1, nullptr);
  for (auto const [idx, key] : fields) {
    auto const str = key.get();
    always_assert(str->isStatic());
    assertx(fieldKeys[idx] == nullptr);
    fieldKeys[idx] = str;
  }

  m_fields = std::move(fieldKeys);
}

RuntimeStruct::RuntimeStruct(
    const StringData* stableIdentifier, FieldKeys&& fields)
  : m_profile(nullptr)
  , m_stableIdentifier(stableIdentifier)
  , m_assignedLayout(nullptr)
  , m_fields(std::move(fields))
  , m_fieldSlots()
{
  assertx(stableIdentifier->isStatic());
}

RuntimeStruct* RuntimeStruct::registerRuntimeStruct(
    const String& stableIdentifier, const FieldIndexVector& fields) {
  assertx(stableIdentifier.get()->isStatic());

  if (!allowBespokeArrayLikes()) return nullptr;

  {
    folly::SharedMutex::ReadHolder lock{s_mapLock};
    auto it = s_runtimeStrMap.find(stableIdentifier.get());
    if (it != s_runtimeStrMap.end()) {
      auto const runtimeStruct = it->second;
      lock.unlock();
      // Validate that the fields provided in the query match the registered
      // fields to avoid catastrophic index mismatches.
      always_assert(runtimeStruct->m_fields.size() == fields.size());
      for (auto const [idx, str] : fields) {
        always_assert(runtimeStruct->m_fields[idx]->same(str.get()));
      }
      return runtimeStruct;
    }
  }

  folly::SharedMutex::WriteHolder lock{s_mapLock};
  auto const rs = new RuntimeStruct(stableIdentifier.get(), fields);
  auto const pair = s_runtimeStrMap.emplace(stableIdentifier.get(), rs);
  if (!pair.second) delete rs;

  return pair.first->second;
}

RuntimeStruct* RuntimeStruct::deserialize(
    const StringData* stableIdentifier, FieldKeys&& fields) {
  assertx(allowBespokeArrayLikes());
  auto const runtimeStruct = new RuntimeStruct(stableIdentifier, std::move(fields));
  s_runtimeStrMap.emplace(stableIdentifier, runtimeStruct);
  return runtimeStruct;
}

RuntimeStruct* RuntimeStruct::findById(const StringData* stableIdentifier) {
  assertx(allowBespokeArrayLikes());
  assertx(stableIdentifier->isStatic());

  folly::SharedMutex::ReadHolder lock{s_mapLock};
  auto const runtimeStruct = s_runtimeStrMap[stableIdentifier];
  assertx(runtimeStruct);
  return runtimeStruct;
}

void RuntimeStruct::eachRuntimeStruct(std::function<void(RuntimeStruct*)> fn) {
  assertx(allowBespokeArrayLikes());

  folly::SharedMutex::ReadHolder lock{s_mapLock};
  for (auto& it : s_runtimeStrMap) fn(it.second);
}

void RuntimeStruct::applyLayout(const bespoke::StructLayout* layout) {
  auto slots = std::vector<Slot>(m_fields.size(), kInvalidSlot);
  for (size_t i = 0; i < m_fields.size(); i++) {
    slots[i] = layout->keySlot(m_fields[i]);
  }

  m_fieldSlots = std::move(slots);
  m_assignedLayout.store(layout, std::memory_order_release);
}

const StringData* RuntimeStruct::toString() const {
  return m_stableIdentifier;
}

const StringData* RuntimeStruct::getStableIdentifier() const {
  return m_stableIdentifier;
}

size_t RuntimeStruct::stableHash() const {
  return m_stableIdentifier->hashStatic();
}

//////////////////////////////////////////////////////////////////////////////

StructDictInit::StructDictInit(RuntimeStruct* structHandle, size_t n) {
  if (allowBespokeArrayLikes() && structHandle) {
    auto const layout =
      structHandle->m_assignedLayout.load(std::memory_order_acquire);

    if (layout) {
      // We have been assigned a layout; use it for the initializer.
      m_arr = StructDict::AllocStructDict(layout);
      m_struct = structHandle;
      return;
    }
  }

  auto const ad = MixedArray::MakeReserveDict(n);
  m_arr = bespoke::maybeMakeLoggingArray(ad, structHandle);
  m_struct = nullptr;
}

StructDictInit::~StructDictInit() {
  if (!m_arr) return;

  assertx(m_arr->hasExactlyOneRef());
  m_arr->release();
}

void StructDictInit::set(size_t idx, const String& key, TypedValue value) {
  tvIncRefGen(value);
  if (m_struct) {
    assertx(m_struct->m_fields[idx]->same(key.get()));
    auto const sad = StructDict::As(m_arr);
    auto const slot = m_struct->m_fieldSlots[idx];
    if (slot == kInvalidSlot) {
      // We are adding a key with no corresponding slot. This must escalate to
      // vanilla, so clear the m_struct field.
      FTRACE(2, "StructDictInit set at key {}, escalate\n", key.get());
      m_arr = StructDict::SetStrMove(sad, key.get(), value);
      assertx(m_arr->isVanilla());
      m_struct = nullptr;
    } else {
      FTRACE(2, "StructDictInit set at key {}, slot {}\n", key.get(), slot);
      if (debug) {
        // Validate that the slot translation is correct.
        auto const DEBUG_ONLY layout =
          m_struct->m_assignedLayout.load(std::memory_order_acquire);
        assertx(layout->field(slot).key->same(key.get()));
      }
      m_arr = StructDict::SetStrInSlot(sad, value, slot);
      assertx(!m_arr->isVanilla());
    }
  } else {
    m_arr = m_arr->setMove(key, value);
  }
}

void StructDictInit::set(size_t idx, const String& key, const Variant& value) {
  set(idx, key, *value.asTypedValue());
}

Variant StructDictInit::toVariant() {
  assertx(m_arr->hasExactlyOneRef());
  assertx(m_arr->isDictType());
  auto const ptr = m_arr;
  m_arr = nullptr;
  return Variant(ptr, KindOfDict, Variant::ArrayInitCtor{});
}

Array StructDictInit::toArray() {
  assertx(m_arr->hasExactlyOneRef());
  assertx(m_arr->isDictType());
  auto const ptr = m_arr;
  m_arr = nullptr;
  return Array(ptr, Array::ArrayInitCtor::Tag);
}

ArrayData* StructDictInit::create() {
  assertx(m_arr->hasExactlyOneRef());
  assertx(m_arr->isDictType());
  auto const ptr = m_arr;
  m_arr = nullptr;
  return ptr;
}

}
