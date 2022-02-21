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

#include "hphp/runtime/vm/jit/shared-profile.h"

#include "hphp/runtime/vm/jit/decref-profile.h"
#include "hphp/runtime/vm/jit/prof-data-serialize.h"

#include <folly/SharedMutex.h>

namespace HPHP { namespace jit {

//////////////////////////////////////////////////////////////////////////////

namespace {

//////////////////////////////////////////////////////////////////////////////

/*
 * To add a new type of SharedProfile, you must:
 *
 *  1. Add your profile type T to this X-macro.
 *
 *  2. Implement the following methods on T:
 *
 *      void reduce(T& out, const T& in);
 *      void deserialize(ProfDataDeserializer& des);
 *      void serialize(ProfDataSerializer& ser) const;
 *
 *  There's no constraint on the signature of the profile's update method,
 *  because SharedProfileEntry<T>::update() takes a callback.
 */
#define SHARED_PROFILE_TYPES \
  X(DecRefProfile)

enum class SharedProfileType {
#define X(name) name,
  SHARED_PROFILE_TYPES
#undef X
};

SharedProfileType typeForTypeIndex(std::type_index type) {
#define X(name)                                        \
    if (type == std::type_index(typeid(name))) { \
      return SharedProfileType::name;                  \
    }
  SHARED_PROFILE_TYPES
#undef X
  always_assert_flog(false, "Unsupported type: {}", type.name());
}

size_t sizeofType(SharedProfileType type) {
  switch (type) {
#define X(name) \
    case SharedProfileType::name: return sizeof(SharedProfileEntry<name>);
  SHARED_PROFILE_TYPES
#undef X
  }
  always_assert(false);
}

void init_value(SharedProfileType type, char* mem) {
  switch (type) {
#define X(name) \
    case SharedProfileType::name: new (mem) SharedProfileEntry<name>(); return;
  SHARED_PROFILE_TYPES
#undef X
  }
  always_assert(false);
}

void read_value(ProfDataDeserializer& des, SharedProfileType type, char* data) {
  switch (type) {
#define X(name)                                                             \
    case SharedProfileType::name: {                                         \
      auto const entry = reinterpret_cast<SharedProfileEntry<name>*>(data); \
      return entry->value.deserialize(des);                                 \
    }
  SHARED_PROFILE_TYPES
#undef X
  }
}

void write_value(ProfDataSerializer& ser, SharedProfileType type, char* data) {
  switch (type) {
#define X(name)                                                             \
    case SharedProfileType::name: {                                         \
      auto const entry = reinterpret_cast<SharedProfileEntry<name>*>(data); \
      std::lock_guard<std::mutex> _(entry->mutex);                          \
      return entry->value.serialize(ser);                                   \
    }
  SHARED_PROFILE_TYPES
#undef X
  }
}

#undef SHARED_PROFILE_TYPES

//////////////////////////////////////////////////////////////////////////////

struct SharedProfileKey {
  TransID tid;
  Offset bcOff;
  LowStringPtr name;
  SharedProfileType type;
};

struct SharedProfileKeyHashCompare {
  bool equal(const SharedProfileKey& a, const SharedProfileKey& b) const {
    return a.tid == b.tid && a.bcOff == b.bcOff &&
           a.name == b.name && a.type == b.type;
  }
  size_t hash(const SharedProfileKey& a) const {
    assertx(a.name->isStatic());
    return folly::hash::hash_combine(a.tid, a.bcOff, a.name->hash(), a.type);
  }
};

SharedProfileKey read_key(ProfDataDeserializer& des) {
  auto key = SharedProfileKey{};
  read_raw(des, key.tid);
  read_raw(des, key.bcOff);
  key.name = read_string(des);
  read_raw(des, key.type);
  return key;
}

void write_key(ProfDataSerializer& ser, const SharedProfileKey& key) {
  write_raw(ser, key.tid);
  write_raw(ser, key.bcOff);
  write_string(ser, key.name);
  write_raw(ser, key.type);
}

struct SharedProfileData {
  char* lookup(const SharedProfileKey& key, bool create) {
    auto const size = sizeofType(key.type);

    {
      Map::const_accessor it;
      if (map.find(it, key)) return it->second;
      if (!create || done.load(std::memory_order_acquire)) return nullptr;
    }

    folly::SharedMutex::ReadHolder lock{done_lock};
    if (done.load(std::memory_order_acquire)) return nullptr;

    Map::accessor it;
    if (map.insert(it, key)) {
      auto const offset = frontier.fetch_add(size);
      auto const result = &getRawData()[offset];
      always_assert(offset + size < capacity);
      init_value(key.type, result);
      it->second = result;
    }
    return it->second;
  }

  void deserialize(ProfDataDeserializer& des) {
    markDoneProfiling();
    always_assert(map.empty());
    auto const data = getRawData();
    auto const size = read_raw<size_t>(des);
    for (auto i = 0; i < size; i++) {
      auto const key = read_key(des);
      auto const offset = frontier.fetch_add(sizeofType(key.type));
      auto const result = &data[offset];
      init_value(key.type, result);
      read_value(des, key.type, result);

      Map::accessor it;
      auto const inserted = map.insert(it, key);
      always_assert(inserted);
      it->second = result;
    }
    always_assert(map.size() == size);
  }

  void serialize(ProfDataSerializer& ser) {
    markDoneProfiling();
    write_raw<size_t>(ser, map.size());
    for (auto const& pair : map) {
      write_key(ser, pair.first);
      write_value(ser, pair.first.type, pair.second);
    }
  }

private:
  using Map = tbb::concurrent_hash_map<
    SharedProfileKey, char*, SharedProfileKeyHashCompare>;

  void markDoneProfiling() {
    folly::SharedMutex::WriteHolder lock{done_lock};
    done.store(true, std::memory_order_release);
  }

  char* getRawData() {
    if (allocation == nullptr) {
      capacity = RuntimeOption::EvalSharedProfileSize;
      auto const alloc = mmap(nullptr, capacity, PROT_READ|PROT_WRITE,
                              MAP_ANONYMOUS|MAP_PRIVATE, -1, 0);
      always_assert(alloc != MAP_FAILED);
      allocation = reinterpret_cast<char*>(alloc);
    }
    return allocation;
  }

  size_t capacity = 0;
  char* allocation = nullptr;
  std::atomic<size_t> frontier = 0;
  std::atomic<bool> done = false;
  folly::SharedMutex done_lock;
  Map map;
};

SharedProfileData s_data;

}

//////////////////////////////////////////////////////////////////////////////

char* fetchSharedProfile(
    TransID tid, Offset bcOff, const StringData* name,
    bool create, std::type_index type) {
  auto const key = SharedProfileKey{tid, bcOff, name, typeForTypeIndex(type)};
  return s_data.lookup(key, create);
}

void deserializeSharedProfiles(ProfDataDeserializer& des) {
  s_data.deserialize(des);
}

void serializeSharedProfiles(ProfDataSerializer& ser) {
  s_data.serialize(ser);
}

///////////////////////////////////////////////////////////////////////////////

}}
