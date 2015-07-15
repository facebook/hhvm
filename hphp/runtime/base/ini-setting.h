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

#ifndef incl_HPHP_INI_SETTING_H_
#define incl_HPHP_INI_SETTING_H_

#include "hphp/runtime/base/type-variant.h"

#include <folly/dynamic.h>

#include <cstdint>
#include <functional>
#include <set>
#include <string>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

class Array;
class Extension;
class String;

bool ini_on_update(const folly::dynamic& value, bool& p);
bool ini_on_update(const folly::dynamic& value, double& p);
bool ini_on_update(const folly::dynamic& value, char& p);
bool ini_on_update(const folly::dynamic& value, int16_t& p);
bool ini_on_update(const folly::dynamic& value, int32_t& p);
bool ini_on_update(const folly::dynamic& value, int64_t& p);
bool ini_on_update(const folly::dynamic& value, unsigned char& p);
bool ini_on_update(const folly::dynamic& value, uint16_t& p);
bool ini_on_update(const folly::dynamic& value, uint32_t& p);
bool ini_on_update(const folly::dynamic& value, uint64_t& p);
bool ini_on_update(const folly::dynamic& value, std::string& p);
bool ini_on_update(const folly::dynamic& value, String& p);
bool ini_on_update(const folly::dynamic& value, Array& p);
bool ini_on_update(const folly::dynamic& value, std::set<std::string>& p);
bool ini_on_update(const folly::dynamic& value, std::vector<std::string>& p);
bool ini_on_update(const folly::dynamic& value,
                   std::map<std::string, std::string>& p);
bool ini_on_update(const folly::dynamic& value,
                   std::map<std::string, std::string,stdltistr>& p);
bool ini_on_update(const folly::dynamic& value,
                   std::set<std::string, stdltistr>& p);
bool ini_on_update(const folly::dynamic& value,
                   boost::container::flat_set<std::string>& p);
bool ini_on_update(const folly::dynamic& value,
                   hphp_string_imap<std::string>& p);
folly::dynamic ini_get(bool& p);
folly::dynamic ini_get(double& p);
folly::dynamic ini_get(char& p);
folly::dynamic ini_get(int16_t& p);
folly::dynamic ini_get(int32_t& p);
folly::dynamic ini_get(int64_t& p);
folly::dynamic ini_get(unsigned char& p);
folly::dynamic ini_get(uint16_t& p);
folly::dynamic ini_get(uint32_t& p);
folly::dynamic ini_get(uint64_t& p);
folly::dynamic ini_get(std::string& p);
folly::dynamic ini_get(String& p);
folly::dynamic ini_get(Array& p);
folly::dynamic ini_get(std::set<std::string>& p);
folly::dynamic ini_get(std::vector<std::string>& p);
folly::dynamic ini_get(std::map<std::string, std::string>& p);
folly::dynamic ini_get(std::map<std::string, std::string,stdltistr>& p);
folly::dynamic ini_get(std::set<std::string, stdltistr>& p);
folly::dynamic ini_get(boost::container::flat_set<std::string>& p);
folly::dynamic ini_get(hphp_string_imap<std::string>& p);

/**
 * If given an ini setting like "hhvm.abc[def][ghi]=yyy" and we have
 * an ini folly::dynamic with the top key being hhvm.abc pointing to its
 * values, we will have something like:
 *       {hhvm.abc {def {ghi : yyy}}}
 * And we pass as the name to get the value of as "def.ghi" for that
 * folly::dynamic, we need to iterate over the pointer for the dot (.) values to
 * get to the final setting value of yyy
 */
const folly::dynamic* ini_iterate(const folly::dynamic& ini,
                                  const std::string& name);

/*
 * Consult the private implementation details in ini-setting.cpp.
 *
 * There's one instance of an IniCallbackData for each initialization
 * setting. Management of system and per-request (per-thread)
 * mappings from names of ini settings to actual IniCallbackData
 * is done privately with the statics s_user_callbacks and
 * s_system_ini_callbacks.
 *
 * In addition, a unique instance of the class UserIniData can be
 * associated with the IniCallbackData. The IniCallbackData instance
 * is the point of ownership of the instance of UserIniData, and is
 * responsible for firing the destructor.
 *
 * The class UserIniData should be subclassed to hold data specific
 * to an initialization regime, such as the zend compatibility layer
 * implementation of zend_ini_entry. That subclass is responsible for
 * allocating/freeing its own internal data.
 *
 * There's a mechanism for registering an initter, which is a factory to
 * produce UserIniData. This registration is done at the same time that
 * the setter and getter are established; see the class SetAndGet
 */
