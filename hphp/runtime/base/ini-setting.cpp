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

#include "hphp/runtime/base/ini-setting.h"

#include "hphp/runtime/base/array-init.h"
#include "hphp/runtime/base/array-iterator.h"
#include "hphp/runtime/base/builtin-functions.h"
#include "hphp/runtime/base/execution-context.h"
#include "hphp/runtime/base/init-fini-node.h"
#include "hphp/runtime/base/req-optional.h"
#include "hphp/runtime/base/stat-cache.h"
#include "hphp/runtime/base/string-functors.h"
#include "hphp/runtime/base/runtime-option.h"

#include "hphp/runtime/base/ini-parser/zend-ini.h"

#include "hphp/runtime/ext/std/ext_std_misc.h"
#include "hphp/runtime/ext/extension.h"
#include "hphp/runtime/ext/extension-registry.h"
#include "hphp/runtime/ext/json/ext_json.h"

#include "hphp/util/build-info.h"
#include "hphp/util/lock.h"
#include "hphp/util/portability.h"
#include "hphp/util/logger.h"

#include "hphp/zend/zend-strtod.h"

#include <glob.h>

#define __STDC_LIMIT_MACROS
#include <cstdint>
#include <boost/range/join.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string.hpp>
#include <map>

#include <folly/container/F14Map.h>
#include <folly/Hash.h>
#include <folly/FileUtil.h>
#include <folly/Overload.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

const Extension* IniSetting::CORE = (Extension*)(-1);

bool IniSetting::s_config_is_a_constant = false;
std::set<std::string> IniSetting::config_names_that_use_constants;
bool IniSetting::s_system_settings_are_set = false;

const StaticString
  s_global_value("global_value"),
  s_local_value("local_value"),
  s_access("access"),
  s_core("core");

std::vector<std::string> split_brackets(const std::string& s) {
  std::vector<std::string> split_value;
  boost::split(split_value, s, boost::is_any_of("[]"));
  // Splitting this way might give us an empty string at the end
  if (split_value.back() == "") {
    split_value.pop_back();
  }
  return split_value;
}

int64_t convert_bytes_to_long(folly::StringPiece value) {
  if (value.empty()) {
    return 0;
  }
  int64_t newInt = strtoll(value.begin(), nullptr, 10);
  auto const lastChar = value[value.size() - 1];
  if (lastChar == 'K' || lastChar == 'k') {
    newInt <<= 10;
  } else if (lastChar == 'M' || lastChar == 'm') {
    newInt <<= 20;
  } else if (lastChar == 'G' || lastChar == 'g') {
    newInt <<= 30;
  }
  return newInt;
}

std::string convert_long_to_bytes(int64_t value) {
  // Only return a larger value if it wouldn't
  // be truncating the precision.
  if ((value & ((1 << 30) - 1)) == 0) {
    return std::to_string(value / (1 << 30)) + "G";
  } else if ((value & ((1 << 20) - 1)) == 0) {
    return std::to_string(value / (1 << 20)) + "M";
  } else if ((value & ((1 << 10) - 1)) == 0) {
    return std::to_string(value / (1 << 10)) + "K";
  } else {
    return std::to_string(value);
  }
}

namespace {

template<typename T>
void set(DictInit& arr, const std::string& key, const T& value) {
  auto keyStr = String(key);
  int64_t n;
  if (keyStr.get()->isStrictlyInteger(n)) {
    arr.set(n, value);
    return;
  }
  arr.set(keyStr.get(), value);
}

template<typename T>
Variant ini_get_vec(std::vector<T>& p) {
  VecInit ret(p.size());
  for (auto& s : p) {
    ret.append(s);
  }
  return ret.toArray();
}

template<class T> struct PreserveOrder { static constexpr bool val = false; };

template<class... Args>
struct PreserveOrder<std::set<Args...>> { static constexpr bool val = true; };

template<class... Args>
struct PreserveOrder<std::map<Args...>> { static constexpr bool val = true; };

template<class... Args>
struct PreserveOrder<boost::container::flat_set<Args...>> {
  static constexpr bool val = true;
};

template<class T>
size_t hash_collection(const T& c) {
  if constexpr (PreserveOrder<T>::val) {
    return folly::hash::hash_range(c.begin(), c.end());
  }
  return folly::hash::commutative_hash_combine_range(c.begin(), c.end());
}

}

#define INI_ASSERT_STR(v) \
  if (!v.isScalar()) { \
    return false; \
  } \
  auto str = v.toString().toCppString();

#define INI_ASSERT_ARR(v) \
  if (!value.isArray() && !value.isObject()) { \
    return false; \
  }

