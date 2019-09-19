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
#include "hphp/runtime/base/builtin-functions.h"
#include "hphp/runtime/base/execution-context.h"
#include "hphp/runtime/base/req-optional.h"
#include "hphp/runtime/base/runtime-option.h"

#include "hphp/runtime/base/ini-parser/zend-ini.h"

#include "hphp/runtime/ext/std/ext_std_misc.h"
#include "hphp/runtime/ext/extension.h"
#include "hphp/runtime/ext/extension-registry.h"
#include "hphp/runtime/ext/json/ext_json.h"

#include "hphp/util/lock.h"
#include "hphp/util/portability.h"
#include "hphp/util/logger.h"

#include "hphp/zend/zend-strtod.h"

#ifndef _MSC_VER
#include <glob.h>
#endif

#define __STDC_LIMIT_MACROS
#include <cstdint>
#include <boost/range/join.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string.hpp>
#include <map>

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

bool ini_on_update(const Variant& value, double& p) {
  INI_ASSERT_STR(value);
  p = zend_strtod(str.data(), nullptr);
  return true;
}

bool ini_on_update(const Variant& value, char& p) {
  INI_ASSERT_STR(value);
  auto n = convert_bytes_to_long(str);
  auto maxValue = 0x7FL;
  if (n > maxValue || n < (- maxValue - 1)) {
    return false;
  }
  p = n;
  return true;
}

bool ini_on_update(const Variant& value, int16_t& p) {
  INI_ASSERT_STR(value);
  auto n = convert_bytes_to_long(str);
  auto maxValue = 0x7FFFL;
  if (n > maxValue || n < (- maxValue - 1)) {
    return false;
  }
  p = n;
  return true;
}

bool ini_on_update(const Variant& value, int32_t& p) {
  INI_ASSERT_STR(value);
  auto n = convert_bytes_to_long(str);
  auto maxValue = 0x7FFFFFFFL;
  if (n > maxValue || n < (- maxValue - 1)) {
    return false;
  }
  p = n;
  return true;
}

bool ini_on_update(const Variant& value, int64_t& p) {
  INI_ASSERT_STR(value);
  p = convert_bytes_to_long(str);
  return true;
}

bool ini_on_update(const Variant& value, unsigned char& p) {
  INI_ASSERT_STR(value);
  auto n = convert_bytes_to_long(str);
  auto mask = ~0xFFUL;
  if (((uint64_t)n & mask)) {
    return false;
  }
  p = n;
  return true;
}

bool ini_on_update(const Variant& value, uint16_t& p) {
  INI_ASSERT_STR(value);
  auto n = convert_bytes_to_long(str);
  auto mask = ~0xFFFFUL;
  if (((uint64_t)n & mask)) {
    return false;
  }
  p = n;
  return true;
}

bool ini_on_update(const Variant& value, uint32_t& p) {
  INI_ASSERT_STR(value);
  auto n = convert_bytes_to_long(str);
  auto mask = ~0x7FFFFFFFUL;
  if (((uint64_t)n & mask)) {
    return false;
  }
  p = n;
  return true;
}

bool ini_on_update(const Variant& value, uint64_t& p) {
  INI_ASSERT_STR(value);
  p = convert_bytes_to_long(str);
  return true;
}

bool ini_on_update(const Variant& value, std::string& p) {
  INI_ASSERT_STR(value);
  p = str;
  return true;
}

bool ini_on_update(const Variant& value, String& p) {
  INI_ASSERT_STR(value);
  p = str.data();
  return true;
}

bool ini_on_update(const Variant& value, Array& p) {
  INI_ASSERT_ARR(value);
  p = value.toArray();
  return true;
}

bool ini_on_update(const Variant& value, std::set<std::string>& p) {
  INI_ASSERT_ARR_INNER(value, String);
  for (ArrayIter iter(value.toArray()); iter; ++iter) {
    p.insert(iter.second().toString().toCppString());
  }
  return true;
}

