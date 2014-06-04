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

#include "hphp/compiler/option.h"
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

#define CONFIG_BODY(T, METHOD) \
T Config::Get##METHOD(const IniSetting::Map &ini, const Hdf& config, \
                      const T defValue /* = 0ish */) { \
  auto* value = ini.get_ptr(IniName(config)); \
  if (value && value->isString()) { \
    T ret; \
    ini_on_update(value->data(), ret); \
    /* The HDF still wins because the -v options
     * are still done via HDF, for now */ \
    return config.configGet##METHOD(ret); \
  } \
  return config.configGet##METHOD(defValue); \
} \
void Config::Bind(T& loc, const IniSetting::Map &ini, const Hdf& config, \
                  const T defValue /* = 0ish */) { \
  loc = Get##METHOD(ini, config, defValue); \
  IniSetting::Bind(IniSetting::CORE, IniSetting::PHP_INI_SYSTEM, \
                   IniName(config), &loc); \
}

CONFIG_BODY(bool, Bool)
CONFIG_BODY(char, Byte)
CONFIG_BODY(unsigned char, UByte)
CONFIG_BODY(int16_t, Int16)
CONFIG_BODY(uint16_t, UInt16)
CONFIG_BODY(int32_t, Int32)
CONFIG_BODY(uint32_t, UInt32)
CONFIG_BODY(int64_t, Int64)
CONFIG_BODY(uint64_t, UInt64)
CONFIG_BODY(double, Double)
CONFIG_BODY(std::string, String)

static HackStrictOption GetHackStrictOption(const IniSettingMap& ini,
                                            const Hdf& config) {
  auto val = Config::GetString(ini, config);
  if (val.empty()) {
    if (Option::EnableHipHopSyntax || RuntimeOption::EnableHipHopSyntax) {
      return HackStrictOption::ERROR;
    }
    return HackStrictOption::OFF;
  }
  if (val == "warn") {
    return HackStrictOption::WARN;
  }
  bool ret;
  ini_on_update(val, ret);
  return ret ? HackStrictOption::ERROR : HackStrictOption::OFF;
}

void Config::Bind(HackStrictOption& loc, const IniSettingMap& ini,
                  const Hdf& config) {
  // Currently this doens't bind to ini_get since it is hard to thread through
  // an enum
  loc = GetHackStrictOption(ini, config);
}

}