class UserIniData {
public:
  virtual ~UserIniData() {}
};

class IniSetting {
  struct CallbackData {
    Variant active_section;
    Variant arr;
  };

public:
  static const Extension* CORE;
  enum ScannerMode {
    NormalScanner,
    RawScanner,
  };
  typedef folly::dynamic Map;

  class ParserCallback {
  public:
    virtual ~ParserCallback() {};
    virtual void onSection(const std::string &name, void *arg);
    virtual void onLabel(const std::string &name, void *arg);
    virtual void onEntry(const std::string &key, const std::string &value,
                         void *arg);
    virtual void onPopEntry(const std::string &key, const std::string &value,
                            const std::string &offset, void *arg);
    virtual void onConstant(std::string &result, const std::string &name);
    virtual void onVar(std::string &result, const std::string &name);
    virtual void onOp(std::string &result, char type, const std::string& op1,
                      const std::string& op2);
  private:
    void makeArray(Variant &hash, const std::string &offset,
                   const std::string &value);
  };
  class SectionParserCallback : public ParserCallback {
  public:
    virtual void onSection(const std::string &name, void *arg);
    virtual void onLabel(const std::string &name, void *arg);
    virtual void onEntry(const std::string &key, const std::string &value,
                         void *arg);
    virtual void onPopEntry(const std::string &key, const std::string &value,
                            const std::string &offset, void *arg);
  private:
    Variant* activeArray(CallbackData* data);
  };
  class SystemParserCallback : public ParserCallback {
  public:
    virtual void onSection(const std::string &name, void *arg);
    virtual void onLabel(const std::string &name, void *arg);
    virtual void onEntry(const std::string &key, const std::string &value,
                         void *arg);
    virtual void onPopEntry(const std::string &key, const std::string &value,
                            const std::string &offset, void *arg);
    virtual void onConstant(std::string &result, const std::string &name);
  private:
    void makeArray(Map &hash, const std::string &offset,
                   const std::string &value);
  };

  enum Mode {
    PHP_INI_NONE   = 0,
    // These 3 match zend
    PHP_INI_USER   = (1u << 0),
    PHP_INI_PERDIR = (1u << 1),
    PHP_INI_SYSTEM = (1u << 2),

    PHP_INI_ONLY   = (1u << 3),
    PHP_INI_ALL    = (1u << 4),

    PHP_INI_SET_USER   = PHP_INI_USER | PHP_INI_ALL,
    PHP_INI_SET_EVERY  = PHP_INI_ONLY | PHP_INI_SYSTEM | PHP_INI_PERDIR |
                         PHP_INI_SET_USER,
  };

public:
  static Variant FromString(const String& ini, const String& filename,
                            bool process_sections, int scanner_mode);
  static Map FromStringAsMap(const std::string& ini,
                             const std::string& filename);

  static bool Get(const std::string& name, folly::dynamic &value);
  static bool Get(const std::string& name, std::string &value);
  static bool Get(const String& name, Variant& value);
  static bool Get(const String& name, String& value);
  static std::string Get(const std::string& name);
  static Array GetAll(const String& extension, bool details);

  // Because folly::dynamic and Variant are too ambiguous
  enum class FollyDynamic {};
  /**
   * Change an INI setting as if it was in the php.ini file
   */
  static bool Set(const std::string& name, const folly::dynamic& value,
                  FollyDynamic);
  static bool Set(const String& name, const Variant& value);
  /**
   * Change an INI setting as if there was a call to ini_set()
   */
  static bool SetUser(const std::string& name, const folly::dynamic& value,
                      FollyDynamic);
  static bool SetUser(const String& name, const Variant& value);
  /**
   * Fill in constant that may not have been bound when an
   * ini file was initially parsed
   */
   static bool FillInConstant(const std::string& name,
                              const folly::dynamic& value,
                              FollyDynamic);
  /**
   * Get the mode for a setting
   */
  static bool GetMode(const std::string& name, Mode& mode);