bool ini_on_update(const Variant& value,
                   std::set<std::string, stdltistr>& p) {
  INI_ASSERT_ARR_INNER(value, String);
  for (ArrayIter iter(value.toArray()); iter; ++iter) {
    p.insert(iter.second().toString().toCppString());
  }
  return true;
}

bool ini_on_update(const Variant& value,
                   boost::container::flat_set<std::string>& p) {
  INI_ASSERT_ARR_INNER(value, String);
  for (ArrayIter iter(value.toArray()); iter; ++iter) {
    p.insert(iter.second().toString().toCppString());
  }
  return true;
}

bool ini_on_update(const Variant& value, std::vector<uint32_t>& p) {
  INI_ASSERT_ARR_INNER(value, Integer);
  for (ArrayIter iter(value.toArray()); iter; ++iter) {
    p.push_back(iter.second().toInt64());
  }
  return true;
}

bool ini_on_update(const Variant& value, std::vector<std::string>& p) {
  INI_ASSERT_ARR_INNER(value, String);
  for (ArrayIter iter(value.toArray()); iter; ++iter) {
    p.push_back(iter.second().toString().toCppString());
  }
  return true;
}

bool ini_on_update(const Variant& value,
                   std::unordered_map<std::string, int>& p) {
  INI_ASSERT_ARR_INNER(value, Integer);
  for (ArrayIter iter(value.toArray()); iter; ++iter) {
    p[iter.first().toString().toCppString()] = iter.second().toInt64();
  }
  return true;
}

bool ini_on_update(const Variant& value,
                   std::map<std::string, std::string>& p) {
  INI_ASSERT_ARR_INNER(value, String);
  for (ArrayIter iter(value.toArray()); iter; ++iter) {
    p[iter.first().toString().toCppString()] =
      iter.second().toString().toCppString();
  }
  return true;
}

bool ini_on_update(const Variant& value,
                   std::map<std::string, std::string, stdltistr>& p) {
  INI_ASSERT_ARR_INNER(value, String);
  for (ArrayIter iter(value.toArray()); iter; ++iter) {
    p[iter.first().toString().toCppString()] =
      iter.second().toString().toCppString();
  }
  return true;
}

bool ini_on_update(const Variant& value,
                   hphp_string_imap<std::string>& p) {
  INI_ASSERT_ARR_INNER(value, String);
  for (ArrayIter iter(value.toArray()); iter; ++iter) {
    p[iter.first().toString().toCppString()] =
      iter.second().toString().toCppString();
  }
  return true;
}

Variant ini_get(bool& p) {
  return p ? "1" : "";
}

Variant ini_get(double& p) {
  return p;
}

Variant ini_get(char& p) {
  return p;
}

Variant ini_get(int16_t& p) {
  return p;
}

Variant ini_get(int32_t& p) {
  return p;
}

Variant ini_get(int64_t& p) {
  return p;
}

Variant ini_get(unsigned char& p) {
  return p;
}

Variant ini_get(uint16_t& p) {
  return p;
}

Variant ini_get(uint32_t& p) {
  return (uint64_t) p;
}

Variant ini_get(uint64_t& p) {
  return p;
}

Variant ini_get(std::string& p) {
  return p.data();
}

Variant ini_get(String& p) {
  return p.data();
}

namespace {

template<typename T>
void add(ArrayInit& arr, const std::string& key, const T& value) {
  auto keyStr = String(key);
  int64_t n;
  if (keyStr.get()->isStrictlyInteger(n)) {
    arr.add(n, value);
    return;
  }
  arr.add(keyStr.get(), value);
}

}

Variant ini_get(std::unordered_map<std::string, int>& p) {
  ArrayInit ret(p.size(), ArrayInit::Map{});
  for (auto& pair : p) {
    add(ret, pair.first, pair.second);
  }
  return ret.toArray();
}

Variant ini_get(std::map<std::string, std::string>& p) {
  ArrayInit ret(p.size(), ArrayInit::Map{});
  for (auto& pair : p) {
    add(ret, pair.first, pair.second);
  }
  return ret.toArray();
}