#define INI_ASSERT_ARR_INNER(v, t) \
  if (!value.isArray() && !value.isObject()) { \
    return false; \
  } \
  for (ArrayIter iter(value.toArray()); iter; ++iter) { \
    if (!iter.second().is##t()) return false; \
  }

#define N(Ty) \
  Variant ini_get(Ty& p) { return p; }                                    \
  bool ini_on_update(const Variant& value, Ty& p) {                       \
    INI_ASSERT_STR(value);                                                \
    auto n = convert_bytes_to_long(str);                                  \
    using L = std::numeric_limits<Ty>;                                    \
    if (n > L::max() || n < L::min()) return false;                       \
    p = n;                                                                \
    return true;                                                          \
  }                                                                       \
  static void ini_log(Ty& t, const char* name, StructuredLogEntry& ent) { \
    ent.setInt(name, t);                                                  \
  }                                                                       \
  static size_t ini_hash(Ty& t) { return std::hash<Ty>{}(t); }            \
  static folly::dynamic ini_dyn(Ty& t) { return t; }                      \
/**/

#define U(Ty) \
  Variant ini_get(Ty& p) { return p; }                                    \
  bool ini_on_update(const Variant& value, Ty& p) {                       \
    INI_ASSERT_STR(value);                                                \
    auto n = convert_bytes_to_long(str);                                  \
    auto mask = ~size_t((Ty)(-1));                                        \
    if (size_t(n) & mask) return false;                                   \
    p = n;                                                                \
    return true;                                                          \
  }                                                                       \
  static void ini_log(Ty& t, const char* name, StructuredLogEntry& ent) { \
    ent.setInt(name, t);                                                  \
  }                                                                       \
  static size_t ini_hash(Ty& t) { return std::hash<Ty>{}(t); }            \
  static folly::dynamic ini_dyn(Ty& t) { return t; }                      \
/**/

#define M(Ty)                                                             \
  Variant ini_get(Ty& p) {                                                \
    DictInit ret(p.size());                                               \
    for (auto& [f, s] : p) {                                              \
      set(ret, f, s);                                                     \
    }                                                                     \
    return ret.toArray();                                                 \
  }                                                                       \
  bool ini_on_update(const Variant& value, Ty& p) {                       \
    INI_ASSERT_ARR_INNER(value, String);                                  \
    for (ArrayIter iter(value.toArray()); iter; ++iter) {                 \
      p[iter.first().toString().toCppString()] =                          \
        iter.second().toString().toCppString();                           \
    }                                                                     \
    return true;                                                          \
  }                                                                       \
  static void ini_log(Ty& t, const char* name, StructuredLogEntry& ent) { \
    std::vector<std::string> strs;                                        \
    std::set<folly::StringPiece> tags;                                    \
    for (auto& [k, v] : t) {                                              \
      strs.emplace_back(folly::to<std::string>(k, ": ", v));              \
      tags.emplace(strs.back());                                          \
    }                                                                     \
    ent.setSet(name, tags);                                               \
  }                                                                       \
  static size_t ini_hash(Ty& t) {                                         \
    return hash_collection(t);                                            \
  }                                                                       \
  static folly::dynamic ini_dyn(Ty& t) {                                  \
    folly::dynamic obj = folly::dynamic::object;                          \
    for (auto& [k, v] : t) {                                              \
      obj[k] = v;                                                         \
    }                                                                     \
    return obj;                                                           \
  }                                                                       \
  /**/

#define S(Ty)                                                             \
  Variant ini_get(Ty& p) {                                                \
    VecInit ret(p.size());                                                \
    for (auto& s : p) {                                                   \
      ret.append(s);                                                      \
    }                                                                     \
    return ret.toArray();                                                 \
  }                                                                       \
  bool ini_on_update(const Variant& value, Ty& p) {                       \
    INI_ASSERT_ARR_INNER(value, String);                                  \
    for (ArrayIter iter(value.toArray()); iter; ++iter) {                 \
      p.insert(iter.second().toString().toCppString());                   \
    }                                                                     \
    return true;                                                          \
  }                                                                       \
  static void ini_log(Ty& t, const char* name, StructuredLogEntry& ent) { \
    std::set<folly::StringPiece> tags;                                    \
    for (auto& v : t) tags.emplace(v);                                    \
    ent.setSet(name, tags);                                               \
  }                                                                       \
  static size_t ini_hash(Ty& t) {                                         \
    return hash_collection(t);                                            \
  }                                                                       \
  static folly::dynamic ini_dyn(Ty& t) {                                  \
    auto arr = folly::dynamic::array();                                   \
    for (auto& v : t) arr.push_back(v);                                   \
    return arr;                                                           \
  }                                                                       \
  /**/

INI_TYPES4(N, U, M, S)

#undef S
#undef M
#undef U
#undef N

Variant ini_get(bool& p) { return p ? "1" : ""; }
bool ini_on_update(const Variant& value, bool& p) {
  INI_ASSERT_STR(value);
  if ((str.size() == 0) ||
      (str.size() == 1 && strcasecmp("0", str.data()) == 0) ||
      (str.size() == 2 && strcasecmp("no", str.data()) == 0) ||
      (str.size() == 3 && strcasecmp("off", str.data()) == 0) ||
      (str.size() == 5 && strcasecmp("false", str.data()) == 0)) {
    p = false;
  } else {
    p = true;
  }
  return true;
}
static void ini_log(bool& t, const char* name, StructuredLogEntry& ent) {
  ent.setInt(name, t ? 1 : 0);
}
static folly::dynamic ini_dyn(bool& t) { return t; }
static size_t ini_hash(bool& t) { return std::hash<bool>{}(t); }

Variant ini_get(double& p) { return p; }
bool ini_on_update(const Variant& value, double& p) {
  INI_ASSERT_STR(value);
  p = zend_strtod(str.data(), nullptr);
  return true;
}
static void ini_log(double& t, const char* name, StructuredLogEntry& ent) {
  ent.setStr(name, folly::to<std::string>(t));
}
static folly::dynamic ini_dyn(double& t) { return t; }
static size_t ini_hash(double& t) { return std::hash<double>{}(t); }

Variant ini_get(std::string& p) { return p.data(); }
bool ini_on_update(const Variant& value, std::string& p) {
  INI_ASSERT_STR(value);
  p = str;
  return true;
}
static void ini_log(std::string& t, const char* name, StructuredLogEntry& ent) {
  ent.setStr(name, t);
}
static folly::dynamic ini_dyn(std::string& t) { return t; }
static size_t ini_hash(std::string& t) { return std::hash<std::string>{}(t); }

Variant ini_get(std::unordered_map<std::string, int>& p) {
  DictInit ret(p.size());
  for (auto& pair : p) {
    set(ret, pair.first, pair.second);
  }
  return ret.toArray();
}
bool ini_on_update(const Variant& value,
                   std::unordered_map<std::string, int>& p) {
  INI_ASSERT_ARR_INNER(value, Integer);
  for (ArrayIter iter(value.toArray()); iter; ++iter) {
    p[iter.first().toString().toCppString()] = iter.second().toInt64();
  }
  return true;
}
static void ini_log(std::unordered_map<std::string, int>& t,
                    const char* name, StructuredLogEntry& ent) {
  std::vector<std::string> strs;
  std::set<folly::StringPiece> tags;
  for (auto& [k, v] : t) {
    strs.emplace_back(folly::to<std::string>(k, ": ", v));
    tags.emplace(strs.back());
  }
  ent.setSet(name, tags);
}
static folly::dynamic ini_dyn(std::unordered_map<std::string, int>& t) {
  folly::dynamic obj = folly::dynamic::object;
  for (auto& [k, v] : t) obj[k] = v;
  return obj;
}
static size_t ini_hash(std::unordered_map<std::string, int>& t) {
  return hash_collection(t);
}

Variant ini_get(Array& p) { return p; }
bool ini_on_update(const Variant& value, Array& p) {
  INI_ASSERT_ARR(value);
  p = value.toArray();
  return true;
}
static void ini_log(Array& t, const char* name, StructuredLogEntry& ent) {
  // Do nothing
}
static size_t ini_hash(Array&) {
  return 0; // don't bother hashing arrays as they aren't included in logging
}
static folly::dynamic ini_dyn(Array& t) {
  // Ignore
  return folly::dynamic::array();
}

Variant ini_get(std::vector<uint32_t>& p) { return ini_get_vec(p); }
bool ini_on_update(const Variant& value, std::vector<uint32_t>& p) {
  INI_ASSERT_ARR_INNER(value, Integer);
  for (ArrayIter iter(value.toArray()); iter; ++iter) {
    p.push_back(iter.second().toInt64());
  }
  return true;
}
static void ini_log(std::vector<uint32_t>& t, const char* name,
                    StructuredLogEntry& ent) {
  std::vector<std::string> strs;
  std::vector<folly::StringPiece> v;
  for (auto i : t) {
    strs.emplace_back(folly::to<std::string>(i));
    v.emplace_back(strs.back());
  }
  ent.setVec(name, v);
}
static folly::dynamic ini_dyn(std::vector<uint32_t>& t) {
  auto arr = folly::dynamic::array();
  for (auto i : t) arr.push_back(i);
  return arr;
}
static size_t ini_hash(std::vector<uint32_t>& t) {
  return folly::hash::hash_range(t.begin(), t.end());
}

Variant ini_get(std::vector<std::string>& p) { return ini_get_vec(p); }
bool ini_on_update(const Variant& value, std::vector<std::string>& p) {
  INI_ASSERT_ARR_INNER(value, String);
  for (ArrayIter iter(value.toArray()); iter; ++iter) {
    p.push_back(iter.second().toString().toCppString());
  }
  return true;
}
static void ini_log(std::vector<std::string>& t, const char* name,
                    StructuredLogEntry& ent) {
  std::vector<folly::StringPiece> v;
  for (auto& s : t) v.emplace_back(s);
  ent.setVec(name, v);
}
static folly::dynamic ini_dyn(std::vector<std::string>& t) {
  auto arr = folly::dynamic::array();
  for (auto& s : t) arr.push_back(s);
  return arr;
}
static size_t ini_hash(std::vector<std::string>& t) {
  return folly::hash::hash_range(t.begin(), t.end());
}

const IniSettingMap ini_iterate(const IniSettingMap &ini,
                                const std::string &name) {
  // This should never happen, but handle it anyway.
  if (ini.isNull()) {
    return init_null();
  }

  // If for some reason we are passed a string (i.e., a leaf value),
  // just return it back
  if (ini.isString()) {
    return ini;
  }

  // If we just passed in a name that already has a value like:
  //   hhvm.server.apc.ttl_limit
  //   max_execution_time
  // then we just return the value now.
  // i.e., a value that didn't look like
  //   hhvm.a.b[c][d], where name = hhvm.a.b.c.d
  //   c[d] (where ini is already hhvm.a.b), where name = c.d
  auto value = ini[name];
  if (!value.isNull()) {
    return value;
  }

  // Otherwise, we split on the dots (if any) to see if we can get a real value
  std::vector<std::string> dot_parts;
  folly::split('.', name, dot_parts);

  int dot_loc = 0;
  int dot_parts_size = dot_parts.size();
  std::string part = dot_parts[0];
  value = ini[part];
  // Loop through the dot parts, getting a pointer to each
  // We may need to concatenate dots to be able to get a real value
  // e.g., if someone passed in hhvm.a.b.c.d, which in ini was equal
  // to hhvm.a.b[c][d], then we would start with hhvm and get null,
  // then hhvm.a and get null, then hhvm.a.b and actually get an object
  // to point to.
  while (value.isNull() && dot_loc < dot_parts_size - 1) {
    dot_loc++;
    part = part + "." + dot_parts[dot_loc];
    value = ini[part];
  }
  // Get to the last dot part and get its value, if it exists
  for (int i = dot_loc + 1; i < dot_parts_size; i++) {
    if (!value.isNull()) {
      part = dot_parts[i];
      value = value[part];
    } else { // If we reach a bad point, just return null
      return init_null();
    }
  }
  return value;
}

///////////////////////////////////
// IniSettingMap

IniSettingMap::IniSettingMap() {
  m_map = Variant(Array::CreateDict());
}

IniSettingMap::IniSettingMap(Type /*t*/) : IniSettingMap() {}

IniSettingMap::IniSettingMap(const IniSettingMap& i) {
  m_map = i.m_map;
}

IniSettingMap::IniSettingMap(IniSettingMap&& i) noexcept {
  m_map = std::move(i.m_map);
}

/* implicit */ IniSettingMap::IniSettingMap(const Variant& v) {
  m_map = v;
}

const IniSettingMap IniSettingMap::operator[](const String& key) const {
  assertx(this->isArray());
  int64_t intish;
  if (key.get()->isStrictlyInteger(intish)) {
    return IniSettingMap(m_map.asCArrRef()[intish]);
  }
  return IniSettingMap(m_map.asCArrRef()[key]);
}

IniSettingMap& IniSettingMap::operator=(const IniSettingMap& i) {
  m_map = i.m_map;
  return *this;
}

namespace {
void mergeSettings(tv_lval curval, TypedValue v) {
  if (isArrayLikeType(v.m_type) &&
      isArrayLikeType(curval.type())) {
    for (auto i = ArrayIter(v.m_data.parr); !i.end(); i.next()) {
      auto& cur_inner_ref = asArrRef(curval);
      if (!cur_inner_ref.exists(i.first())) {
        cur_inner_ref.set(i.first(), Array::CreateDict());
      }
      mergeSettings(cur_inner_ref.lval(i.first()), i.secondVal());
    }
  } else {
    tvSet(tvToInit(v), curval);
  }
}
}

void IniSettingMap::set(const String& key, const Variant& v) {
  assertx(this->isArray());
  auto& mapref = m_map.asArrRef();
  if (!mapref.exists(key)) {
    mapref.set(key, Array::CreateDict());
  }
  auto const curval = mapref.lval(key);
  mergeSettings(curval, *v.asTypedValue());
}

///////////////////////////////////////////////////////////////////////////////
// callbacks for creating arrays out of ini

Array IniSetting::ParserCallback::emptyArrayForMode() const {
  switch (mode_) {
    case ParserCallbackMode::DARRAY:
    case ParserCallbackMode::DICT:
      return Array::CreateDict();
  }
  not_reached();
}

Array& IniSetting::ParserCallback::forceToArrayForMode(Variant& var) const {
  switch (mode_) {
    case ParserCallbackMode::DARRAY:
    case ParserCallbackMode::DICT:
      return forceToDict(var);
  }
  not_reached();
}

Array& IniSetting::ParserCallback::forceToArrayForMode(tv_lval var) const {
  switch (mode_) {
    case ParserCallbackMode::DARRAY:
    case ParserCallbackMode::DICT:
      return forceToDict(var);
  }
  not_reached();
}

void IniSetting::ParserCallback::onSection(const std::string& /*name*/,
                                           void* /*arg*/) {
  // do nothing
}

void IniSetting::ParserCallback::onLabel(const std::string& /*name*/,
                                         void* /*arg*/) {
  // do nothing
}

void IniSetting::ParserCallback::onEntry(
    const std::string &key, const std::string &value, void *arg) {
  auto arr = static_cast<Variant*>(arg);
  String skey(key);
  Variant sval(value);
  forceToArrayForMode(*arr).set(skey, sval);
}

void IniSetting::ParserCallback::onPopEntry(
    const std::string &key,
    const std::string &value,
    const std::string &offset,
    void *arg) {
  auto arr = static_cast<Variant*>(arg);

  String skey(key);
  auto& arr_ref = forceToArrayForMode(*arr);
  if (!arr_ref.exists(skey)) {
    arr_ref.set(skey, emptyArrayForMode());
  }
  auto const hash = arr_ref.lval(skey);
  forceToArrayForMode(hash);
  if (!offset.empty()) {                 // a[b]
    makeArray(hash, offset, value);
  } else {                               // a[]
    asArrRef(hash).append(value);
  }
}

void IniSetting::ParserCallback::makeArray(tv_lval val,
                                           const std::string& offset,
                                           const std::string& value) {
  assertx(!offset.empty());
  assertx(variant_ref{val}.isArray());
  auto start = offset.c_str();
  auto p = start;
  bool last = false;
  for (;;) {
    String index(p);
    last = p + index.size() >= start + offset.size();
    // This is mandatory in case we have a nested array like:
    //   hhvm.a[b][c][d]
    // b will be hash and an array already, but c and d might
    // not exist and will need to be made an array
    auto& arr = forceToArrayForMode(val);
    Variant key = index;
    int64_t intish;
    if (index.get()->isStrictlyInteger(intish)) {
      key = intish;
    }
    if (last) {
      arr.set(key, value);
      break;
    }
    // Similar to the above, in the case of a nested array we need to ensure
    // that we create empty arrays on the way down if they don't already exist.
    if (!arr.exists(key)) {
      arr.set(key, emptyArrayForMode());
    }
    val = arr.lval(key);
    p += index.size() + 1;
  }
}

void IniSetting::ParserCallback::onConstant(std::string &result,
                                            const std::string &name) {
  if (HHVM_FN(defined)(name)) {
    result = HHVM_FN(constant)(name).toString().toCppString();
  } else {
    result = name;
  }
}

void IniSetting::ParserCallback::onVar(std::string &result,
                                       const std::string& name) {
  result.clear();
  if (IniSetting::Get(name, result)) return;

  if (g_context) {
    auto const value = g_context->getenv(name);
    if (value) result = value.toCppString();
    return;
  }

  // We read -c ini files before ExecutionContext is initialized, so we can't
  // use the m_env cache or CLI-server-specific env variables in that case.
  auto const value = ::getenv(name.data());
  if (value) result = value;
}

void IniSetting::ParserCallback::onOp(
    std::string &result, char type, const std::string& op1,
    const std::string& op2) {
  int i_op1 = strtoll(op1.c_str(), nullptr, 10);
  int i_op2 = strtoll(op2.c_str(), nullptr, 10);
  int i_result = 0;
  switch (type) {
    case '|': i_result = i_op1 | i_op2; break;
    case '&': i_result = i_op1 & i_op2; break;
    case '^': i_result = i_op1 ^ i_op2; break;
    case '~': i_result = ~i_op1;        break;
    case '!': i_result = !i_op1;        break;
  }
  result = std::to_string((int64_t)i_result);
}

void IniSetting::SectionParserCallback::onSection(
    const std::string &name, void *arg) {
  auto const data = (CallbackData*)arg;
  if (!data->active_name.isNull()) {
    data->arr.asArrRef().set(data->active_name, data->active_section);
    data->active_section.unset();
  }
  data->active_section = emptyArrayForMode();
  data->active_name = String(name);
}

Variant* IniSetting::SectionParserCallback::activeArray(CallbackData* data) {
  if (!data->active_section.isNull()) {
    return &data->active_section;
  } else {
    return &data->arr;
  }
}

void IniSetting::SectionParserCallback::onLabel(const std::string &name,
                                                void *arg) {
  IniSetting::ParserCallback::onLabel(name, activeArray((CallbackData*)arg));
}

void IniSetting::SectionParserCallback::onEntry(
    const std::string &key, const std::string &value, void *arg) {
  IniSetting::ParserCallback::onEntry(key, value,
                                      activeArray((CallbackData*)arg));
}

void IniSetting::SectionParserCallback::onPopEntry(
    const std::string &key, const std::string &value, const std::string &offset,
    void *arg) {
  IniSetting::ParserCallback::onPopEntry(key, value, offset,
                                         activeArray((CallbackData*)arg));
}

void IniSetting::SystemParserCallback::onEntry(
    const std::string &key, const std::string &value, void *arg) {
  assertx(!key.empty());
  // onConstant will always be called before onEntry, so we can check
  // here
  if (IniSetting::s_config_is_a_constant) {
    IniSetting::config_names_that_use_constants.insert(key);
    IniSetting::s_config_is_a_constant = false;
  }
  ParserCallback::onEntry(key, value, arg);

}

void IniSetting::SystemParserCallback::onPopEntry(const std::string& key,
                                                  const std::string& value,
                                                  const std::string& offset,
                                                  void* arg) {
  assertx(!key.empty());
  if (IniSetting::s_config_is_a_constant) {
    IniSetting::config_names_that_use_constants.insert(key);
    IniSetting::s_config_is_a_constant = false;
  }
  ParserCallback::onPopEntry(key, value, offset, arg);
}

void IniSetting::SystemParserCallback::onConstant(std::string &result,
                                                  const std::string &name) {
  IniSetting::s_config_is_a_constant = true;
  if (HHVM_FN(defined)(name, false)) {
    result = HHVM_FN(constant)(name).toString().toCppString();
  } else {
    result = name;
  }
}

///////////////////////////////////////////////////////////////////////////////

static Mutex s_mutex;
Variant IniSetting::FromString(const String& ini, const String& filename,
                               bool process_sections /* = false */,
                               int scanner_mode /* = NormalScanner */) {
  Lock lock(s_mutex); // ini parser is not thread-safe
  // We are parsing something new, so reset this flag
  s_config_is_a_constant = false;
  auto ini_cpp = ini.toCppString();
  auto filename_cpp = filename.toCppString();
  Variant ret = false;
  if (process_sections) {
    CallbackData data;
    SectionParserCallback cb(ParserCallbackMode::DARRAY);
    data.arr = Array::CreateDict();
    if (zend_parse_ini_string(ini_cpp, filename_cpp, scanner_mode, cb, &data)) {
      if (!data.active_name.isNull()) {
        data.arr.asArrRef().set(data.active_name, data.active_section);
      }
      ret = data.arr;
    }
  } else {
    ParserCallback cb(ParserCallbackMode::DARRAY);
    Variant arr = Array::CreateDict();
    if (zend_parse_ini_string(ini_cpp, filename_cpp, scanner_mode, cb, &arr)) {
      ret = arr;
    }
  }
  return ret;
}

IniSettingMap IniSetting::FromStringAsMap(const std::string& ini,
                                          const std::string& filename) {
  Lock lock(s_mutex); // ini parser is not thread-safe
  // We are parsing something new, so reset this flag
  s_config_is_a_constant = false;
  SystemParserCallback cb(ParserCallbackMode::DICT);
  Variant parsed;
  zend_parse_ini_string(ini, filename, NormalScanner, cb, &parsed);
  if (parsed.isNull()) {
    return uninit_null();
  }
  return parsed;
}

namespace {

Variant doGet(IniSetting::OptionData& data) {
#define F(Ty) \
  [] (IniSetting::SetAndGetImpl<Ty>& data) -> Variant { \
    Ty v;                                               \
    if (data.getter) v = data.getter();                 \
    else if (data.val) v = *data.val;                   \
    return ini_get(v);                                  \
  },
  return folly::variant_match<Variant>(
    data,
    INI_TYPES(F)
    [] (std::nullptr_t) { always_assert(false); return Variant(); }
  );
#undef F
}

bool doSet(IniSetting::OptionData& data, const Variant& value) {
#define F(Ty) \
  [&] (IniSetting::SetAndGetImpl<Ty>& data) -> bool { \
    Ty v;                                             \
    if (!ini_on_update(value, v)) return false;       \
    if (data.setter && !data.setter(v)) return false; \
    if (data.val) *data.val = v;                      \
    return true;                                      \
  },
  return folly::variant_match<bool>(
    data,
    INI_TYPES(F)
    [] (std::nullptr_t) { always_assert(false); return false; }
  );
#undef F
}

void doLog(IniSetting::OptionData& data, const char* name,
           StructuredLogEntry& ent) {
#define F(Ty) \
  [&] (IniSetting::SetAndGetImpl<Ty>& data) { \
    Ty v;                                     \
    if (data.getter) v = data.getter();       \
    else if (data.val) v = *data.val;         \
    ini_log(v, name, ent);                    \
  },
  folly::variant_match<void>(
    data,
    INI_TYPES(F)
    [] (std::nullptr_t) {}
  );
#undef F
}

folly::dynamic doDyn(IniSetting::OptionData& data) {
#define F(Ty) \
  [] (IniSetting::SetAndGetImpl<Ty>& data) { \
    Ty v;                                    \
    if (data.getter) v = data.getter();      \
    else if (data.val) v = *data.val;        \
    return ini_dyn(v);                       \
  },
  return folly::variant_match<folly::dynamic>(
    data,
    INI_TYPES(F)
    [] (std::nullptr_t) { always_assert(false); return folly::dynamic(); }
  );
#undef F
}

size_t doHash(IniSetting::OptionData& data) {
#define F(Ty) \
  [] (IniSetting::SetAndGetImpl<Ty>& data) { \
    Ty v;                                    \
    if (data.getter) v = data.getter();      \
    else if (data.val) v = *data.val;        \
    return ini_hash(v);                      \
  },
  return folly::variant_match<size_t>(
    data,
    INI_TYPES(F)
    [] (std::nullptr_t) { always_assert(false); return 0; }
  );
#undef F
}

void logSettings() {
  if (RO::RepoAuthoritative) return;
  if (!StructuredLog::coinflip(RO::EvalStartOptionLogRate)) return;

  auto const hash = IniSetting::HashAll(RO::EvalStartOptionLogOptions,
                                        RO::EvalStartOptionLogExcludeOptions);
  if (!RO::EvalStartOptionLogCache.empty()) {
    auto name = RO::EvalStartOptionLogCache;
    replacePlaceholders(name, {{"%{hash}", folly::to<std::string>(hash)}});
    struct stat s;
    if (statSyscall(name.data(), &s) != 0) {
      if (time(nullptr) - s.st_mtim.tv_sec < RO::EvalStartOptionLogWindow) {
        return;
      }
    }
    auto f = open(name.data(), O_CREAT|O_TRUNC|O_WRONLY, S_IRUSR | S_IWUSR);
    if (f >= 0) {
      SCOPE_EXIT { close(f); };
      auto id = compilerId();
      folly::writeFull(f, id.data(), id.size());
    }
  }

  StructuredLogEntry ent;
  ent.force_init = true;
  ent.setStr("cmd_line", Process::GetAppName());
  ent.setInt("server_mode", RO::ServerExecutionMode() ? 1 : 0);
  ent.setInt("sample_rate", RO::EvalStartOptionLogRate);
  ent.setInt("hash", hash);
  ent.setProcessUuid("hhvm_uuid");
  IniSetting::Log(ent,
                  RO::EvalStartOptionLogOptions,
                  RO::EvalStartOptionLogExcludeOptions);
  StructuredLog::log("hhvm_runtime_options", ent);
}

InitFiniNode s_logSettings(logSettings, InitFiniNode::When::ProcessInit);

}

struct IniCallbackData {
  const Extension* extension{nullptr};
  IniSetting::Mode mode{IniSetting::Mode::Request};
  IniSetting::OptionData callbacks{nullptr};
};

using CallbackMap = folly::F14FastMap<
  String, IniCallbackData, hphp_string_hash, hphp_string_same>;

struct SystemSettings {
  std::unordered_map<std::string,Variant> settings;
  TYPE_SCAN_IGNORE_ALL; // the variants are always static
};

struct LocalSettings {
  // Using hash_map for reference stability
  using Map = req::hash_map<std::string,Variant>;
  req::Optional<Map> settings;
  Map& init() {
    if (!settings) settings.emplace();
    return settings.value();
  }
  void clear() { settings.reset(); }
  bool empty() { return !settings.has_value() || settings.value().empty(); }
};

namespace {

//
// These are for settings/callbacks only settable at startup.
//
// Empirically and surprisingly (20Jan2015):
//   * server mode: the contents of system map are     destructed on SIGTERM
//   * CLI    mode: the contents of system map are NOT destructed on SIGTERM
//
CallbackMap s_system_ini_callbacks;

//
// These are for settings/callbacks that the script
// can change during the request.
//
// Empirically and surprisingly (20Jan2015), when there are N threads:
//   * server mode: the contents of user map are     destructed N-1 times
//   * CLI    mode: the contents of user map are NOT destructed on SIGTERM
//
RDS_LOCAL(CallbackMap, s_user_callbacks);

// Set by a .ini file at the start
SystemSettings s_system_settings;

// Changed during the course of the request
RDS_LOCAL(LocalSettings, s_saved_defaults);

struct IniSettingExtension final : Extension {
  IniSettingExtension() : Extension("hhvm.ini", NO_EXTENSION_VERSION_YET, NO_ONCALL_YET) {}

  // s_saved_defaults should be clear at the beginning of any request
  void requestInit() override {
    assertx(!s_saved_defaults->settings.has_value());
  }
} s_ini_extension;

}

void IniSetting::Bind(
  const Extension* extension,
  const Mode mode,
  const std::string& name,
  OptionData callbacks,
  const char* defaultValue
) {
  assertx(!name.empty());

  bool is_thread_local = canSet(mode, Mode::Request);
  assertx(is_thread_local || !ExtensionRegistry::modulesInitialised() ||
         !s_system_settings_are_set);

  // When the debugger is loading its configuration, there will be some
  // cases where Extension::ModulesInitialised(), but the name appears
  // in neither s_user_callbacks nor s_system_ini_callbacks. The bottom
  // line is that we can't really use ModulesInitialised() to help steer
  // the choices here.
  auto const staticName = String::attach(makeStaticString(name));
  assertx(IMPLIES(!is_thread_local, !s_user_callbacks->count(staticName)));

  // For now, we require the extensions to use their own thread local
  // memory for user-changeable settings. This means you need to use
  // the default field to Bind and can't statically initialize them.
  // The main reasoning to do that is so that the extensions have the
  // values already parsed into their types. If you are setting an int,
  // it does the string parsing once and then when you read it, it is
  // already an int. If we did some shared thing, we would just hand you
  // back the strings and you'd have to parse them on every request or
  // build some convoluted caching mechanism which is slower than just
  // the int access.
  //
  // We could conceivably let you use static memory and have our own
  // thread local here that users can change and then reset it back to
  // the default, but we haven't built that yet.
  IniCallbackData &data = is_thread_local ? (*s_user_callbacks)[staticName]
                                          : s_system_ini_callbacks[staticName];

  data.extension = extension;
  data.mode = mode;
  data.callbacks = callbacks;

  auto hasSystemDefault = ResetSystemDefault(name);
  if (!hasSystemDefault && defaultValue) {
    doSet(callbacks, defaultValue);
  }
}

void IniSetting::Unbind(const std::string& name) {
  assertx(!name.empty());
  s_user_callbacks->erase(name);
}

static IniCallbackData* get_callback(const String& name) {
  auto iter = s_system_ini_callbacks.find(name);
  if (iter == s_system_ini_callbacks.end()) {
    iter = s_user_callbacks->find(name);
    if (iter == s_user_callbacks->end()) {
      return nullptr;
    }
  }
  return &iter->second;
}

bool IniSetting::Get(const std::string& name, std::string &value) {
  Variant b;
  auto ret = Get(name, b);
  value = b.toString().toCppString();
  return ret && !value.empty();
}

bool IniSetting::Get(const String& name, std::string &value) {
  Variant b;
  auto ret = Get(name, b);
  value = b.toString().toCppString();
  return ret && !value.empty();
}

bool IniSetting::Get(const String& name, String& value) {
  Variant b;
  auto ret = Get(name, b);
  value = b.toString();
  return ret;
}

static bool shouldHideSetting(const String& name) {
  for (auto& sub : RuntimeOption::EvalIniGetHide) {
    if (name.find(sub) != -1) {
      return true;
    }
  }
  return false;
}

bool IniSetting::Get(const String& name, Variant& value) {
  if (shouldHideSetting(name)) {
    return false;
  }
  auto cb = get_callback(name);
  if (!cb) {
    return false;
  }
  value = doGet(cb->callbacks);
  return true;
}

std::string IniSetting::Get(const std::string& name) {
  std::string ret;
  Get(name, ret);
  return ret;
}

std::string IniSetting::Get(const String& name) {
  std::string ret;
  Get(name, ret);
  return ret;
}

static bool ini_set(const String& name, const Variant& value,
                    IniSetting::Mode mode) {
  auto cb = get_callback(name);
  if (!cb || !IniSetting::canSet(cb->mode, mode)) {
    return false;
  }
  return doSet(cb->callbacks, value);
}

bool IniSetting::FillInConstant(const std::string& name,
                                const Variant& value) {

  if (config_names_that_use_constants.find(name) ==
      config_names_that_use_constants.end()) {
    return false;
  }
  // We can cheat here since we fill in constants a while after
  // runtime options are loaded.
  s_system_settings_are_set = false;
  auto const ret = IniSetting::SetSystem(name, value);
  s_system_settings_are_set = true;
  return ret;
}

bool IniSetting::SetSystem(const String& name, const Variant& value) {
  // Shouldn't be calling this function after the runtime options are loaded.
  assertx(!s_system_settings_are_set);
  // Since we're going to keep these settings for the lifetime of the program,
  // we need to make them static.
  Variant eval_scalar_variant = value;
  eval_scalar_variant.setEvalScalar();
  s_system_settings.settings[name.toCppString()] = eval_scalar_variant;
  return ini_set(name, value, Mode::Config);
}

bool IniSetting::GetSystem(const String& name, Variant& value) {
  auto it = s_system_settings.settings.find(name.toCppString());
  if (it == s_system_settings.settings.end()) {
    return false;
  }
  value = it->second;
  return true;
}

bool IniSetting::SetUser(const String& name, const Variant& value) {
  auto& defaults = s_saved_defaults->init();
  if (!defaults.count(name.toCppString())) {
    Variant def;
    auto success = Get(name, def); // def gets populated here
    if (success) {
      defaults[name.toCppString()] = def;
    }
  }
  return ini_set(name, value, Mode::Request);
}

void IniSetting::RestoreUser(const String& name) {
  if (!s_saved_defaults->empty()) {
    auto& defaults = s_saved_defaults->settings.value();
    auto it = defaults.find(name.toCppString());
    if (it != defaults.end() &&
        ini_set(name, it->second, Mode::Request)) {
      defaults.erase(it);
    }
  }
};

bool IniSetting::ResetSystemDefault(const std::string& name) {
  auto it = s_system_settings.settings.find(name);
  if (it == s_system_settings.settings.end()) {
    return false;
  }
  return ini_set(name, it->second, Mode::Config);
}

void IniSetting::ResetSavedDefaults() {
  if (!s_saved_defaults->empty()) {
    for (auto& item : s_saved_defaults->settings.value()) {
      ini_set(item.first, item.second, Mode::Request);
    }
  }
  // destroy the local settings hashtable even if it's empty
  s_saved_defaults->clear();
}

bool IniSetting::canSet(Mode settingMode, Mode checkMode) {
  assertx(checkMode != Mode::Constant);
  if (settingMode == Mode::Constant) return false;
  return settingMode <= checkMode;
}

Optional<IniSetting::Mode> IniSetting::GetMode(const String& name) {
  auto cb = get_callback(name);
  if (!cb) {
    return {};
  }
  return {cb->mode};
}

void IniSetting::Log(StructuredLogEntry& ent,
                     const hphp_fast_string_set& toLog,
                     const hphp_fast_string_set& toExclude) {
  for (auto& iter: boost::join(s_system_ini_callbacks, *s_user_callbacks)) {
    if (shouldHideSetting(iter.first)) continue;
    if (!toLog.empty() && !toLog.count(iter.first.toCppString())) continue;
    if (toExclude.count(iter.first.toCppString())) continue;

    doLog(iter.second.callbacks, iter.first.data(), ent);
  }
}

folly::dynamic IniSetting::GetAllAsDynamic() {
  folly::dynamic obj = folly::dynamic::object;
  for (auto& iter: boost::join(s_system_ini_callbacks, *s_user_callbacks)) {
    if (shouldHideSetting(iter.first)) continue;

    obj[iter.first.toCppString()] = doDyn(iter.second.callbacks);
  }
  return obj;
}

size_t IniSetting::HashAll(const hphp_fast_string_set& toLog,
                           const hphp_fast_string_set& toExclude) {
  size_t hash = 0;
  for (auto& iter: boost::join(s_system_ini_callbacks, *s_user_callbacks)) {
    if (!toLog.empty() && !toLog.count(iter.first.toCppString())) continue;
    if (toExclude.count(iter.first.toCppString())) continue;
    if (shouldHideSetting(iter.first)) continue;

    hash = folly::hash::commutative_hash_combine_value_generic(
      hash,
      std::hash<size_t>{},
      folly::hash::hash_combine(
        iter.first.toCppString(), doHash(iter.second.callbacks)
      )
    );
  }
  return hash;
}

Array IniSetting::GetAll(const String& ext_name, bool details) {
  Array r = Array::CreateDict();

  const Extension* ext = nullptr;
  if (!ext_name.empty()) {
    if (ext_name == s_core) {
      ext = IniSetting::CORE;
    } else {
      ext = ExtensionRegistry::get(ext_name);
      if (!ext) {
        raise_warning("Unable to find extension '%s'",
                      ext_name.toCppString().c_str());
        return r;
      }
    }
  }

  for (auto& iter: boost::join(s_system_ini_callbacks, *s_user_callbacks)) {
    if (ext && ext != iter.second.extension) {
      continue;
    }
    if (shouldHideSetting(iter.first)) {
      continue;
    }

    auto value = doGet(iter.second.callbacks);
    // Cast all non-arrays to strings since that is what everything used to be
    if (!value.isArray()) {
      value = value.toString();
    }
    if (details) {
      Array item = Array::CreateDict();
      item.set(s_global_value, value);
      item.set(s_local_value, value);
      item.set(s_access, Variant(int64_t(iter.second.mode)));
      r.set(String(iter.first), item);
    } else {
      r.set(String(iter.first), value);
    }
  }
  return r;
}

std::string IniSetting::GetAllAsJSON() {
  Array settings = GetAll(empty_string(), true);
  auto const opts = k_JSON_FB_FORCE_HACK_ARRAYS;
  String out = Variant::attach(HHVM_FN(json_encode)(settings, opts)).toString();
  return std::string(out.c_str());
}

void add_default_config_files_globbed(
  const char *default_config_file,
  std::function<void (const char *filename)> cb
) {
  glob_t globbuf;
  memset(&globbuf, 0, sizeof(glob_t));
  int flags = 0;  // Use default glob semantics
  int nret = glob(default_config_file, flags, nullptr, &globbuf);
  if (nret == GLOB_NOMATCH ||
      globbuf.gl_pathc == 0 ||
      globbuf.gl_pathv == 0 ||
      nret != 0) {
    globfree(&globbuf);
    return;
  }

  for (int n = 0; n < (int)globbuf.gl_pathc; n++) {
    if (access(globbuf.gl_pathv[n], R_OK) != -1) {
      cb(globbuf.gl_pathv[n]);
    }
  }
  globfree(&globbuf);
}

///////////////////////////////////////////////////////////////////////////////
}
