/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2014 Facebook, Inc. (http://www.facebook.com)     |
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

#include "hphp/runtime/base/config.h"

#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <fstream>

#include "hphp/runtime/base/ini-setting.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

static std::string normalize(const std::string &name) {
  std::string out = "";
  bool start = true;
  bool supress_next_underscore = false;
  for (auto &c : name) {
    if (start) {
      out += ".";
      out += tolower(c);
      start = false;
      supress_next_underscore = true;
    } else if (!isalpha(c)) {
      out += c;
      supress_next_underscore = true;
    } else if (isupper(c)) {
      if (!supress_next_underscore) {
        out += "_";
      }
      out += tolower(c);
      supress_next_underscore = true;
    } else {
      out += c;
      supress_next_underscore = false;
    }
  }
  boost::replace_first(out, ".eval.", ".");
  boost::replace_first(out, ".my_sql.", ".mysql.");
  boost::replace_first(out, ".enable_hip_hop_syntax", ".force_hh");
  return out;
}

std::string Config::IniName(const Hdf& config) {
  return "hhvm" + normalize(config.getFullPath());
}

void Config::Parse(const std::string &config, IniSetting::Map &ini, Hdf &hdf) {
  if (boost::ends_with(config, "ini")) {
    std::ifstream ifs(config);
    const std::string str((std::istreambuf_iterator<char>(ifs)),
                          std::istreambuf_iterator<char>());
    auto parsed_ini = IniSetting::FromStringAsMap(str, config);
    for (auto &pair : parsed_ini.items()) {
      ini[pair.first] = pair.second;
    }
  } else {
    hdf.append(config);
  }
}


const char* Config::Get(const IniSetting::Map &ini, const Hdf& config,
                         const char *defValue /* = nullptr */) {
  auto* value = ini.get_ptr(IniName(config));
  if (value && value->isString()) {
    return value->data();
  }
  return config.configGet(defValue);
}

// The HDF wins because the -v options are still done via HDF
#define GET_BODY(T, METHOD) \
T Config::Get##METHOD(const IniSetting::Map &ini, const Hdf& config, \
                      const T defValue) { \
  auto* value = ini.get_ptr(IniName(config)); \
  if (value && value->isString()) { \
    T ret; \
    ini_on_update(value->data(), ret); \
    return config.configGet##METHOD(ret); \
  } \
  return config.configGet##METHOD(defValue); \
}

GET_BODY(bool, Bool)
GET_BODY(std::string, String)
GET_BODY(char, Byte)
GET_BODY(unsigned char, UByte)
GET_BODY(int16_t, Int16)
GET_BODY(uint16_t, UInt16)
GET_BODY(int32_t, Int32)
GET_BODY(uint32_t, UInt32)
GET_BODY(int64_t, Int64)
GET_BODY(uint64_t, UInt64)
GET_BODY(double, Double)

HackStrictOption Config::GetHackStrictOption(const IniSettingMap& ini,
                                             const Hdf& config,
                                             const bool EnableHipHopSyntax) {
  auto val = Config::GetString(ini, config);
  if (val.empty()) {
    if (EnableHipHopSyntax) {
      return HackStrictOption::ERROR;
    }
    return HackStrictOption::OFF;
  }
  if (val == "off") {
    return HackStrictOption::OFF;
  }
  if (val == "warn") {
    return HackStrictOption::WARN;
  }
  if (val == "error") {
    return HackStrictOption::ERROR;
  }
  throw Exception("%s must be 'off', 'warn', 'error', or the empty "
                  "string - got '%s'",
                  config.getFullPath().c_str(),
                  val.c_str());
}

}
