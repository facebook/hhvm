/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-present Facebook, Inc. (http://www.facebook.com)  |
   +----------------------------------------------------------------------+
   | This source path is subject to version 3.01 of the PHP license,      |
   | that is bundled with this package in the path LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | http://www.php.net/license/3_01.txt                                  |
   | If you did not receive a copy of the PHP license and are unable to   |
   | obtain it through the world-wide-web, please send a note to          |
   | license@php.net so we can mail you a copy immediately.               |
   +----------------------------------------------------------------------+
*/

#include <lmdb.h>

#include "hphp/runtime/base/static-string-table.h"
#include "hphp/runtime/base/string-data.h"
#include "hphp/runtime/base/tv-type.h"
#include "hphp/runtime/base/type-string.h"
#include "hphp/runtime/base/unit-cache.h"
#include "hphp/runtime/base/variable-serializer.h"
#include "hphp/runtime/base/variable-unserializer.h"
#include "hphp/system/systemlib.h"

namespace HPHP {
namespace lmdb {

namespace {
const StaticString s_major("major");
const StaticString s_minor("minor");
const StaticString s_patch("patch");
const StaticString s_env_handle("env_handle");
const StaticString s_txn_handle("txn_handle");
const StaticString s_dbi_handle("dbi_handle");
const StaticString s_cursor_handle("cursor_handle");
const StaticString s_stat_psize("ms_psize");
const StaticString s_stat_depth("ms_depth");
const StaticString s_stat_branch_pages("ms_branch_pages");
const StaticString s_stat_leaf_pages("ms_leaf_pages");
const StaticString s_stat_overflow_pages("ms_overflow_pages");
const StaticString s_stat_entries("ms_entries");

} // namespace

template <typename T>
struct LmdbObjectMap {
 public:
  static LmdbObjectMap<T>* get() {
    static LmdbObjectMap* const map = new LmdbObjectMap<T>();
    return map;
  }

  unsigned long addObject(T* object) {
    objects[++counter] = object;
    return counter;
  }

  T* getObject(unsigned long handle) {
    auto iter = objects.find(handle);
    if (iter == objects.end()) {
      return nullptr;
    }
    return iter->second;
  }

  T* releaseObject(unsigned long handle) {
    auto iter = objects.find(handle);
    if (iter == objects.end()) {
      return nullptr;
    }
    auto* object = iter->second;
    objects.erase(handle);
    return object;
  }

  bool isIdValid(unsigned long id) {
    return (objects.find(id) != objects.end());
  }

 private:
  std::map<unsigned long, T*> objects;
  unsigned int counter = 0;
};

typedef struct LmdbObjectMap<MDB_cursor> LmdbCursorMap;
typedef struct LmdbObjectMap<Variant> LmdbDbiMap;
typedef struct LmdbObjectMap<MDB_env> LmdbEnvMap;
typedef struct LmdbObjectMap<MDB_txn> LmdbTxnMap;

/*
 * mdb_set_compare
 * mdb_strerror
 */

MDB_txn* maybeGetTransaction(const Array& env) {
  TypedValue tv = env->get(s_txn_handle, /* error = */ true);
  return tvIsInt(tv) ? LmdbTxnMap::get()->getObject(tv.m_data.num) : nullptr;
}

MDB_txn* getTransaction(const Array& env) {
  auto* txn = maybeGetTransaction(env);
  if (txn == nullptr) {
    HPHP::SystemLib::throwInvalidArgumentExceptionObject(
        Variant{"Invalid or unknown transaction handle"});
  }
  return txn;
}

MDB_cursor* getCursor(const Array& env) {
  TypedValue tv = env->get(s_cursor_handle, /* error = */ true);
  if (!tvIsInt(tv)) {
    HPHP::SystemLib::throwInvalidArgumentExceptionObject(
        Variant{"Invalid or unknown cursor handle"});
  }

  auto* cursor = LmdbCursorMap::get()->getObject(tv.m_data.num);
  if (cursor == nullptr) {
    HPHP::SystemLib::throwInvalidArgumentExceptionObject(
        Variant{"Invalid or unknown cursor handle"});
  }
  return cursor;
}

MDB_dbi getDbi(const Array& dbi) {
  TypedValue tv = dbi->get(s_dbi_handle, /* error = */ true);
  if (!tvIsInt(tv)) {
    HPHP::SystemLib::throwInvalidArgumentExceptionObject(
        Variant{"Invalid or unknown dbi handle"});
  }
  return tv.m_data.num;
}

MDB_env* getEnvironment(const Array& env) {
  TypedValue tv = env->get(s_env_handle, /* error = */ true);
  if (!tvIsInt(tv)) {
    HPHP::SystemLib::throwInvalidArgumentExceptionObject(
        Variant{"Invalid or unknown environment handle"});
  }

  auto* env_ptr = LmdbEnvMap::get()->getObject(tv.m_data.num);
  if (env_ptr == nullptr) {
    HPHP::SystemLib::throwInvalidArgumentExceptionObject(
        Variant{"Invalid or unknown environment handle"});
  }

  return env_ptr;
}

Array mdbStatToArray(const MDB_stat& stat) {
  Array result;
  result.set(s_stat_psize, Variant{stat.ms_psize});
  result.set(s_stat_depth, Variant{stat.ms_depth});
  result.set(s_stat_branch_pages, Variant{stat.ms_branch_pages});
  result.set(s_stat_leaf_pages, Variant{stat.ms_leaf_pages});
  result.set(s_stat_overflow_pages, Variant{stat.ms_overflow_pages});
  result.set(s_stat_entries, Variant{stat.ms_entries});
  return result;
}

#define THROW_ON_ERROR(fn)                                  \
  {                                                         \
    int err = (fn);                                         \
    if (err != 0) {                                         \
      HPHP::SystemLib::throwInvalidArgumentExceptionObject( \
          Variant{mdb_strerror(err)});                      \
    }                                                       \
  }

// int mdb_env_create(MDB_env **env);
Array HHVM_FUNCTION(lmdb_mdb_env_create) {
  MDB_env* env = nullptr;

  THROW_ON_ERROR(mdb_env_create(&env));

  auto handle = LmdbEnvMap::get()->addObject(env);
  Array r = Array::CreateDict();
  r.set(s_env_handle, Variant{handle});
  return r;
}

// int  mdb_env_open(MDB_env *env, const char *path, unsigned int flags,
// mdb_mode_t mode);
void HHVM_FUNCTION(
    lmdb_mdb_env_open,
    const Array& env,
    const String& path,
    int64_t flags,
    int64_t mode) {
  auto* env_ptr = getEnvironment(env);

  THROW_ON_ERROR(mdb_env_open(env_ptr, path.c_str(), flags, mode));
}

// void mdb_env_close(MDB_env *env);
void HHVM_FUNCTION(lmdb_mdb_env_close, const Array& env) {
  TypedValue tv = env->get(s_env_handle, /* error = */ true);
  if (!tvIsInt(tv)) {
    HPHP::SystemLib::throwInvalidArgumentExceptionObject(
        Variant{"Invalid or unknown environment handle"});
  }

  auto* env_ptr = LmdbEnvMap::get()->releaseObject(tv.m_data.num);
  if (env_ptr == nullptr) {
    HPHP::SystemLib::throwInvalidArgumentExceptionObject(
        // fmt.format("Invalid or unknown environment handle: {}",
        // tv.m_data.num)
        Variant{"Invalid or unknown environment handle"});
  }

  mdb_env_close(env_ptr);
}

// int  mdb_env_set_mapsize(MDB_env *env, size_t size);
void HHVM_FUNCTION(lmdb_mdb_env_set_mapsize, const Array& env, int64_t size) {
  if (size < 0) {
    // throw
  }
  auto* env_ptr = getEnvironment(env);
  THROW_ON_ERROR(mdb_env_set_mapsize(env_ptr, size));
}

// int  mdb_env_set_maxreaders(MDB_env *env, unsigned int readers);
void HHVM_FUNCTION(
    lmdb_mdb_env_set_maxreaders,
    const Array& env,
    int64_t readers) {
  if (readers < 0) {
    // throw;
  }

  auto* env_ptr = getEnvironment(env);
  THROW_ON_ERROR(mdb_env_set_maxreaders(env_ptr, readers));
}

// int  mdb_env_set_maxdbs(MDB_env *env, MDB_dbi dbs);
void HHVM_FUNCTION(lmdb_mdb_env_set_maxdbs, const Array& env, int64_t dbs) {
  if (dbs < 0) {
    HPHP::SystemLib::throwRuntimeExceptionObject(
        Variant{"Maximum number of dbs cannot be negative."});
  }
  auto* env_ptr = getEnvironment(env);
  THROW_ON_ERROR(mdb_env_set_maxdbs(env_ptr, dbs));
}

// int  mdb_env_stat(MDB_env *env, MDB_stat *stat);
Array HHVM_FUNCTION(lmdb_mdb_env_stat, const Array& env) {
  auto* env_ptr = getEnvironment(env);

  MDB_stat stat;
  THROW_ON_ERROR(mdb_env_stat(env_ptr, &stat));
  return mdbStatToArray(stat);
}

// int  mdb_env_get_path(MDB_env *env, const char **path);
String HHVM_FUNCTION(lmdb_mdb_env_get_path, const Array& env) {
  auto* env_ptr = getEnvironment(env);
  const char* path = nullptr;
  THROW_ON_ERROR(mdb_env_get_path(env_ptr, &path));
  return String{path};
}

// int  mdb_env_get_maxkeysize(MDB_env *env);
int64_t HHVM_FUNCTION(lmdb_mdb_env_get_maxkeysize, const Array& env) {
  auto* env_ptr = getEnvironment(env);
  return mdb_env_get_maxkeysize(env_ptr);
}

// int  mdb_txn_begin(MDB_env *env, MDB_txn *parent, unsigned int flags, MDB_txn
// **txn);
Array HHVM_FUNCTION(
    lmdb_mdb_txn_begin,
    const Array& env,
    const Variant& parent,
    int64_t flags) {
  // TODO: check validity of parent
  auto* env_ptr = getEnvironment(env);
  // auto* parent_ptr = maybeGetTransaction(parent);
  MDB_txn* parent_ptr = nullptr; // TODO

  MDB_txn* txn = nullptr;
  THROW_ON_ERROR(mdb_txn_begin(env_ptr, parent_ptr, flags, &txn));

  auto handle = LmdbTxnMap::get()->addObject(txn);

  Array result;
  result.set(s_txn_handle, Variant{handle});
  return result;
}

// int  mdb_txn_commit(MDB_txn *txn);
void HHVM_FUNCTION(lmdb_mdb_txn_commit, const Array& txn) {
  auto* txn_ptr = getTransaction(txn);
  THROW_ON_ERROR(mdb_txn_commit(txn_ptr));
}

// void mdb_txn_abort(MDB_txn *txn);
void HHVM_FUNCTION(lmdb_mdb_txn_abort, const Array& txn) {
  auto* txn_ptr = getTransaction(txn);
  mdb_txn_abort(txn_ptr);
}

int MDB_val_compare_int(MDB_val* left, MDB_val* right) {
  int64_t* l_val = (int64_t*)left->mv_data;
  int64_t* r_val = (int64_t*)right->mv_data;
  return *l_val - *r_val;
}

struct CaseInsensitiveCompare {
  bool operator()(const unsigned char& l, const unsigned char& r) {
    return std::tolower(l) < std::tolower(r);
  }
};

int MDB_val_compare_string_nocase(MDB_val* left, MDB_val* right) {
  std::string_view l((const char*)left->mv_data, left->mv_size);
  std::string_view r((const char*)right->mv_data, right->mv_size);
  std::strong_ordering res = std::lexicographical_compare_three_way(
      l.begin(), l.end(), r.begin(), r.end(), [](const char l, const char r) {
        return std::toupper(l) <=> std::toupper(r);
      });

  if (res < 0)
    return -1;
  if (res > 0)
    return 1;
  return 0;
}

// int mdb_dbi_open(MDB_txn *txn, const String& name, int flags, MDB_dbi *dbi);
Array HHVM_FUNCTION(
    lmdb_mdb_dbi_open,
    const Array& txn,
    const Variant& name,
    int64_t flags,
    int64_t comparator_type) {
  auto* txn_ptr = getTransaction(txn);

  MDB_dbi dbi;
  if (name.isString()) {
    String n = name.toString();
    THROW_ON_ERROR(mdb_dbi_open(txn_ptr, n.c_str(), flags, &dbi));
  } else {
    THROW_ON_ERROR(mdb_dbi_open(txn_ptr, nullptr, flags, &dbi));
  }

  Array result;
  result.set(s_dbi_handle, dbi);
  return result;
}

// int mdb_reader_check(MDB_env *env, int *dead);
int64_t HHVM_FUNCTION(lmdb_mdb_reader_check, const Array& env) {
  auto* env_ptr = getEnvironment(env);

  int dead = 0;
  THROW_ON_ERROR(mdb_reader_check(env_ptr, &dead));
  return dead;
}

// int mdb_stat(MDB_txn *txn, MDB_dbi dbi, MDB_stat *stat);
Array HHVM_FUNCTION(lmdb_mdb_stat, const Array& txn, const Array& dbi) {
  auto* txn_ptr = getTransaction(txn);

  MDB_stat stat;
  THROW_ON_ERROR(mdb_stat(txn_ptr, getDbi(dbi), &stat));
  return mdbStatToArray(stat);
}

namespace {
// The string used by the return value must
// outlive the return value.
MDB_val wrapStringIntoVal(const String& str) {
  MDB_val val;
  val.mv_size = str.size();
  val.mv_data = (void*)str.c_str();
  return val;
}

int64_t get(
    const Array& txn,
    const Array& dbi,
    const Variant& key,
    Variant& data,
    bool raw
) {
  auto* txn_ptr = getTransaction(txn);

  MDB_val kv;
  auto key_val = key.asTypedValue();
  if (tvIsString(key_val)) {
    kv.mv_data = (void*)key_val->m_data.pstr->data();
    kv.mv_size = key_val->m_data.pstr->size();
  } else if (tvIsInt(key_val)) {
    *((int64_t*)kv.mv_data) = key_val->m_data.num;
    kv.mv_size = sizeof(key_val->m_data.num);
  }

  MDB_val dv;
  int err = mdb_get(txn_ptr, getDbi(dbi), &kv, &dv);
  if (err != 0) {
    return err;
  }

  if (raw) {
    data = String::attach(StringData::Make((const char*)dv.mv_data, dv.mv_size, CopyString));
  } else {
    VariableUnserializer vu(
        (const char*)dv.mv_data,
        dv.mv_size,
        VariableUnserializer::Type::Serialize);
    data = vu.unserialize();
  }
  return 0;
}

MDB_val keyToMdbVal(const Variant& key) {
  MDB_val kv;
  auto key_val = key.asTypedValue();
  if (tvIsString(key_val)) {
    kv.mv_data = (void*)key_val->m_data.pstr->data();
    kv.mv_size = key_val->m_data.pstr->size();
  } else if (tvIsInt(key_val)) {
    *((int64_t*)kv.mv_data) = key_val->m_data.num;
    kv.mv_size = sizeof(key_val->m_data.num);
  }
  return kv;
}

int64_t put(
    const Array& txn,
    const Array& dbi,
    const Variant& key,
    Variant& data,
    int64_t flags,
    bool raw
) {
  auto* txn_ptr = getTransaction(txn);

  MDB_val kv = keyToMdbVal(key);

  String serialized_value;
  if (raw && data.isString()) {
    serialized_value = data.toString();
  } else {
    VariableSerializer vs(VariableSerializer::Type::Serialize);
    serialized_value = vs.serializeValue(data, true);
  }

  MDB_val dv = wrapStringIntoVal(serialized_value);

  int err = mdb_put(txn_ptr, getDbi(dbi), &kv, &dv, flags);
  if (err != 0) {
    return err;
  }

  // The inout value of dv only gets updated if this flag is set
  if ((flags & MDB_NOOVERWRITE) != 0) {
    if (raw) {
      data = String::attach(StringData::Make((const char*)dv.mv_data, dv.mv_size, CopyString));
    } else {
      VariableUnserializer vu(
          (const char*)dv.mv_data,
          dv.mv_size,
          VariableUnserializer::Type::Serialize);
      data = vu.unserialize();
    }
  }
  return 0;
}

} // namespace

// int mdb_get(MDB_txn *txn, MDB_dbi dbi, MDB_val *key, MDB_val *data);
int64_t HHVM_FUNCTION(
    lmdb_mdb_get,
    const Array& txn,
    const Array& dbi,
    const Variant& key,
    Variant& data) {
  return get(txn, dbi, key, data, false);
}

int64_t HHVM_FUNCTION(
    lmdb_mdb_get_raw,
    const Array& txn,
    const Array& dbi,
    const Variant& key,
    Variant& data) {
  return get(txn, dbi, key, data, true);
}

int64_t HHVM_FUNCTION(
    lmdb_mdb_put,
    const Array& txn,
    const Array& dbi,
    const Variant& key,
    Variant& data,
    int64_t flags) {
  return put(txn, dbi, key, data, flags, false);
}

int64_t HHVM_FUNCTION(
    lmdb_mdb_put_raw,
    const Array& txn,
    const Array& dbi,
    const Variant& key,
    Variant& data,
    int64_t flags) {
  return put(txn, dbi, key, data, flags, true);
}

// int  mdb_del(MDB_txn *txn, MDB_dbi dbi, MDB_val *key, MDB_val *data);
int64_t HHVM_FUNCTION(
    lmdb_mdb_del,
    const Array& txn,
    const Array& dbi,
    const Variant& key,
    const Variant& data) {
  auto* txn_ptr = getTransaction(txn);

  VariableSerializer vs(VariableSerializer::Type::Serialize);
  String serialized_value = vs.serializeValue(data, true);

  MDB_val kv = keyToMdbVal(key);
  MDB_val dv = wrapStringIntoVal(serialized_value);

  return mdb_del(txn_ptr, getDbi(dbi), &kv, &dv);
}

// int  mdb_cursor_open(MDB_txn *txn, MDB_dbi dbi, MDB_cursor **cursor);
Array HHVM_FUNCTION(lmdb_mdb_cursor_open, const Array& txn, const Array& dbi) {
  auto* txn_ptr = getTransaction(txn);

  MDB_cursor* cursor = nullptr;
  THROW_ON_ERROR(mdb_cursor_open(txn_ptr, getDbi(dbi), &cursor));

  auto handle = LmdbCursorMap::get()->addObject(cursor);

  Array result;
  result.set(s_cursor_handle, handle);
  return result;
}

// int  mdb_cursor_get(MDB_cursor *cursor, MDB_val *key, MDB_val *data,
// MDB_cursor_op op);
int64_t HHVM_FUNCTION(
    lmdb_mdb_cursor_get,
    const Array& cursor,
    Variant& key,
    Variant& data,
    int64_t op) {
  auto* cursor_ptr = getCursor(cursor);

  VariableSerializer vs(VariableSerializer::Type::Serialize);
  String serialized_data = vs.serializeValue(data, true);

  MDB_val kv = keyToMdbVal(key);
  MDB_val dv = wrapStringIntoVal(serialized_data);

  int err = mdb_cursor_get(cursor_ptr, &kv, &dv, (MDB_cursor_op)op);
  if (err != 0) {
    return err;
  }

  auto typed_key = key.asTypedValue();
  if (tvIsInt(typed_key)) {
    key = Variant{*((int64_t*)kv.mv_data)};
  } else if (tvIsString(typed_key)) {
    key = Variant{
        StringData::Make((const char*)kv.mv_data, kv.mv_size, CopyString)};
  }

  VariableUnserializer data_vu(
      (const char*)dv.mv_data,
      dv.mv_size,
      VariableUnserializer::Type::Serialize);
  data = data_vu.unserialize();

  return 0;
}

// void mdb_cursor_close(MDB_cursor *cursor);
void HHVM_FUNCTION(lmdb_mdb_cursor_close, const Array& cursor) {
  TypedValue tv = cursor->get(s_cursor_handle, /* error = */ true);
  if (!tvIsInt(tv)) {
    HPHP::SystemLib::throwInvalidArgumentExceptionObject(
        Variant{"Invalid or unknown cursor handle"});
  }

  auto* cursor_ptr = LmdbCursorMap::get()->releaseObject(tv.m_data.num);
  if (cursor_ptr == nullptr) {
    HPHP::SystemLib::throwInvalidArgumentExceptionObject(
        Variant{"Invalid or unknown cursor handle"});
  }

  mdb_cursor_close(cursor_ptr);
}

// char *mdb_version(int *major, int *minor, int *patch);
const StaticString s_major("major");
const StaticString s_minor("minor");
const StaticString s_patch("patch");

Array HHVM_FUNCTION(lmdb_mdb_version) {
  int major, minor, patch;
  std::ignore = mdb_version(&major, &minor, &patch);

  Array version = Array::CreateDict();
  version.set(StaticString{"major"}, Variant{major});
  version.set(StaticString{"minor"}, Variant{minor});
  version.set(StaticString{"patch"}, Variant{patch});
  return version;
}

struct LmdbExtension final : HPHP::Extension {
  LmdbExtension() : HPHP::Extension("lmdb", "0.1", "sandbox_infra") {}

  void moduleLoad(const IniSetting::Map& ini, Hdf config) override {}

  void moduleInit() override {}

  void moduleRegisterNative() override {
    HHVM_NAMED_FE(HH\\lmdb\\mdb_version, HHVM_FN(lmdb_mdb_version));
    HHVM_NAMED_FE(HH\\lmdb\\mdb_env_create, HHVM_FN(lmdb_mdb_env_create));
    HHVM_NAMED_FE(HH\\lmdb\\mdb_env_open, HHVM_FN(lmdb_mdb_env_open));
    HHVM_NAMED_FE(HH\\lmdb\\mdb_env_close, HHVM_FN(lmdb_mdb_env_close));
    HHVM_NAMED_FE(
        HH\\lmdb\\mdb_env_set_mapsize, HHVM_FN(lmdb_mdb_env_set_mapsize));
    HHVM_NAMED_FE(
        HH\\lmdb\\mdb_env_set_maxreaders, HHVM_FN(lmdb_mdb_env_set_maxreaders));
    HHVM_NAMED_FE(
        HH\\lmdb\\mdb_env_set_maxdbs, HHVM_FN(lmdb_mdb_env_set_maxdbs));
    HHVM_NAMED_FE(HH\\lmdb\\mdb_env_stat, HHVM_FN(lmdb_mdb_env_stat));
    HHVM_NAMED_FE(HH\\lmdb\\mdb_env_get_path, HHVM_FN(lmdb_mdb_env_get_path));
    HHVM_NAMED_FE(
        HH\\lmdb\\mdb_env_get_maxkeysize, HHVM_FN(lmdb_mdb_env_get_maxkeysize));
    HHVM_NAMED_FE(HH\\lmdb\\mdb_txn_begin, HHVM_FN(lmdb_mdb_txn_begin));
    HHVM_NAMED_FE(HH\\lmdb\\mdb_txn_commit, HHVM_FN(lmdb_mdb_txn_commit));
    HHVM_NAMED_FE(HH\\lmdb\\mdb_txn_abort, HHVM_FN(lmdb_mdb_txn_abort));
    HHVM_NAMED_FE(HH\\lmdb\\mdb_dbi_open, HHVM_FN(lmdb_mdb_dbi_open));
    HHVM_NAMED_FE(HH\\lmdb\\mdb_reader_check, HHVM_FN(lmdb_mdb_reader_check));
    HHVM_NAMED_FE(HH\\lmdb\\mdb_stat, HHVM_FN(lmdb_mdb_stat));
    HHVM_NAMED_FE(HH\\lmdb\\mdb_get, HHVM_FN(lmdb_mdb_get));
    HHVM_NAMED_FE(HH\\lmdb\\mdb_get_raw, HHVM_FN(lmdb_mdb_get_raw));
    HHVM_NAMED_FE(HH\\lmdb\\mdb_put, HHVM_FN(lmdb_mdb_put));
    HHVM_NAMED_FE(HH\\lmdb\\mdb_put_raw, HHVM_FN(lmdb_mdb_put_raw));
    HHVM_NAMED_FE(HH\\lmdb\\mdb_del, HHVM_FN(lmdb_mdb_del));
    HHVM_NAMED_FE(HH\\lmdb\\mdb_cursor_open, HHVM_FN(lmdb_mdb_cursor_open));
    HHVM_NAMED_FE(HH\\lmdb\\mdb_cursor_get, HHVM_FN(lmdb_mdb_cursor_get));
    HHVM_NAMED_FE(HH\\lmdb\\mdb_cursor_close, HHVM_FN(lmdb_mdb_cursor_close));
  }

  void moduleShutdown() override {}
} s_ext;

/*
 * TODO:
 *
 * Need to put some sort of cleanup on the treadmill
 * - close dbs
 * - abort transactions
 * - close env
 *
 * Thread safety?  Probably can just make everything thread local.
 *
 * Clean up dependencies and headers.
 */

} // namespace lmdb
} // namespace HPHP