Variant ini_get(std::map<std::string, std::string, stdltistr>& p) {
  ArrayInit ret(p.size(), ArrayInit::Map{});
  for (auto& pair : p) {
    add(ret, pair.first, pair.second);
  }
  return ret.toArray();
}

Variant ini_get(hphp_string_imap<std::string>& p) {
  ArrayInit ret(p.size(), ArrayInit::Map{});
  for (auto& pair : p) {
    add(ret, pair.first, pair.second);
  }
  return ret.toArray();
}

Variant ini_get(Array& p) {
  return p;
}

Variant ini_get(std::set<std::string>& p) {
  ArrayInit ret(p.size(), ArrayInit::Map{});
  auto idx = 0;
  for (auto& s : p) {
    ret.add(idx++, s);
  }
  return ret.toArray();
}

Variant ini_get(std::set<std::string, stdltistr>& p) {
  ArrayInit ret(p.size(), ArrayInit::Map{});
  auto idx = 0;
  for (auto& s : p) {
    ret.add(idx++, s);
  }
  return ret.toArray();
}

Variant ini_get(boost::container::flat_set<std::string>& p) {
  ArrayInit ret(p.size(), ArrayInit::Map{});
  auto idx = 0;
  for (auto& s : p) {
    ret.add(idx++, s);
  }
  return ret.toArray();
}

template<typename T>
Variant ini_get(std::vector<T>& p) {
  ArrayInit ret(p.size(), ArrayInit::Map{});
  auto idx = 0;
  for (auto& s : p) {
    ret.add(idx++, s);
  }
  return ret.toArray();
}

template Variant ini_get<uint32_t>(std::vector<uint32_t>&);
template Variant ini_get<std::string>(std::vector<std::string>&);

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
  m_map = Variant(Array::Create());
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
  if (auto const intish = tryIntishCast<IntishCast::Cast>(key.get())) {
    return IniSettingMap(m_map.toCArrRef()[*intish]);
  }
  return IniSettingMap(m_map.toCArrRef()[key]);
}

IniSettingMap& IniSettingMap::operator=(const IniSettingMap& i) {
  m_map = i.m_map;
  return *this;
}

namespace {
void mergeSettings(tv_lval curval, TypedValue v) {
  auto const cur_inner = curval.unboxed();
  auto const cell = tvToCell(v);

  if (isArrayLikeType(cell.m_type) &&
      isArrayLikeType(cur_inner.type())) {
    for (auto i = ArrayIter(cell.m_data.parr); !i.end(); i.next()) {
      auto& cur_inner_ref = asArrRef(cur_inner);
      if (!cur_inner_ref.exists(i.first())) {
        cur_inner_ref.set(i.first(), empty_array());
      }
      mergeSettings(cur_inner_ref.lvalAt(i.first()), i.secondVal());
    }
  } else {
    tvSet(tvToInitCell(v), curval);
  }
}
}

void IniSettingMap::set(const String& key, const Variant& v) {
  assertx(this->isArray());
  auto& mapref = m_map.toArrRef();
  if (!mapref.exists(key)) {
    mapref.set(key, empty_array());
  }
  auto const curval = mapref.lvalAt(key);
  mergeSettings(curval, *v.asTypedValue());
}

///////////////////////////////////////////////////////////////////////////////
// callbacks for creating arrays out of ini

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
  forceToArray(*arr).set(skey, sval);
}

