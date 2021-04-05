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

#pragma once

#include "hphp/runtime/base/type-variant.h"

#include <folly/Range.h>

#include <cstdint>
#include <functional>
#include <set>
#include <string>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

struct Array;
struct Extension;
struct String;

bool ini_on_update(const Variant& value, bool& p);
bool ini_on_update(const Variant& value, double& p);
bool ini_on_update(const Variant& value, char& p);
bool ini_on_update(const Variant& value, int16_t& p);
bool ini_on_update(const Variant& value, int32_t& p);
bool ini_on_update(const Variant& value, int64_t& p);
bool ini_on_update(const Variant& value, unsigned char& p);
bool ini_on_update(const Variant& value, uint16_t& p);
bool ini_on_update(const Variant& value, uint32_t& p);
bool ini_on_update(const Variant& value, uint64_t& p);
bool ini_on_update(const Variant& value, std::string& p);
bool ini_on_update(const Variant& value, String& p);
bool ini_on_update(const Variant& value, Array& p);
bool ini_on_update(const Variant& value, std::set<std::string>& p);
bool ini_on_update(const Variant& value, std::vector<uint32_t>& p);
bool ini_on_update(const Variant& value, std::vector<std::string>& p);
bool ini_on_update(const Variant& value,
                   std::unordered_map<std::string, int>& p);
bool ini_on_update(const Variant& value,
                   std::map<std::string, std::string>& p);
bool ini_on_update(const Variant& value,
                   std::map<std::string, std::string, stdltistr>& p);
bool ini_on_update(const Variant& value,
                   std::set<std::string, stdltistr>& p);
bool ini_on_update(const Variant& value,
                   boost::container::flat_set<std::string>& p);
bool ini_on_update(const Variant& value,
                   hphp_string_imap<std::string>& p);
Variant ini_get(bool& p);
Variant ini_get(double& p);
Variant ini_get(char& p);
Variant ini_get(int16_t& p);
Variant ini_get(int32_t& p);
Variant ini_get(int64_t& p);
Variant ini_get(unsigned char& p);
Variant ini_get(uint16_t& p);
Variant ini_get(uint32_t& p);
Variant ini_get(uint64_t& p);
Variant ini_get(std::string& p);
Variant ini_get(String& p);
Variant ini_get(Array& p);
Variant ini_get(std::set<std::string>& p);
template<typename T>
Variant ini_get(std::vector<T>& p);
Variant ini_get(std::unordered_map<std::string, int>& p);
Variant ini_get(std::map<std::string, std::string>& p);
Variant ini_get(std::map<std::string, std::string, stdltistr>& p);
Variant ini_get(std::set<std::string, stdltistr>& p);
Variant ini_get(boost::container::flat_set<std::string>& p);
Variant ini_get(hphp_string_imap<std::string>& p);

/**
 * If given an ini setting like "hhvm.abc[def][ghi]=yyy" and we have
 * an ini Variant with the top key being hhvm.abc pointing to its
 * values, we will have something like:
 *       {hhvm.abc {def {ghi : yyy}}}
 * And we pass as the name to get the value of as "def.ghi" for that
 * Variant, we need to iterate over the pointer for the dot (.) values to
 * get to the final setting value of yyy
 */
