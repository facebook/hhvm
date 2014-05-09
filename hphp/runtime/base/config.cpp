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
#include "hphp/runtime/base/ini-setting.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

/*
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
  return out;
}

static std::string ini_name(const Hdf& config) {
  return "hhvm" + normalize(config.getFullPath());
}
*/

const char *Config::Get(/* const IniSetting::Map &ini, */const Hdf& config,
                        const char *defValue /* = nullptr */) {
/*
  auto* value = ini.get_ptr(ini_name(config));
  if (value && value->isString()) {
    return value->data();
  }
  */
  return config.configGet(defValue);
}

/* The macro will soon contain
auto* value = ini.get_ptr(ini_name(config)); \
if (value && value->isString()) { \
  T ret; \
  ini_on_update(value->data(), ret); \
  return ret; \
} \
*/
#define GET_BODY(T, METHOD) \
T Config::Get##METHOD(/* const IniSetting::Map &ini, */ const Hdf& config, \
                      const T defValue) { \
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

#define GET_BYREF_BODY(T) \
void Config::Get(/* const IniSetting::Map &ini, */ const Hdf& config, \
                 T& values) { \
  config.configGet(values); \
}

#define COMMA ,
GET_BYREF_BODY(std::vector<std::string>)
GET_BYREF_BODY(std::set<std::string>)
GET_BYREF_BODY(std::set<std::string COMMA stdltistr>)
GET_BYREF_BODY(boost::container::flat_set<std::string>)
GET_BYREF_BODY(std::map<std::string COMMA std::string>)
GET_BYREF_BODY(hphp_string_imap<std::string>)

}