void IniSetting::ParserCallback::onPopEntry(
    const std::string &key,
    const std::string &value,
    const std::string &offset,
    void *arg) {
  auto arr = static_cast<Variant*>(arg);
  forceToArray(*arr);

  String skey(key);
  auto& arr_ref = arr->toArrRef();
  if (!arr_ref.exists(skey)) {
    arr_ref.set(skey, empty_array());
  }
  auto const hash = arr_ref.lvalAt(skey);
  forceToArray(hash);
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
    auto& arr = forceToArray(val);
    const auto key = arr.convertKey<IntishCast::Cast>(index);
    if (last) {
      String s{value};
      arr.set(key, make_tv<KindOfString>(s.get()));
      break;
    }
    // Similar to the above, in the case of a nested array we need to ensure
    // that we create empty arrays on the way down if they don't already exist.
    if (!arr.exists(key)) {
      arr.set(key, make_tv<KindOfArray>(ArrayData::Create()));
    }
    val = arr.lvalAt(key);
    p += index.size() + 1;
  }
}

void IniSetting::ParserCallback::onConstant(std::string &result,
                                            const std::string &name) {
  if (f_defined(name)) {
    result = f_constant(name).toString().toCppString();
  } else {
    result = name;
  }
}

void IniSetting::ParserCallback::onVar(std::string &result,
                                       const std::string& name) {
  std::string curval;
  if (IniSetting::Get(name, curval)) {
    result = curval;
    return;
  }
  String value = g_context->getenv(name);
  if (!value.isNull()) {
    result = value.toCppString();
    return;
  }
  result.clear();
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
    data->arr.toArrRef().set(data->active_name, data->active_section);
    data->active_section.unset();
  }
  data->active_section = Array::Create();
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
  if (f_defined(name, false)) {
    result = f_constant(name).toString().toCppString();
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
    SectionParserCallback cb;
    data.arr = Array::Create();
    if (zend_parse_ini_string(ini_cpp, filename_cpp, scanner_mode, cb, &data)) {
      if (!data.active_name.isNull()) {
        data.arr.toArrRef().set(data.active_name, data.active_section);
      }
      ret = data.arr;
    }
  } else {
    ParserCallback cb;
    Variant arr = Array::Create();
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
  SystemParserCallback cb;
  Variant parsed;
  zend_parse_ini_string(ini, filename, NormalScanner, cb, &parsed);
  if (parsed.isNull()) {
    return uninit_null();
  }
  // We have the final values for our ini settings.
  // Unbox everything so that we have no more references in the map since we do
  // things that might require us not to have references
  // (e.g. calling Variant::SetEvalScalar(), which will assert if an
  // arraydata's elements are KindOfRef)
  std::set<ArrayData*> seen;
  bool use_defaults = false;
  Variant ret = Unbox(parsed, seen, use_defaults, empty_string());
  if (use_defaults) {
    return uninit_null();
  }
  return ret;
}

Variant IniSetting::Unbox(const_variant_ref boxed, std::set<ArrayData*>& seen,
                          bool& use_defaults, const String& array_key) {
  assertx(boxed.isArray());
  Variant unboxed(Array::Create());
  auto ad = boxed.getArrayData();
  if (seen.insert(ad).second) {
    for (auto it = boxed.toArray().begin(); it; it.next()) {
      auto key = it.first();
      // asserting here to ensure that key is  a scalar type that can be
      // converted to a string.
      assertx(key.isScalar());
      auto elem = it.secondRef();
      unboxed.asArrRef().set(
        key,
        elem.isArray()
          ? *Unbox(elem, seen, use_defaults, key.toString()).asTypedValue()
          : *elem.rval()
      );
    }
    seen.erase(ad);
  } else {
    // The insert into seen wasn't successful. We have recursion.  break the
    // recursive cycle, so the elements can be freed by the MM. The
    // as_variant_ref() is ok because we fully own the array, with no sharing.

    // Use the current array key to give a little help in the log message
    boxed.as_variant_ref().unset();
    use_defaults = true;
    Logger::Warning("INI Recursion Detected at offset named %s. "
                    "Using default runtime settings.",
                    array_key.toCppString().c_str());
  }
  return unboxed;
}

