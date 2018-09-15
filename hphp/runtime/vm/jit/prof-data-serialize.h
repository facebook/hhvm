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

#ifndef incl_HPHP_PROF_DATA_SERIALIZE_H_
#define incl_HPHP_PROF_DATA_SERIALIZE_H_

#include <string>
#include "hphp/runtime/vm/jit/containers.h"
#include "hphp/runtime/base/repo-auth-type-array.h"

namespace HPHP {
//////////////////////////////////////////////////////////////////////

struct Unit;
struct Class;
struct Func;
struct StringData;
struct ArrayData;
struct SrcKey;
struct TypeAliasReq;

namespace jit {
//////////////////////////////////////////////////////////////////////

struct ProfDataSerializer {
  explicit ProfDataSerializer(const std::string& name);
  ~ProfDataSerializer();

  friend void write_raw(ProfDataSerializer& ser, const void* data, size_t sz);

  bool serialize(const Unit* unit);
  bool serialize(const Func* func);
  bool serialize(const Class* cls);

  template<typename T>
  bool serialize(const T* x) {
    return serializedStatics.emplace(x).second;
  }
  template<typename T>
  bool wasSerialized(const T* x) {
    return serializedStatics.count(x);
  }

  // Atomically create the output file, or throw runtime error upon failure.
  void finalize();
private:
  int fd;
  static constexpr uint32_t buffer_size = 8192;
  uint32_t offset{0};
  char buffer[buffer_size];
  const std::string& fileName;
  // keep track of things that have already been serialized.
  jit::fast_set<const void*> serializedStatics;
};

struct ProfDataDeserializer {
  template<typename T>
  using EntMap = jit::hash_map<uintptr_t, T>;

  explicit ProfDataDeserializer(const std::string& name);
  ~ProfDataDeserializer();

  friend void read_raw(ProfDataDeserializer& ser, void* data, size_t sz);

  template<typename T>
  T* remap(T* elm) {
    auto const ent = getEnt(elm);
    assertx(ent);
    return ent;
  }

  /*
   * Create a mapping from the funcId that was serialized to the
   * corresponding funcId in this run.
   */
  void recordFid(uint32_t origId, uint32_t newId) {
    auto const DEBUG_ONLY inserted = fidMap.emplace(origId, newId).second;
    assertx(inserted);
  }

  /*
   * Get the funcId in this run corresponding to the one that got
   * serialized.
   */
  uint32_t getFid(uint32_t origId) {
    auto const it = fidMap.find(origId);
    assertx(it != fidMap.end());
    return it->second;
  }

  void recordRat(const RepoAuthType::Array* origRat,
                 const RepoAuthType::Array* newRat) {
    auto const DEBUG_ONLY inserted = ratMap.emplace(
      reinterpret_cast<uintptr_t>(origRat), newRat).second;
    assertx(inserted);
  }

  StringData*& getEnt(const StringData* p);
  ArrayData*& getEnt(const ArrayData* p);
  Unit*& getEnt(const Unit* p);
  Func*& getEnt(const Func* p);
  Class*& getEnt(const Class* p);
  const RepoAuthType::Array*& getEnt(const RepoAuthType::Array* p);

  bool done();
 private:
  int fd;
  static constexpr uint32_t buffer_size = 8192;
  uint32_t offset{buffer_size};
  char buffer[buffer_size];

  EntMap<StringData*>  stringMap;
  EntMap<ArrayData*>   arrayMap;
  EntMap<Unit*>        unitMap;
  EntMap<Func*>        funcMap;
  EntMap<Class*>       classMap;
  EntMap<const RepoAuthType::Array*> ratMap;
  jit::fast_map<uint32_t, uint32_t> fidMap;
};

template<class T>
void write_raw(ProfDataSerializer& ser, const T& t) {
  write_raw(ser, &t, sizeof(t));
}

template<class T>
void read_raw(ProfDataDeserializer& ser, T& t) {
  read_raw(ser, &t, sizeof(t));
}

template<class T>
T read_raw(ProfDataDeserializer& ser) {
  std::aligned_storage_t<sizeof(T), alignof(T)> t;
  read_raw(ser, t);
  return reinterpret_cast<T&>(t);
}

void write_string(ProfDataSerializer& ser, const StringData* str);
StringData* read_string(ProfDataDeserializer& ser);
void write_array(ProfDataSerializer& ser, const ArrayData* arr);
ArrayData* read_array(ProfDataDeserializer& ser);
void write_unit(ProfDataSerializer& ser, const Unit* unit);
Unit* read_unit(ProfDataDeserializer& ser);
void write_class(ProfDataSerializer& ser, const Class* cls);
Class* read_class(ProfDataDeserializer& ser);
void write_func(ProfDataSerializer& ser, const Func* func);
Func* read_func(ProfDataDeserializer& ser);

// Return an empty string upon success, and a string that describes the reason
// of failure otherwise.
std::string serializeProfData(const std::string& filanme);
std::string deserializeProfData(const std::string& filanme, int numWorkers);

//////////////////////////////////////////////////////////////////////
} }

#endif