  template<class T>
  struct SetAndGet {
    explicit SetAndGet(
      std::function<bool (const T&)> setter,
      std::function<T ()> getter,
      std::function<class UserIniData *()>initter = nullptr)
      : setter(setter),
        getter(getter),
        initter(initter) {}

    explicit SetAndGet() {}

    std::function<bool (const T&)> setter;
    std::function<T ()> getter;
    std::function<class UserIniData *()> initter;
  };

  /**
   * The heavy lifting of creating ini settings. First of all, if you don't
   * have to use this method, please don't. Instead use the simpler:
   *
   *   IniSetting::Bind(this, PHP_INI_SYSTEM, "my.ini", &some_global_var);
   *     or
   *   IniSetting::Bind(this, PHP_INI_ALL, "my.ini", &some_thread_local_var);
   *
   * If you have to do something special before your variable is set or gotten
   * then you can use this function to add callbacks. Both callbacks are
   * optional (but if you don't pass any, why are you using this method in the
   * first place?). If the setter callback returns "false" then the value will
   * not be saved into p. If the getter is not provided, the contents of p will
   * directly be used.
   */
  template<class T>
  static void Bind(const Extension* extension, const Mode mode,
                   const std::string& name, const char *defaultValue,
                   SetAndGet<T> callbacks, T* p = nullptr) {
    auto setter = [callbacks, p](const folly::dynamic &value) {
      T v;
      auto ret = ini_on_update(value, v);
      if (!ret) {
        return false;
      }
      if (callbacks.setter) {
        ret = callbacks.setter(v);
        if (!ret) {
          return false;
        }
      }
      if (p) {
        *p = v;
      }
      return true;
    };
    auto getter = [callbacks, p]() {
      T v;
      if (callbacks.getter) {
        v = callbacks.getter();
      } else if (p) {
        v = *p;
      }
      return ini_get(v);
    };
    auto initter = [callbacks, p]() {
      return callbacks.initter ? callbacks.initter() : nullptr;
    };
    Bind(extension, mode, name, setter, getter, initter);
    auto hasSystemDefault = ResetSystemDefault(name);
    if (!hasSystemDefault && defaultValue) {
      setter(defaultValue);
    }
  }

  template<class T>
  static void Bind(const Extension* extension, const Mode mode,
                   const std::string& name, SetAndGet<T> callbacks,
                   T* p = nullptr) {
    Bind(extension, mode, name, nullptr, callbacks, p);
  }

  /**
   * Prefer to use this method whenever possible (the non-default one is ok
   * too). Use Config::Bind if immediate access to the ini setting is
   * necessary. For performance reasons, Config::Bind should only be used
   * when access to the ini setting is needed prior to loading the rest of
   * the ini settings.
   */
  template<class T>
  static void Bind(const Extension* extension, const Mode mode,
                   const std::string& name, const char *defaultValue, T *p) {
    Bind(extension, mode, name, defaultValue, SetAndGet<T>(), p);
  }

  template<class T>
  static void Bind(const Extension* extension, const Mode mode,
                   const std::string& name, T *p) {
    Bind(extension, mode, name, nullptr, p);
  }

  static void Unbind(const std::string& name);

  /**
   * Set an ini setting back to the value from the config file
   * (or the hard-coded default)
   */
  static bool ResetSystemDefault(const std::string& name);

  // Used to allow you to Bind to PHP_INI_SYSTEM settings even after modules
  // have been initialized. This should only be used in rare cases that can't
  // be refactored into registration before extensions are done.
  static bool s_pretendExtensionsHaveNotBeenLoaded;

  // Used to help us late bind extension constants (e.g. E_ALL) that
  // were incorrectly bound initially, and needed to be bound again after
  // all was loaded.
  static bool s_config_is_a_constant;
  static std::set<std::string> config_names_that_use_constants;

private:
  static void Bind(
    const Extension* extension,
    const Mode mode,
    const std::string& name,
    std::function<bool(const folly::dynamic&)>updateCallback,
    std::function<folly::dynamic()> getCallback,
    std::function<UserIniData *(void)> userDataCallback = nullptr);
};

int64_t convert_bytes_to_long(const std::string& value);

void add_default_config_files_globbed(
  const char *default_config_file,
  std::function<void (const char *filename)> cb);

///////////////////////////////////////////////////////////////////////////////
}

#endif // incl_HPHP_INI_SETTING_H_