struct IniCallbackData {
  IniCallbackData() {
    extension = nullptr;
    mode = IniSetting::PHP_INI_NONE;
    iniData = nullptr;
    updateCallback = nullptr;
    getCallback = nullptr;
  }
  virtual ~IniCallbackData() {
    delete iniData;
    iniData = nullptr;
  }
public:
  const Extension* extension;
  IniSetting::Mode mode;
  UserIniData *iniData;
  std::function<bool(const Variant& value)> updateCallback;
  std::function<Variant()> getCallback;
};

typedef std::map<std::string, IniCallbackData> CallbackMap;

//
// These are for settings/callbacks only settable at startup.
//
// Empirically and surprisingly (20Jan2015):
//   * server mode: the contents of system map are     destructed on SIGTERM
//   * CLI    mode: the contents of system map are NOT destructed on SIGTERM
//
static CallbackMap s_system_ini_callbacks;

//
// These are for settings/callbacks that the script
// can change during the request.
//
// Empirically and surprisingly (20Jan2015), when there are N threads:
//   * server mode: the contents of user map are     destructed N-1 times
//   * CLI    mode: the contents of user map are NOT destructed on SIGTERM
//
static RDS_LOCAL(CallbackMap, s_user_callbacks);

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
  void clear() { settings.clear(); }
  bool empty() { return !settings.hasValue() || settings.value().empty(); }
};

// Set by a .ini file at the start
static SystemSettings s_system_settings;

// Changed during the course of the request
static RDS_LOCAL(LocalSettings, s_saved_defaults);

struct IniSettingExtension final : Extension {
  IniSettingExtension() : Extension("hhvm.ini", NO_EXTENSION_VERSION_YET) {}

  // s_saved_defaults should be clear at the beginning of any request
  void requestInit() override {
    assertx(!s_saved_defaults->settings.hasValue());
  }
} s_ini_extension;

void IniSetting::Bind(
  const Extension* extension,
  const Mode mode,
  const std::string& name,
  std::function<bool(const Variant&)> updateCallback,
  std::function<Variant()> getCallback,
  std::function<struct UserIniData *(void)> userDataCallback
) {
  assertx(!name.empty());

  /*
   * WATCH OUT: unlike php5, a Mode is not necessarily a bit mask.
   * PHP_INI_ALL is NOT encoded as the union:
   *   PHP_INI_USER|PHP_INI_PERDIR|PHP_INI_SYSTEM
   *
   * Note that Mode value PHP_INI_SET_USER and PHP_INI_SET_EVERY are bit
   * sets; "SET" in this use means "bitset", and not "assignment".
   */
  bool is_thread_local;
  if (RuntimeOption::EnableZendIniCompat) {
    is_thread_local = (
    (mode == PHP_INI_USER) ||
    (mode == PHP_INI_PERDIR) ||
    (mode == PHP_INI_ALL) ||  /* See note above */
    (mode &  PHP_INI_USER) ||
    (mode &  PHP_INI_PERDIR) ||
    (mode &  PHP_INI_ALL)
    );
  } else {
    is_thread_local = (mode == PHP_INI_USER || mode == PHP_INI_ALL);
    assertx(is_thread_local || !ExtensionRegistry::modulesInitialised() ||
           !s_system_settings_are_set);
  }
  //
  // When the debugger is loading its configuration, there will be some
  // cases where Extension::ModulesInitialised(), but the name appears
  // in neither s_user_callbacks nor s_system_ini_callbacks. The bottom
  // line is that we can't really use ModulesInitialised() to help steer
  // the choices here.
  //

  bool use_user = is_thread_local;
  if (RuntimeOption::EnableZendIniCompat && !use_user) {
    //
    // If it is already in the user callbacks, continue to use it from
    // there. We don't expect it to be already there, but it has been
    // observed during development.
    //
    bool in_user_callbacks =
      (s_user_callbacks->find(name) != s_user_callbacks->end());
    assert (!in_user_callbacks);  // See note above
    use_user = in_user_callbacks;
  }

  //
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
  //

  IniCallbackData &data =
    use_user ? (*s_user_callbacks)[name] : s_system_ini_callbacks[name];

  data.extension = extension;
  data.mode = mode;
  data.updateCallback = updateCallback;
  data.getCallback = getCallback;
  if (data.iniData == nullptr && userDataCallback != nullptr) {
    data.iniData = userDataCallback();
  }
}

