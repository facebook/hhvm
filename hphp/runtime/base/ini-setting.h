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

#include "hphp/runtime/base/complex-types.h"
#include "folly/dynamic.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

class Extension;

#define ini_on_update_fail HPHP::IniSetting::UpdateCallback()
bool ini_on_update(const std::string& value, bool *p);
bool ini_on_update(const std::string& value, double *p);
bool ini_on_update(const std::string& value, int16_t *p);
bool ini_on_update(const std::string& value, int32_t *p);
bool ini_on_update(const std::string& value, int64_t *p);
bool ini_on_update(const std::string& value, uint16_t *p);
bool ini_on_update(const std::string& value, uint32_t *p);
bool ini_on_update(const std::string& value, uint64_t *p);
bool ini_on_update(const std::string& value, std::string *p);
bool ini_on_update(const std::string& value, String *p);
std::string ini_get(bool *p);
std::string ini_get(double *p);
std::string ini_get(int16_t *p);
std::string ini_get(int32_t *p);
std::string ini_get(int64_t *p);
std::string ini_get(uint16_t *p);
std::string ini_get(uint32_t *p);
std::string ini_get(uint64_t *p);
std::string ini_get(std::string *p);
std::string ini_get(String *p);

class IniSetting {
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
    struct CallbackData {
      Variant active_section;
      Variant arr;
    };
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
  };

  typedef std::function<bool(const std::string& value, void*p)> UpdateCallback;
  typedef std::function<std::string(void* p)> GetCallback;

public:
  static Variant FromString(const String& ini, const String& filename,
                            bool process_sections, int scanner_mode);
  static Map FromStringAsMap(const std::string& ini,
                             const std::string& filename);

  static bool Get(const std::string& name, std::string &value);
  static bool Get(const String& name, String &value);
  static std::string Get(const std::string& name);
  static Array GetAll(const String& extension, bool details);

  /**
   * Change an INI setting as if it was in the php.ini file
   */
  static bool Set(const String& name, const String& value);
  /**
   * Change an INI setting as if there was a call to ini_set()
   */
  static bool SetUser(const String& name, const String& value);

  static void Bind(const Extension* extension, const Mode mode,
                   const char *name, const char *value,
                   UpdateCallback updateCallback, GetCallback getCallback,
                   void *p = nullptr);
  static void Bind(const Extension* extension, const Mode mode,
                   const char *name,
                   UpdateCallback updateCallback, GetCallback getCallback,
                   void *p = nullptr);
  template<class T>
  static void Bind(const Extension* extension, const Mode mode,
                   const char *name, T *p) {
    Bind(extension, mode, name,
         [](const std::string& strval, void* val) {
           return ini_on_update(strval, static_cast<T*>(val));
         },
         [](void* val) {
           return ini_get(static_cast<T*>(val));
         },
         p);
  }
  template<class T>
  static void Bind(const Extension* extension, const Mode mode,
                   const char *name, const char *value, T *p) {
    Bind(extension, mode, name, value,
         [](const std::string &s, void *p) {
           return ini_on_update(s, (T*) p);
         },
         [](void *p) {
           return ini_get((T*) p);
         },
         p);
  }

  static void Unbind(const char *name);

  // Used to allow you to Bind to PHP_INI_SYSTEM settings even after modules
  // have been initialized. This should only be used in rare cases that can't
  // be refactored into registration before extensions are done.
  static bool s_pretendExtensionsHaveNotBeenLoaded;
};

int64_t convert_bytes_to_long(const std::string& value);

///////////////////////////////////////////////////////////////////////////////
}

#endif // incl_HPHP_INI_SETTING_H_