const IniSettingMap ini_iterate(const IniSettingMap& ini,
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
 * In addition, a unique instance of the struct UserIniData can be
 * associated with the IniCallbackData. The IniCallbackData instance
 * is the point of ownership of the instance of UserIniData, and is
 * responsible for firing the destructor.
 *
 * The struct UserIniData should be subclassed to hold data specific
 * to an initialization regime, such as the zend compatibility layer
 * implementation of zend_ini_entry. That subclass is responsible for
 * allocating/freeing its own internal data.
 *
 * There's a mechanism for registering an initter, which is a factory to
 * produce UserIniData. This registration is done at the same time that
 * the setter and getter are established; see the class SetAndGet
 */
struct UserIniData {
  virtual ~UserIniData() {}
};

struct IniSettingMap {
  // placeholder to allow the form:
  //    IniSettingMap ini = IniSettingMap::object;
  // is used throughout the codebase. We can convert later to remove them
  enum Type { object };
  IniSettingMap();
  /* implicit */ IniSettingMap(Type);
  /* implicit */ IniSettingMap(const IniSettingMap& i); // copy constructor
  /* implicit */ IniSettingMap(const Variant& v);
  /* implicit */ IniSettingMap(IniSettingMap&& i) noexcept; // move constructor
  IniSettingMap& operator=(const IniSettingMap& i);

public:
  const IniSettingMap operator[](const String& key) const;
  String toString() const { return m_map.toString();}
  Array toArray() const { return m_map.toArray();}
  Object toObject() const { return m_map.toObject();}
  bool isNull() const { return m_map.isNull();}
  bool isString() const { return m_map.isString();}
  bool isArray() const { return m_map.isArray();}
  bool isObject() const { return m_map.isObject();}
  Variant& toVariant() { return m_map; }
  void set(const String& key, const Variant& v);
  TypedValue detach() noexcept {
    return m_map.detach();
  }
private:
  Variant m_map;
};

struct IniSetting {
private:

  struct CallbackData {
    String active_name;
    Variant active_section;
    Variant arr;
  };

public:
  // can remove later in a diff that explicitly changes all uses of
  // IniSetting::Map to IniSettingMap
  using Map = IniSettingMap;
  static const Extension* CORE;
  enum ScannerMode {
    NormalScanner,
    RawScanner,
  };

  enum struct ParserCallbackMode {
    DARRAY = 0,
    DICT = 1,
  };

  struct ParserCallback {
    explicit ParserCallback(ParserCallbackMode mode) : mode_(mode) {}
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
  protected:
    void makeArray(tv_lval hash, const std::string &offset,
                   const std::string &value);

    Array emptyArrayForMode() const;
    Array& forceToArrayForMode(Variant& var) const;
    Array& forceToArrayForMode(tv_lval var) const;

  private:
    // Substitution copy or symlink via @ or : markers in the config line
    void makeSettingSub(const String &key, const std::string &offset,
                        const std::string &value, Variant& cur_settings);
    void traverseToSet(const String &key, const std::string& offset,
                       tv_lval value, Variant& cur_settings,
                       const std::string& stopChar);

    ParserCallbackMode mode_;
  };
  struct SectionParserCallback : ParserCallback {
    using ParserCallback::ParserCallback;

    void onSection(const std::string& name, void* arg) override;
    void onLabel(const std::string& name, void* arg) override;
    void onEntry(const std::string& key, const std::string& value, void* arg)
        override;
    void onPopEntry(
        const std::string& key,
        const std::string& value,
        const std::string& offset,
        void* arg) override;

   private:
    Variant* activeArray(CallbackData* data);
  };
  struct SystemParserCallback : ParserCallback {
    using ParserCallback::ParserCallback;

    void onEntry(const std::string& key, const std::string& value, void* arg)
        override;
    void onPopEntry(
        const std::string& key,
        const std::string& value,
        const std::string& offset,
        void* arg) override;
    void onConstant(std::string& result, const std::string& name) override;
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
                            bool process_sections = false,
                            int scanner_mode = NormalScanner);
  static IniSettingMap FromStringAsMap(const std::string& ini,
                                       const std::string& filename);

  static bool Get(const std::string& name, std::string &value);
  static bool Get(const String& name, std::string &value);
  static bool Get(const String& name, Variant& value);
  static bool Get(const String& name, String& value);
  static std::string Get(const std::string& name);
  static std::string Get(const String& name);
  static Array GetAll(const String& extension, bool details);
  static std::string GetAllAsJSON();

  /**
   * Change an INI setting as if it was in the php.ini file
   */
  static bool SetSystem(const String& name, const Variant& value);
  /**
   * Get a system value
   */
  static bool GetSystem(const String& name, Variant& value);

  /**
   * Change an INI setting as if there was a call to ini_set()
   */
  static bool SetUser(const String& name, const Variant& value);

  /**
   * Restore an INI setting to the default value before the first call to
   * SetUser().
   */
  static void RestoreUser(const String& name);

  /**
   * Fill in constant that may not have been bound when an
   * ini file was initially parsed
   */
   static bool FillInConstant(const std::string& name,
                              const Variant& value);
  /**
   * Get the mode for a setting
   */
  static bool GetMode(const String& name, Mode& mode);

  template<class T>
  struct SetAndGet {
    explicit SetAndGet(
      std::function<bool (const T&)> setter,
      std::function<T ()> getter,
      std::function<struct UserIniData *()>initter = nullptr)
      : setter(setter),
        getter(getter),
        initter(initter) {}

    explicit SetAndGet() {}

    std::function<bool (const T&)> setter;
    std::function<T ()> getter;
    std::function<struct UserIniData *()> initter;
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
    auto callback_set = callbacks.setter;
    auto setter = [callback_set, p](const Variant &value) {
      T v;
      auto ret = ini_on_update(value, v);
      if (!ret) {
        return false;
      }
      if (callback_set) {
        ret = callback_set(v);
        if (!ret) {
          return false;
        }
      }
      if (p) {
        *p = v;
      }
      return true;
    };
    auto callback_get = callbacks.getter;
    auto getter = [callback_get, p]() {
      T v;
      if (callback_get) {
        v = callback_get();
      } else if (p) {
        v = *p;
      }
      return ini_get(v);
    };
    Bind(extension, mode, name, setter, getter, callbacks.initter);
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

  /**
   * After a request, we want to set all user settings back to their original
   * defaults before the request. This should be called on requestShutdown.
   */
  static void ResetSavedDefaults();

  /**
   *  Used to help us late bind extension constants (e.g. E_ALL) that
   *  were incorrectly bound initially, and needed to be bound again after
   *  all was loaded.
   */
  static bool s_config_is_a_constant;
  static std::set<std::string> config_names_that_use_constants;

  /**
   * A flag to ensure we don't try to add a system setting after the
   * runtime options have been loaded
   */
  static bool s_system_settings_are_set;

private:
  static void Bind(
    const Extension* extension,
    const Mode mode,
    const std::string& name,
    std::function<bool(const Variant&)>updateCallback,
    std::function<Variant()> getCallback,
    std::function<UserIniData *(void)> userDataCallback = nullptr);
};

int64_t convert_bytes_to_long(folly::StringPiece value);
std::string convert_long_to_bytes(int64_t value);

void add_default_config_files_globbed(
  const char *default_config_file,
  std::function<void (const char *filename)> cb);

///////////////////////////////////////////////////////////////////////////////
}

