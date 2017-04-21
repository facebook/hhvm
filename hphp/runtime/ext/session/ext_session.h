/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-present Facebook, Inc. (http://www.facebook.com)  |
   | Copyright (c) 1997-2010 The PHP Group                                |
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

#ifndef incl_HPHP_EXT_SESSION_H_
#define incl_HPHP_EXT_SESSION_H_

#include "hphp/runtime/ext/extension.h"
#include <vector>
#include "hphp/runtime/base/request-local.h"
#include "hphp/runtime/base/request-event-handler.h"

namespace HPHP {

///////////////////////////////////////////////////////////////////////////////
// SessionModule

/**
 * Session modules are implemeted by extending this class and
 * implementing its virtual methods, usually registering by
 * name as a by-product of the constructor.
 *
 * open() is called on session_init() (or request startup is autostart)
 * read()/write() should load and save serialized session data, respectively
 * destroy() should remove the session from the underlying storage media
 * gc() should look for an clean up expired sessions
 * close() is called on session_destroy() (or request end)
 */
struct SessionModule {
  enum {
    md5,
    sha1,
  };

  explicit SessionModule(const char *name) : m_name(name) {
    RegisteredModules.push_back(this);
  }
  virtual ~SessionModule() {}

  const char *getName() const { return m_name; }

  virtual bool open(const char *save_path, const char *session_name) = 0;
  virtual bool close() = 0;
  virtual bool read(const char *key, String &value) = 0;
  virtual bool write(const char *key, const String& value) = 0;
  virtual bool destroy(const char *key) = 0;
  virtual bool gc(int maxlifetime, int *nrdels) = 0;
  virtual String create_sid();

  static SessionModule *Find(const char *name) {
    for (unsigned int i = 0; i < RegisteredModules.size(); i++) {
      SessionModule *mod = RegisteredModules[i];
      if (mod && strcasecmp(name, mod->m_name) == 0) {
        return mod;
      }
    }
    return nullptr;
  }

private:
  static std::vector<SessionModule*> RegisteredModules;
  const char *m_name;
};

///////////////////////////////////////////////////////////////////////////////
// SystemlibSessionModule

struct SystemlibSessionInstance final : RequestEventHandler {
  SystemlibSessionInstance() { }

  const Object& getObject() { return m_obj; }
  void setObject(Object&& obj) { m_obj = std::move(obj); }
  void destroy() { m_obj.reset(); }
  void requestInit() override { m_obj.reset(); }
  void requestShutdown() override { m_obj.reset(); }

private:
  Object m_obj;
};

struct SystemlibSessionModule : SessionModule {
  SystemlibSessionModule(const char *mod_name, const char *phpclass_name) :
           SessionModule(mod_name),
           m_classname(phpclass_name) { }

  virtual bool open(const char *save_path, const char *session_name);
  virtual bool close();
  virtual bool read(const char *key, String &value);
  virtual bool write(const char *key, const String& value);
  virtual bool destroy(const char *key);
  virtual bool gc(int maxlifetime, int *nrdels);

private:
  void lookupClass();
  Func* lookupFunc(Class *cls, StringData *fname);
  const Object& getObject();

private:
  const char *m_classname;
  LowPtr<Class> m_cls;
  static LowPtr<Class> s_SHIClass;
  DECLARE_STATIC_REQUEST_LOCAL(SystemlibSessionInstance, s_obj);
  const Func* m_ctor;
  LowPtr<const Func> m_open, m_close;
  LowPtr<const Func> m_read, m_write;
  LowPtr<const Func> m_destroy, m_gc;
};

///////////////////////////////////////////////////////////////////////////////
}

#endif // incl_HPHP_EXT_SESSION_H_