void IniSetting::Unbind(const std::string& name) {
  assertx(!name.empty());
  s_user_callbacks->erase(name);
}

static IniCallbackData* get_callback(const std::string& name) {
  CallbackMap::iterator iter = s_system_ini_callbacks.find(name);
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

bool IniSetting::Get(const String& name, String& value) {
  Variant b;
  auto ret = Get(name, b);
  value = b.toString();
  return ret;
}

static bool shouldHideSetting(const std::string& name) {
  for (auto& sub : RuntimeOption::EvalIniGetHide) {
    if (name.find(sub) != std::string::npos) {
      return true;
    }
  }
  return false;
}

bool IniSetting::Get(const String& name, Variant& value) {
  auto nameStr = name.toCppString();
  if (shouldHideSetting(nameStr)) {
    return false;
  }
  auto cb = get_callback(nameStr);
  if (!cb) {
    return false;
  }
  value = cb->getCallback();
  return true;
}

std::string IniSetting::Get(const std::string& name) {
  std::string ret;
  Get(name, ret);
  return ret;
}

static bool ini_set(const std::string& name, const Variant& value,
                    IniSetting::Mode mode) {
  auto cb = get_callback(name);
  if (!cb || !(cb->mode & mode)) {
    return false;
  }
  return cb->updateCallback(value);
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
  return ini_set(name.toCppString(), value, PHP_INI_SET_EVERY);
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
  return ini_set(name.toCppString(), value, PHP_INI_SET_USER);
}

void IniSetting::RestoreUser(const String& name) {
  if (!s_saved_defaults->empty()) {
    auto& defaults = s_saved_defaults->settings.value();
    auto it = defaults.find(name.toCppString());
    if (it != defaults.end() &&
        ini_set(name.toCppString(), it->second, PHP_INI_SET_USER)) {
      defaults.erase(it);
    }
  }
};

bool IniSetting::ResetSystemDefault(const std::string& name) {
  auto it = s_system_settings.settings.find(name);
  if (it == s_system_settings.settings.end()) {
    return false;
  }
  return ini_set(name, it->second, PHP_INI_SET_EVERY);
}

void IniSetting::ResetSavedDefaults() {
  if (!s_saved_defaults->empty()) {
    for (auto& item : s_saved_defaults->settings.value()) {
      ini_set(item.first, item.second, PHP_INI_SET_USER);
    }
  }
  // destroy the local settings hashtable even if it's empty
  s_saved_defaults->clear();
}

bool IniSetting::GetMode(const std::string& name, Mode& mode) {
  auto cb = get_callback(name);
  if (!cb) {
    return false;
  }
  mode = cb->mode;
  return true;
}

Array IniSetting::GetAll(const String& ext_name, bool details) {
  Array r = Array::Create();

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

    auto value = iter.second.getCallback();
    // Cast all non-arrays to strings since that is what everything used to be
    if (!value.isArray()) {
      value = value.toString();
    }
    if (details) {
      Array item = Array::Create();
      item.set(s_global_value, value);
      item.set(s_local_value, value);
      if (iter.second.mode == PHP_INI_ALL) {
        item.set(
          s_access,
          Variant(PHP_INI_USER | PHP_INI_SYSTEM | PHP_INI_PERDIR)
        );
      } else if (iter.second.mode == PHP_INI_ONLY) {
        item.set(s_access, Variant(PHP_INI_SYSTEM));
      } else {
        item.set(s_access, Variant(iter.second.mode));
      }
      r.set(String(iter.first), item);
    } else {
      r.set(String(iter.first), value);
    }
  }
  return r;
}

std::string IniSetting::GetAllAsJSON() {
  Array settings = GetAll(empty_string(), true);
  String out = Variant::attach(HHVM_FN(json_encode)(settings)).toString();
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
