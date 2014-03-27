/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2013 Facebook, Inc. (http://www.facebook.com)     |
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
#include "hphp/runtime/ext/session/ext_session.h"

#include <string>

#include <sys/types.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <dirent.h>
#include <vector>

#include "folly/String.h"

#include "hphp/util/lock.h"
#include "hphp/util/logger.h"
#include "hphp/util/compatibility.h"

#include "hphp/runtime/base/array-iterator.h"
#include "hphp/runtime/base/builtin-functions.h"
#include "hphp/runtime/base/datetime.h"
#include "hphp/runtime/base/ini-setting.h"
#include "hphp/runtime/base/object-data.h"
#include "hphp/runtime/base/request-local.h"
#include "hphp/runtime/base/string-buffer.h"
#include "hphp/runtime/base/variable-serializer.h"
#include "hphp/runtime/base/variable-unserializer.h"
#include "hphp/runtime/base/zend-math.h"
#include "hphp/runtime/ext/ext_function.h"
#include "hphp/runtime/ext/ext_hash.h"
#include "hphp/runtime/ext/std/ext_std_options.h"
#include "hphp/runtime/vm/jit/translator-inline.h"
#include "hphp/runtime/base/request-event-handler.h"

namespace HPHP {

///////////////////////////////////////////////////////////////////////////////

using std::string;

static bool ini_on_update_save_handler(const std::string& value);
static std::string ini_get_save_handler();
static bool ini_on_update_serializer(const std::string& value);
static std::string ini_get_serializer();
static bool ini_on_update_trans_sid(const bool& value);
static bool ini_on_update_save_dir(const std::string& value);
static bool mod_is_open();

///////////////////////////////////////////////////////////////////////////////
// global data

class SessionSerializer;
class Session {
public:
  enum Status {
    Disabled,
    None,
    Active
  };

  std::string m_save_path;
  std::string m_session_name;
  std::string m_extern_referer_chk;
  std::string m_entropy_file;
  int64_t     m_entropy_length;
  std::string m_cache_limiter;
  int64_t     m_cookie_lifetime;
  std::string m_cookie_path;
  std::string m_cookie_domain;
  bool        m_cookie_secure;
  bool        m_cookie_httponly;
  bool        m_mod_data = false;
  bool        m_mod_user_implemented = false;

  SessionModule *m_mod;
  SessionModule *m_default_mod;

  Status   m_session_status;
  int64_t  m_gc_probability;
  int64_t  m_gc_divisor;
  int64_t  m_gc_maxlifetime;
  int      m_module_number;
  int64_t  m_cache_expire;

  ObjectData *m_ps_session_handler;

  SessionSerializer *m_serializer;

  bool m_auto_start;
  bool m_use_cookies;
  bool m_use_only_cookies;
  bool m_use_trans_sid;   // contains the INI value of whether to use trans-sid
  bool m_apply_trans_sid; // whether to enable trans-sid for current request

  std::string m_hash_func;
  int64_t m_hash_bits_per_character;

  int  m_send_cookie;
  int  m_define_sid;
  bool m_invalid_session_id;  /* allows the driver to report about an invalid
                                 session id and request id regeneration */

  Session()
    : m_entropy_length(0), m_cookie_lifetime(0), m_cookie_secure(false),
      m_cookie_httponly(false), m_mod(nullptr), m_default_mod(nullptr),
      m_session_status(None), m_gc_probability(0), m_gc_divisor(0),
      m_gc_maxlifetime(0), m_module_number(0), m_cache_expire(0),
      m_ps_session_handler(nullptr),
      m_serializer(nullptr), m_auto_start(false), m_use_cookies(false),
      m_use_only_cookies(false), m_use_trans_sid(false),
      m_apply_trans_sid(false), m_hash_bits_per_character(0), m_send_cookie(0),
      m_define_sid(0), m_invalid_session_id(false) {
  }
};

const int64_t k_PHP_SESSION_DISABLED = Session::Disabled;
const int64_t k_PHP_SESSION_NONE     = Session::None;
const int64_t k_PHP_SESSION_ACTIVE   = Session::Active;
const StaticString s_session_ext_name("session");

struct SessionRequestData final : RequestEventHandler, Session {
  SessionRequestData() {}

  void destroy() {
    m_id.reset();
    m_session_status = Session::None;
    m_ps_session_handler = nullptr;
  }

  void requestInit() override {
    destroy();
  }

  void requestShutdown() override {
    // We don't actually want to do our requestShutdownImpl here---it
    // is run explicitly from the execution context, because it could
    // run user code.
  }

  void requestShutdownImpl();

public:
  String m_id;

};
IMPLEMENT_STATIC_REQUEST_LOCAL(SessionRequestData, s_session);
#define PS(name) s_session->m_ ## name

void SessionRequestData::requestShutdownImpl() {
  if (mod_is_open()) {
    try {
      m_mod->close();
    } catch (...) {}
  }
  if (ObjectData* obj = m_ps_session_handler) {
    m_ps_session_handler = nullptr;
    decRefObj(obj);
  }
  m_id.reset();
}

void ext_session_request_shutdown() {
  f_session_write_close();
  s_session->requestShutdownImpl();
}

std::vector<SessionModule*> SessionModule::RegisteredModules;

/*
 * Note that we cannot use the BASE64 alphabet here, because
 * it contains "/" and "+": both are unacceptable for simple inclusion
 * into URLs.
 */
static char hexconvtab[] =
  "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ,-";

/* returns a pointer to the byte after the last valid character in out */
static void bin_to_readable(const String& in, StringBuffer &out, char nbits) {
  unsigned char *p = (unsigned char *)in.data();
  unsigned char *q = (unsigned char *)in.data() + in.size();
  unsigned short w = 0;
  int have = 0;
  int mask = (1 << nbits) - 1;

  while (true) {
    if (have < nbits) {
      if (p < q) {
        w |= *p++ << have;
        have += 8;
      } else {
        /* consumed everything? */
        if (have == 0) break;
        /* No? We need a final round */
        have = nbits;
      }
    }

    /* consume nbits */
    out.append(hexconvtab[w & mask]);
    w >>= nbits;
    have -= nbits;
  }
}

const StaticString
  s_REMOTE_ADDR("REMOTE_ADDR"),
  s__SERVER("_SERVER"),
  s__SESSION("_SESSION"),
  s__COOKIE("_COOKIE"),
  s__GET("_GET"),
  s__POST("_POST");

String SessionModule::create_sid() {
  GlobalVariables *g = get_global_variables();
  String remote_addr = g->get(s__SERVER).toArray()[s_REMOTE_ADDR].toString();

  struct timeval tv;
  gettimeofday(&tv, NULL);

  StringBuffer buf;
  buf.printf("%.15s%ld%ld%0.8F", remote_addr.data(),
             tv.tv_sec, (long int)tv.tv_usec, math_combined_lcg() * 10);

  if (String(PS(hash_func)).isNumeric()) {
    switch (String(PS(hash_func)).toInt64()) {
    case md5:  PS(hash_func) = "md5";  break;
    case sha1: PS(hash_func) = "sha1"; break;
    }
  }

  Variant context = HHVM_FN(hash_init)(PS(hash_func));
  if (same(context, false)) {
    Logger::Error("Invalid session hash function: %s", PS(hash_func).c_str());
    return String();
  }
  if (!HHVM_FN(hash_update)(context.toResource(), buf.detach())) {
    Logger::Error("hash_update() failed");
    return String();
  }

  if (PS(entropy_length) > 0) {
    int fd = ::open(PS(entropy_file).c_str(), O_RDONLY);
    if (fd >= 0) {
      unsigned char rbuf[2048];
      int n;
      int to_read = PS(entropy_length);
      while (to_read > 0) {
        n = ::read(fd, rbuf, (to_read < (int)sizeof(rbuf) ?
                              to_read : (int)sizeof(buf)));
        if (n <= 0) break;
        if (!HHVM_FN(hash_update)(context.toResource(),
                           String((const char *)rbuf, n, CopyString))) {
          Logger::Error("hash_update() failed");
          ::close(fd);
          return String();
        }
        to_read -= n;
      }
      ::close(fd);
    }
  }

  String hashed = HHVM_FN(hash_final)(context.toResource());

  if (PS(hash_bits_per_character) < 4 || PS(hash_bits_per_character) > 6) {
    PS(hash_bits_per_character) = 4;
    raise_warning("The ini setting hash_bits_per_character is out of range "
                  "(should be 4, 5, or 6) - using 4 for now");
  }

  StringBuffer readable;
  bin_to_readable(hashed, readable, PS(hash_bits_per_character));
  return readable.detach();
}

///////////////////////////////////////////////////////////////////////////////
// SystemlibSessionModule

static StaticString s_SessionHandlerInterface("SessionHandlerInterface");

static StaticString s_open("open");
static StaticString s_close("close");
static StaticString s_read("read");
static StaticString s_write("write");
static StaticString s_gc("gc");
static StaticString s_destroy("destroy");

Class *SystemlibSessionModule::s_SHIClass = nullptr;

/**
 * Relies on the fact that only one SessionModule will be active
 * in a given thread at any one moment.
 */
IMPLEMENT_REQUEST_LOCAL(SystemlibSessionInstance, SystemlibSessionModule::s_obj);

Func* SystemlibSessionModule::lookupFunc(Class *cls, StringData *fname) {
  Func *f = cls->lookupMethod(fname);
  if (!f) {
    throw InvalidArgumentException(0, "class %s must declare method %s()",
                                   m_classname, fname->data());
  }

  if (f->attrs() & AttrStatic) {
    throw InvalidArgumentException(0, "%s::%s() must not be declared static",
                                   m_classname, fname->data());
  }

  if (f->attrs() & (AttrPrivate|AttrProtected|AttrAbstract)) {
    throw InvalidArgumentException(0, "%s::%s() must be declared public",
                                   m_classname, fname->data());
  }

  return f;
}

void SystemlibSessionModule::lookupClass() {
  Class *cls;
  if (!(cls = Unit::loadClass(String(m_classname, CopyString).get()))) {
    throw InvalidArgumentException(0, "Unable to locate systemlib class '%s'",
                                   m_classname);
  }

  if (cls->attrs() & (AttrTrait|AttrInterface)) {
    throw InvalidArgumentException(0, "'%s' must be a real class, "
                                      "not an interface or trait", m_classname);
  }

  if (!s_SHIClass) {
    s_SHIClass = Unit::lookupClass(s_SessionHandlerInterface.get());
    if (!s_SHIClass) {
      throw InvalidArgumentException(0, "Unable to locate '%s' interface",
                                        s_SessionHandlerInterface.data());
    }
  }

  if (!cls->classof(s_SHIClass)) {
    throw InvalidArgumentException(0, "SystemLib session module '%s' "
                                      "must implement '%s'",
                                      m_classname,
                                      s_SessionHandlerInterface.data());
  }

  if (LookupResult::MethodFoundWithThis !=
      g_context->lookupCtorMethod(m_ctor, cls)) {
    throw InvalidArgumentException(0, "Unable to call %s's constructor",
                                   m_classname);
  }

  m_open    = lookupFunc(cls, s_open.get());
  m_close   = lookupFunc(cls, s_close.get());
  m_read    = lookupFunc(cls, s_read.get());
  m_write   = lookupFunc(cls, s_write.get());
  m_gc      = lookupFunc(cls, s_gc.get());
  m_destroy = lookupFunc(cls, s_destroy.get());
  m_cls = cls;
}

ObjectData* SystemlibSessionModule::getObject() {
  if (auto o = s_obj->getObject()) {
    return o;
  }

  JIT::VMRegAnchor _;
  Variant ret;

  if (!m_cls) {
    lookupClass();
  }
  s_obj->setObject(ObjectData::newInstance(m_cls));
  ObjectData *obj = s_obj->getObject();
  g_context->invokeFuncFew(ret.asTypedValue(), m_ctor, obj);

  return obj;
}

bool SystemlibSessionModule::open(const char *save_path,
                                  const char *session_name) {
  ObjectData *obj = getObject();

  Variant savePath = String(save_path, CopyString);
  Variant sessionName = String(session_name, CopyString);
  Variant ret;
  TypedValue args[2] = { *savePath.asCell(), *sessionName.asCell() };
  g_context->invokeFuncFew(ret.asTypedValue(), m_open, obj,
                             nullptr, 2, args);

  if (ret.isBoolean() && ret.toBoolean()) {
    return true;
  }

  raise_warning("Failed calling %s::open()", m_classname);
  return false;
}

bool SystemlibSessionModule::close() {
  auto obj = s_obj->getObject();
  if (!obj) {
    // close() can be called twice in some circumstances
    return true;
  }

  Variant ret;
  g_context->invokeFuncFew(ret.asTypedValue(), m_close, obj);
  s_obj->destroy();

  if (ret.isBoolean() && ret.toBoolean()) {
    return true;
  }

  raise_warning("Failed calling %s::close()", m_classname);
  return false;
}

bool SystemlibSessionModule::read(const char *key, String &value) {
  ObjectData *obj = getObject();

  Variant sessionKey = String(key, CopyString);
  Variant ret;
  g_context->invokeFuncFew(ret.asTypedValue(), m_read, obj,
                             nullptr, 1, sessionKey.asCell());

  if (ret.isString()) {
    value = ret.toString();
    return true;
  }

  raise_warning("Failed calling %s::read()", m_classname);
  return false;
}

bool SystemlibSessionModule::write(const char *key, const String& value) {
  ObjectData *obj = getObject();

  Variant sessionKey = String(key, CopyString);
  Variant sessionVal = value;
  Variant ret;
  TypedValue args[2] = { *sessionKey.asCell(), *sessionVal.asCell() };
  g_context->invokeFuncFew(ret.asTypedValue(), m_write, obj,
                             nullptr, 2, args);

  if (ret.isBoolean() && ret.toBoolean()) {
    return true;
  }

  raise_warning("Failed calling %s::write()", m_classname);
  return false;
}

bool SystemlibSessionModule::destroy(const char *key) {
  ObjectData *obj = getObject();

  Variant sessionKey = String(key, CopyString);
  Variant ret;
  g_context->invokeFuncFew(ret.asTypedValue(), m_destroy, obj,
                             nullptr, 1, sessionKey.asCell());

  if (ret.isBoolean() && ret.toBoolean()) {
    return true;
  }

  raise_warning("Failed calling %s::destroy()", m_classname);
  return false;
}

bool SystemlibSessionModule::gc(int maxlifetime, int *nrdels) {
  ObjectData *obj = getObject();

  Variant maxLifeTime = maxlifetime;
  Variant ret;
  g_context->invokeFuncFew(ret.asTypedValue(), m_gc, obj,
                             nullptr, 1, maxLifeTime.asCell());

  if (ret.isInteger()) {
    if (nrdels) {
      *nrdels = ret.toInt64();
    }
    return true;
  }

  raise_warning("Failed calling %s::gc()", m_classname);
  return false;
}

//////////////////////////////////////////////////////////////////////////////
// SystemlibSessionModule implementations

static class RedisSessionModule : public SystemlibSessionModule {
 public:
  RedisSessionModule() :
    SystemlibSessionModule("redis", "RedisSessionModule") { }
} s_redis_session_module;

//////////////////////////////////////////////////////////////////////////////
// FileSessionModule

class FileSessionData {
public:
  FileSessionData() : m_fd(-1), m_dirdepth(0), m_st_size(0), m_filemode(0600) {
  }

  bool open(const char *save_path, const char *session_name) {
    String tmpdir;
    if (*save_path == '\0') {
      tmpdir = HHVM_FN(sys_get_temp_dir)();
      save_path = tmpdir.data();
    }

    /* split up input parameter */
    const char *argv[3];
    int argc = 0;
    const char *last = save_path;
    const char *p = strchr(save_path, ';');
    while (p) {
      argv[argc++] = last; last = ++p; p = strchr(p, ';');
      if (argc > 1) break;
    }
    argv[argc++] = last;

    if (argc > 1) {
      errno = 0;
      m_dirdepth = (size_t) strtol(argv[0], NULL, 10);
      if (errno == ERANGE) {
        raise_warning("The first parameter in session.save_path is invalid");
        return false;
      }
    }

    if (argc > 2) {
      errno = 0;
      m_filemode = strtol(argv[1], NULL, 8);
      if (errno == ERANGE || m_filemode < 0 || m_filemode > 07777) {
        raise_warning("The second parameter in session.save_path is invalid");
        return false;
      }
    }

    save_path = argv[argc - 1];
    if (File::TranslatePath(save_path).empty()) {
      raise_warning("Unable to open save_path %s", save_path);
      return false;
    }

    m_fd = -1;
    m_basedir = save_path;
    PS(mod_data) = true;
    return true;
  }

  bool close() {
    closeImpl();
    m_lastkey.clear();
    m_basedir.clear();
    PS(mod_data) = false;
    return true;
  }

  bool read(const char *key, String &value) {
    openImpl(key);
    if (m_fd < 0) {
      return false;
    }

    struct stat sbuf;
    if (fstat(m_fd, &sbuf)) {
      return false;
    }
    m_st_size = sbuf.st_size;
    if (m_st_size == 0) {
      value = "";
      return true;
    }

    String s = String(m_st_size, ReserveString);
    char *val = s.bufferSlice().ptr;

#if defined(HAVE_PREAD)
    long n = pread(m_fd, val, m_st_size, 0);
#else
    lseek(m_fd, 0, SEEK_SET);
    long n = ::read(m_fd, val, m_st_size);
#endif

    if (n != (int)m_st_size) {
      if (n == -1) {
        raise_warning("read failed: %s (%d)", folly::errnoStr(errno).c_str(),
                      errno);
      } else {
        raise_warning("read returned less bytes than requested");
      }
      return false;
    }

    value = s.setSize(m_st_size);
    return true;
  }

  bool write(const char *key, const String& value) {
    openImpl(key);
    if (m_fd < 0) {
      return false;
    }

    struct stat sbuf;
    if (fstat(m_fd, &sbuf)) {
      return false;
    }
    m_st_size = sbuf.st_size;

    /*
     * truncate file, if the amount of new data is smaller than
     * the existing data set.
     */
    if (value.size() < (int)m_st_size) {
      if (ftruncate(m_fd, 0) < 0) {
        raise_warning("truncate failed: %s (%d)",
                      folly::errnoStr(errno).c_str(), errno);
        return false;
      }
    }

#if defined(HAVE_PWRITE)
    long n = pwrite(m_fd, value.data(), value.size(), 0);
#else
    lseek(m_fd, 0, SEEK_SET);
    long n = ::write(m_fd, value.data(), value.size());
#endif

    if (n != value.size()) {
      if (n == -1) {
        raise_warning("write failed: %s (%d)",
                      folly::errnoStr(errno).c_str(), errno);
      } else {
        raise_warning("write wrote less bytes than requested");
      }
      return false;
    }

    return true;
  }

  bool destroy(const char *key) {
    char buf[PATH_MAX];
    if (!createPath(buf, sizeof(buf), key)) {
      return false;
    }

    if (m_fd != -1) {
      closeImpl();
      if (unlink(buf) == -1) {
        /* This is a little safety check for instances when we are dealing
           with a regenerated session that was not yet written to disk */
        if (!access(buf, F_OK)) {
          return false;
        }
      }
    }

    return true;
  }

  bool gc(int maxlifetime, int *nrdels) {
    /* we don't perform any cleanup, if dirdepth is larger than 0.
       we return true, since all cleanup should be handled by
       an external entity (i.e. find -ctime x | xargs rm) */
    if (m_dirdepth == 0) {
      *nrdels = CleanupDir(m_basedir.c_str(), maxlifetime);
    }
    return true;
  }

private:
  int m_fd;
  std::string m_lastkey;
  std::string m_basedir;
  size_t m_dirdepth;
  size_t m_st_size;
  int m_filemode;

  /* If you change the logic here, please also update the error message in
   * ps_files_open() appropriately */
  static bool IsValid(const char *key) {
    const char *p; char c;
    bool ret = true;
    for (p = key; (c = *p); p++) {
      /* valid characters are a..z,A..Z,0..9 */
      if (!((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z')
            || (c >= '0' && c <= '9') || c == ',' || c == '-')) {
        ret = false;
        break;
      }
    }
    size_t len = p - key;
    if (len == 0) {
      ret = false;
    }
    return ret;
  }

#define FILE_PREFIX "sess_"

  bool createPath(char *buf, size_t buflen, const char *key) {
    size_t key_len = strlen(key);
    if (key_len <= m_dirdepth ||
        buflen < (m_basedir.size() + 2 * m_dirdepth + key_len +
                  5 + sizeof(FILE_PREFIX))) {
      return false;
    }

    const char *p = key;
    int n = m_basedir.size();
    memcpy(buf, m_basedir.c_str(), n);
    buf[n++] = PHP_DIR_SEPARATOR;
    for (int i = 0; i < (int)m_dirdepth; i++) {
      buf[n++] = *p++;
      buf[n++] = PHP_DIR_SEPARATOR;
    }
    memcpy(buf + n, FILE_PREFIX, sizeof(FILE_PREFIX) - 1);
    n += sizeof(FILE_PREFIX) - 1;
    memcpy(buf + n, key, key_len);
    n += key_len;
    buf[n] = '\0';

    return true;
  }

#ifndef O_BINARY
#define O_BINARY 0
#endif

  void closeImpl() {
    if (m_fd != -1) {
#ifdef PHP_WIN32
      /* On Win32 locked files that are closed without being explicitly
         unlocked will be unlocked only when "system resources become
         available". */
      flock(m_fd, LOCK_UN);
#endif
      ::close(m_fd);
      m_fd = -1;
    }
  }

  void openImpl(const char *key) {
    if (m_fd < 0 || !m_lastkey.empty() || m_lastkey != key) {
      m_lastkey.clear();
      closeImpl();

      if (!IsValid(key)) {
        raise_warning("The session id contains illegal characters, "
                      "valid characters are a-z, A-Z, 0-9 and '-,'");
        PS(invalid_session_id) = true;
        return;
      }

      char buf[PATH_MAX];
      if (!createPath(buf, sizeof(buf), key)) {
        return;
      }

      m_lastkey = key;
      m_fd = ::open(buf, O_CREAT | O_RDWR | O_BINARY, m_filemode);

      if (m_fd != -1) {
#ifdef PHP_WIN32
        flock(m_fd, LOCK_EX);
#endif

#ifdef F_SETFD
# ifndef FD_CLOEXEC
#  define FD_CLOEXEC 1
# endif
        if (fcntl(m_fd, F_SETFD, FD_CLOEXEC)) {
          raise_warning("fcntl(%d, F_SETFD, FD_CLOEXEC) failed: %s (%d)",
                        m_fd, folly::errnoStr(errno).c_str(), errno);
        }
#endif
      } else {
        raise_warning("open(%s, O_RDWR) failed: %s (%d)", buf,
                      folly::errnoStr(errno).c_str(), errno);
      }
    }
  }

  static int CleanupDir(const char *dirname, int maxlifetime) {
    DIR *dir = opendir(dirname);
    if (!dir) {
      raise_notice("ps_files_cleanup_dir: opendir(%s) failed: %s (%d)",
                   dirname, folly::errnoStr(errno).c_str(), errno);
      return 0;
    }

    time_t now;
    time(&now);

    size_t dirname_len = strlen(dirname);
    char dentry[sizeof(struct dirent) + PATH_MAX];
    struct dirent *entry = (struct dirent *) &dentry;
    struct stat sbuf;
    int nrdels = 0;

    /* Prepare buffer (dirname never changes) */
    char buf[PATH_MAX];
    memcpy(buf, dirname, dirname_len);
    buf[dirname_len] = PHP_DIR_SEPARATOR;

    while (readdir_r(dir, (struct dirent *)dentry, &entry) == 0 && entry) {
      /* does the file start with our prefix? */
      if (!strncmp(entry->d_name, FILE_PREFIX, sizeof(FILE_PREFIX) - 1)) {
        size_t entry_len = strlen(entry->d_name);

        /* does it fit into our buffer? */
        if (entry_len + dirname_len + 2 < PATH_MAX) {
          /* create the full path.. */
          memcpy(buf + dirname_len + 1, entry->d_name, entry_len);

          /* NUL terminate it and */
          buf[dirname_len + entry_len + 1] = '\0';

          /* check whether its last access was more than maxlifet ago */
          if (stat(buf, &sbuf) == 0 && (now - sbuf.st_mtime) > maxlifetime) {
            unlink(buf);
            nrdels++;
          }
        }
      }
    }

    closedir(dir);
    return nrdels;
  }
};
IMPLEMENT_THREAD_LOCAL(FileSessionData, s_file_session_data);

class FileSessionModule : public SessionModule {
public:
  FileSessionModule() : SessionModule("files") {
  }
  virtual bool open(const char *save_path, const char *session_name) {
    return s_file_session_data->open(save_path, session_name);
  }
  virtual bool close() {
    return s_file_session_data->close();
  }
  virtual bool read(const char *key, String &value) {
    return s_file_session_data->read(key, value);
  }
  virtual bool write(const char *key, const String& value) {
    return s_file_session_data->write(key, value);
  }
  virtual bool destroy(const char *key) {
    return s_file_session_data->destroy(key);
  }
  virtual bool gc(int maxlifetime, int *nrdels) {
    return s_file_session_data->gc(maxlifetime, nrdels);
  }
};
static FileSessionModule s_file_session_module;

///////////////////////////////////////////////////////////////////////////////
// UserSessionModule

class UserSessionModule : public SessionModule {
public:
  UserSessionModule() : SessionModule("user") {}

  virtual bool open(const char *save_path, const char *session_name) {
    auto func = make_packed_array(Object(PS(ps_session_handler)), s_open);
    auto args = make_packed_array(String(save_path), String(session_name));

    auto res = vm_call_user_func(func, args);
    PS(mod_user_implemented) = true;
    return res.toBoolean();
  }

  virtual bool close() {
    auto func = make_packed_array(Object(PS(ps_session_handler)), s_close);
    auto args = Array::Create();

    auto res = vm_call_user_func(func, args);
    PS(mod_user_implemented) = false;
    return res.toBoolean();
  }

  virtual bool read(const char *key, String &value) {
    Variant ret = vm_call_user_func(
       make_packed_array(Object(PS(ps_session_handler)), s_read),
       make_packed_array(String(key))
    );
    if (ret.isString()) {
      value = ret.toString();
      return true;
    }
    return false;
  }

  virtual bool write(const char *key, const String& value) {
    return vm_call_user_func(
       make_packed_array(Object(PS(ps_session_handler)), s_write),
       make_packed_array(String(key, CopyString), value)
    ).toBoolean();
  }

  virtual bool destroy(const char *key) {
    return vm_call_user_func(
       make_packed_array(Object(PS(ps_session_handler)), s_destroy),
       make_packed_array(String(key))
    ).toBoolean();
  }

  virtual bool gc(int maxlifetime, int *nrdels) {
    return vm_call_user_func(
       make_packed_array(Object(PS(ps_session_handler)), s_gc),
       make_packed_array((int64_t)maxlifetime)
    ).toBoolean();
  }
};
static UserSessionModule s_user_session_module;

///////////////////////////////////////////////////////////////////////////////
// session serializers

class SessionSerializer {
public:
  explicit SessionSerializer(const char *name) : m_name(name) {
    RegisteredSerializers.push_back(this);
  }
  virtual ~SessionSerializer() {}

  const char *getName() const { return m_name; }

  virtual String encode() = 0;
  virtual bool decode(const String& value) = 0;

  static SessionSerializer *Find(const char *name) {
    for (unsigned int i = 0; i < RegisteredSerializers.size(); i++) {
      SessionSerializer *ss = RegisteredSerializers[i];
      if (ss && strcasecmp(name, ss->m_name) == 0) {
        return ss;
      }
    }
    return NULL;
  }

private:
  static std::vector<SessionSerializer*> RegisteredSerializers;

  const char *m_name;
};
std::vector<SessionSerializer*> SessionSerializer::RegisteredSerializers;

#define PS_BIN_NR_OF_BITS 8
#define PS_BIN_UNDEF (1<<(PS_BIN_NR_OF_BITS-1))
#define PS_BIN_MAX (PS_BIN_UNDEF-1)

class BinarySessionSerializer : public SessionSerializer {
public:
  BinarySessionSerializer() : SessionSerializer("php_binary") {}

  virtual String encode() {
    StringBuffer buf;
    GlobalVariables *g = get_global_variables();
    VariableSerializer vs(VariableSerializer::Type::Serialize);
    for (ArrayIter iter(g->get(s__SESSION).toArray()); iter; ++iter) {
      Variant key = iter.first();
      if (key.isString()) {
        String skey = key.toString();
        if (skey.size() <= PS_BIN_MAX) {
          buf.append((unsigned char)skey.size());
          buf.append(skey);
          buf.append(vs.serialize(iter.second(), /* ret */ true,
                                  /* keepCount */ true));
        }
      } else {
        raise_notice("Skipping numeric key %" PRId64, key.toInt64());
      }
    }
    return buf.detach();
  }

  virtual bool decode(const String& value) {
    const char *endptr = value.data() + value.size();
    GlobalVariables *g = get_global_variables();
    VariableUnserializer vu(nullptr, nullptr,
                            VariableUnserializer::Type::Serialize);
    for (const char *p = value.data(); p < endptr; ) {
      int namelen = ((unsigned char)(*p)) & (~PS_BIN_UNDEF);
      if (namelen < 0 || namelen > PS_BIN_MAX || (p + namelen) >= endptr) {
        return false;
      }

      int has_value = *p & PS_BIN_UNDEF ? 0 : 1;
      String key(p + 1, namelen, CopyString);
      p += namelen + 1;
      if (has_value) {
        vu.set(p, endptr);
        try {
          auto& sess = g->getRef(s__SESSION);
          forceToArray(sess).set(key, vu.unserialize());
          p = vu.head();
        } catch (Exception &e) {
        }
      }
    }
    return true;
  }
};
static BinarySessionSerializer s_binary_session_serializer;

#define PS_DELIMITER '|'
#define PS_UNDEF_MARKER '!'

class PhpSessionSerializer : public SessionSerializer {
public:
  PhpSessionSerializer() : SessionSerializer("php") {}

  virtual String encode() {
    StringBuffer buf;
    GlobalVariables *g = get_global_variables();
    VariableSerializer vs(VariableSerializer::Type::Serialize);
    for (ArrayIter iter(g->get(s__SESSION).toArray()); iter; ++iter) {
      Variant key = iter.first();
      if (key.isString()) {
        String skey = key.toString();
        buf.append(skey);
        if (skey.find(PS_DELIMITER) >= 0) {
          return String();
        }
        buf.append(PS_DELIMITER);
        buf.append(vs.serialize(iter.second(), /* ret */ true,
                                /* keepCount */ true));
      } else {
        raise_notice("Skipping numeric key %" PRId64, key.toInt64());
      }
    }
    return buf.detach();
  }

  virtual bool decode(const String& value) {
    const char *p = value.data();
    const char *endptr = value.data() + value.size();
    GlobalVariables *g = get_global_variables();
    VariableUnserializer vu(nullptr, nullptr,
                            VariableUnserializer::Type::Serialize);
    while (p < endptr) {
      const char *q = p;
      while (*q != PS_DELIMITER) {
        if (++q >= endptr) return true;
      }
      int has_value;
      if (p[0] == PS_UNDEF_MARKER) {
        p++;
        has_value = 0;
      } else {
        has_value = 1;
      }

      String key(p, q - p, CopyString);
      q++;
      if (has_value) {
        vu.set(q, endptr);
        try {
          auto& sess = g->getRef(s__SESSION);
          forceToArray(sess).set(key, vu.unserialize());
          q = vu.head();
        } catch (Exception &e) {
        }
      }
      p = q;
    }
    return true;
  }
};
static PhpSessionSerializer s_php_session_serializer;

///////////////////////////////////////////////////////////////////////////////

#define SESSION_CHECK_ACTIVE_STATE                                      \
  if (PS(session_status) == Session::Active) {                          \
    raise_warning("A session is active. You cannot change the session"  \
                  " module's ini settings at this time");               \
    return false;                                                       \
  }

static bool mod_is_open() {
  return PS(mod_data) || PS(mod_user_implemented);
}

static bool ini_on_update_save_handler(const std::string& value) {
  SESSION_CHECK_ACTIVE_STATE;
  PS(mod) = SessionModule::Find(value.c_str());
  return true;
}

static std::string ini_get_save_handler() {
  auto &mod = PS(mod);
  if (mod == nullptr) {
    return "";
  }
  return mod->getName();
}

static bool ini_on_update_serializer(const std::string& value) {
  SESSION_CHECK_ACTIVE_STATE;
  SessionSerializer *serializer = SessionSerializer::Find(value.data());
  if (serializer == nullptr) {
    raise_warning("ini_set(): Cannot find serialization handler '%s'",
                  value.data());
    return false;
  }
  PS(serializer) = serializer;
  return true;
}

static std::string ini_get_serializer() {
  auto &serializer = PS(serializer);
  if (serializer == nullptr) {
    return "";
  }
  return serializer->getName();
}

static bool ini_on_update_trans_sid(const bool& value) {
  SESSION_CHECK_ACTIVE_STATE;
  return true;
}

static bool ini_on_update_save_dir(const std::string& value) {
  if (value.find('\0') != std::string::npos) {
    return false;
  }
  const char *path = value.data() + value.rfind(';') + 1;
  if (File::TranslatePath(path).empty()) {
    return false;
  }
  PS(save_path) = path;
  return true;
}

///////////////////////////////////////////////////////////////////////////////

static int php_session_destroy() {
  int retval = true;

  if (PS(session_status) != Session::Active) {
    raise_warning("Trying to destroy uninitialized session");
    return false;
  }

  if (PS(mod)->destroy(PS(id).data()) == false) {
    retval = false;
    raise_warning("Session object destruction failed");
  }

  s_session->requestShutdownImpl();
  s_session->destroy();

  return retval;
}

static String php_session_encode() {
  if (!PS(serializer)) {
    raise_warning("Unknown session.serialize_handler. "
                  "Failed to encode session object");
    return String();
  }
  return PS(serializer)->encode();
}

static void php_session_decode(const String& value) {
  if (!PS(serializer)) {
    raise_warning("Unknown session.serialize_handler. "
                  "Failed to decode session object");
    return;
  }
  if (!PS(serializer)->decode(value)) {
    php_session_destroy();
    raise_warning("Failed to decode session object. "
                  "Session has been destroyed");
  }
}

static void php_session_initialize() {
  /* check session name for invalid characters */
  if (strpbrk(PS(id).data(), "\r\n\t <>'\"\\")) {
    PS(id).reset();
  }

  if (!PS(mod)) {
    raise_error("No storage module chosen - failed to initialize session");
    return;
  }

  /* Open session handler first */
  if (!PS(mod)->open(PS(save_path).c_str(), PS(session_name).c_str())) {
    raise_error("Failed to initialize storage module: %s (path: %s)",
                PS(mod)->getName(), PS(save_path).c_str());
    return;
  }

  /* If there is no ID, use session module to create one */
  int attempts = 3;
  if (PS(id).empty()) {
new_session:
    PS(id) = PS(mod)->create_sid();
    if (PS(id).empty()) {
      raise_error("Failed to create session id: %s", PS(mod)->getName());
      return;
    }
    if (PS(use_cookies)) {
      PS(send_cookie) = 1;
    }
  }

  /* Read data */
  /* Question: if you create a SID here, should you also try to read data?
   * I'm not sure, but while not doing so will remove one session operation
   * it could prove usefull for those sites which wish to have "default"
   * session information
   */

  /* Unconditionally destroy existing arrays -- possible dirty data */
  GlobalVariables *g = get_global_variables();
  g->set(s__SESSION, Array::Create(), false);

  PS(invalid_session_id) = false;
  String value;
  if (PS(mod)->read(PS(id).data(), value)) {
    php_session_decode(value);
  } else if (PS(invalid_session_id)) {
    /* address instances where the session read fails due to an invalid id */
    PS(invalid_session_id) = false;
    PS(id).reset();
    if (--attempts > 0) {
      goto new_session;
    }
  }
}

static void php_session_save_current_state() {
  bool ret = false;
  if (mod_is_open()) {
    String value = php_session_encode();
    if (!value.isNull()) {
      ret = PS(mod)->write(PS(id).data(), value);
    }
  }
  if (!ret) {
    raise_warning("Failed to write session data (%s). Please verify that the "
                  "current setting of session.save_path is correct (%s)",
                  PS(mod)->getName(), PS(save_path).c_str());
  }
  if (mod_is_open()) {
    PS(mod)->close();
  }
}

///////////////////////////////////////////////////////////////////////////////
// Cookie Management

static void php_session_send_cookie() {
  Transport *transport = g_context->getTransport();
  if (!transport) return;

  if (transport->headersSent()) {
    raise_warning("Cannot send session cookie - headers already sent");
    return;
  }

  int64_t expire = 0;
  if (PS(cookie_lifetime) > 0) {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    expire = tv.tv_sec + PS(cookie_lifetime);
  }
  transport->setCookie(PS(session_name), PS(id), expire, PS(cookie_path),
                       PS(cookie_domain), PS(cookie_secure),
                       PS(cookie_httponly), true);
}

static void php_session_reset_id() {
  if (PS(use_cookies) && PS(send_cookie)) {
    php_session_send_cookie();
    PS(send_cookie) = 0;
  }

  EnvConstants *g = get_env_constants();
  if (PS(define_sid)) {
    StringBuffer var;
    var.append(String(PS(session_name)));
    var.append('=');
    var.append(PS(id));
    g->k_SID = var.detach();
  } else {
    g->k_SID = empty_string;
  }

  // hzhao: not sure how to support this yet
#if 0
  if (PS(apply_trans_sid)) {
    php_url_scanner_reset_vars();
    php_url_scanner_add_var(PS(session_name), strlen(PS(session_name)),
                            PS(id), strlen(PS(id)), 1);
  }
#endif
}

///////////////////////////////////////////////////////////////////////////////
// Cache Limiters

typedef struct {
  char *name;
  void (*func)();
} php_session_cache_limiter_t;

#define CACHE_LIMITER(name) _php_cache_limiter_##name
#define CACHE_LIMITER_FUNC(name) static void CACHE_LIMITER(name)()
#define CACHE_LIMITER_ENTRY(name) { #name, CACHE_LIMITER(name) },
#define ADD_HEADER(hdr) g_context->getTransport()->addHeader(hdr)

#define LAST_MODIFIED "Last-Modified: "
#define EXPIRES "Expires: "
#define MAX_STR 512

static char *month_names[] = {
  "Jan", "Feb", "Mar", "Apr", "May", "Jun",
  "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
};

static char *week_days[] = {
  "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat", "Sun"
};

static inline void strcpy_gmt(char *ubuf, time_t *when) {
  char buf[MAX_STR];
  struct tm tm, *res;
  int n;

  res = gmtime_r(when, &tm);

  if (!res) {
    buf[0] = '\0';
    return;
  }

  n = snprintf(buf, sizeof(buf), "%s, %02d %s %d %02d:%02d:%02d GMT", // SAFE
               week_days[tm.tm_wday], tm.tm_mday,
               month_names[tm.tm_mon], tm.tm_year + 1900,
               tm.tm_hour, tm.tm_min,
               tm.tm_sec);
  memcpy(ubuf, buf, n);
  ubuf[n] = '\0';
}

const StaticString s_PATH_TRANSLATED("PATH_TRANSLATED");

static inline void last_modified() {
  GlobalVariables *g = get_global_variables();
  String path = g->get(s__SERVER).toArray()[s_PATH_TRANSLATED].toString();
  if (!path.empty()) {
    struct stat sb;
    if (stat(path.data(), &sb) == -1) {
      return;
    }

    char buf[MAX_STR + 1];
    memcpy(buf, LAST_MODIFIED, sizeof(LAST_MODIFIED) - 1);
    strcpy_gmt(buf + sizeof(LAST_MODIFIED) - 1, &sb.st_mtime);
    ADD_HEADER(buf);
  }
}

CACHE_LIMITER_FUNC(public) {
  char buf[MAX_STR + 1];
  struct timeval tv;
  time_t now;

  gettimeofday(&tv, NULL);
  now = tv.tv_sec + PS(cache_expire) * 60;
  memcpy(buf, EXPIRES, sizeof(EXPIRES) - 1);
  strcpy_gmt(buf + sizeof(EXPIRES) - 1, &now);
  ADD_HEADER(buf);

  snprintf(buf, sizeof(buf) , "Cache-Control: public, max-age=%" PRId64,
           PS(cache_expire) * 60); /* SAFE */
  ADD_HEADER(buf);

  last_modified();
}

CACHE_LIMITER_FUNC(private_no_expire) {
  char buf[MAX_STR + 1];

  snprintf(buf, sizeof(buf), "Cache-Control: private, max-age=%" PRId64 ", "
           "pre-check=%" PRId64, PS(cache_expire) * 60,
           PS(cache_expire) * 60); /* SAFE */
  ADD_HEADER(buf);

  last_modified();
}

CACHE_LIMITER_FUNC(private) {
  ADD_HEADER("Expires: Thu, 19 Nov 1981 08:52:00 GMT");
  CACHE_LIMITER(private_no_expire)();
}

CACHE_LIMITER_FUNC(nocache) {
  ADD_HEADER("Expires: Thu, 19 Nov 1981 08:52:00 GMT");

  /* For HTTP/1.1 conforming clients and the rest (MSIE 5) */
  ADD_HEADER("Cache-Control: no-store, no-cache, must-revalidate, "
             "post-check=0, pre-check=0");

  /* For HTTP/1.0 conforming clients */
  ADD_HEADER("Pragma: no-cache");
}

static php_session_cache_limiter_t php_session_cache_limiters[] = {
  CACHE_LIMITER_ENTRY(public)
  CACHE_LIMITER_ENTRY(private)
  CACHE_LIMITER_ENTRY(private_no_expire)
  CACHE_LIMITER_ENTRY(nocache)
  {0}
};

static int php_session_cache_limiter() {
  if (PS(cache_limiter)[0] == '\0') return 0;

  Transport *transport = g_context->getTransport();
  if (transport) {
    if (transport->headersSent()) {
      raise_warning("Cannot send session cache limiter - "
                    "headers already sent");
      return -2;
    }

    php_session_cache_limiter_t *lim;
    for (lim = php_session_cache_limiters; lim->name; lim++) {
      if (!strcasecmp(lim->name, PS(cache_limiter).c_str())) {
        lim->func();
        return 0;
      }
    }
  }

  return -1;
}

///////////////////////////////////////////////////////////////////////////////

int64_t f_session_status() {
  return PS(session_status);
}

void f_session_set_cookie_params(int64_t lifetime,
                                 const String& path /* = null_string */,
                                 const String& domain /* = null_string */,
                                 const Variant& secure /* = null */,
                                 const Variant& httponly /* = null */) {
  if (PS(use_cookies)) {
    HHVM_FN(ini_set)("session.cookie_lifetime", lifetime);
    if (!path.isNull()) {
      HHVM_FN(ini_set)("session.cookie_path", path);
    }
    if (!domain.isNull()) {
      HHVM_FN(ini_set)("session.cookie_domain", domain);
    }
    if (!secure.isNull()) {
      HHVM_FN(ini_set)("session.cookie_secure", secure.toBoolean());
    }
    if (!httponly.isNull()) {
      HHVM_FN(ini_set)("session.cookie_httponly", httponly.toBoolean());
    }
  }
}

const StaticString
  s_lifetime("lifetime"),
  s_path("path"),
  s_domain("domain"),
  s_secure("secure"),
  s_httponly("httponly");

Array f_session_get_cookie_params() {
  ArrayInit ret(5);
  ret.set(s_lifetime, PS(cookie_lifetime));
  ret.set(s_path,     String(PS(cookie_path)));
  ret.set(s_domain,   String(PS(cookie_domain)));
  ret.set(s_secure,   PS(cookie_secure));
  ret.set(s_httponly, PS(cookie_httponly));
  return ret.create();
}

String f_session_name(const String& newname /* = null_string */) {
  String oldname = String(PS(session_name));
  if (!newname.isNull()) {
    HHVM_FN(ini_set)("session.name", newname);
  }
  return oldname;
}

Variant f_session_module_name(const String& newname /* = null_string */) {
  String oldname;
  if (PS(mod) && PS(mod)->getName()) {
    oldname = String(PS(mod)->getName(), CopyString);
  }

  if (!newname.isNull()) {
    if (!SessionModule::Find(newname.data())) {
      raise_warning("Cannot find named PHP session module (%s)",
                    newname.data());
      return false;
    }
    if (mod_is_open()) {
      PS(mod)->close();
    }
    PS(mod_data) = false;

    HHVM_FN(ini_set)("session.save_handler", newname);
  }

  return oldname;
}

bool HHVM_FUNCTION(session_set_save_handler,
    const Object& sessionhandler,
    bool register_shutdown /* = true */) {

  if (PS(mod) &&
      PS(session_status) != Session::None &&
      PS(mod) != &s_user_session_module) {
    return false;
  }

  if (PS(session_status) == Session::Active) {
    return false;
  }

  if (PS(default_mod) == nullptr) {
    PS(default_mod) = PS(mod);
  }

  if (ObjectData* obj = PS(ps_session_handler)) {
    PS(ps_session_handler) = nullptr;
    decRefObj(obj);
  }
  PS(ps_session_handler) = sessionhandler.get();
  PS(ps_session_handler)->incRefCount();

  // remove previous shutdown function
  g_context->popShutdownFunction(ExecutionContext::ShutDown);
  if (register_shutdown) {
    f_register_shutdown_function(1, String("session_write_close"));
  }

  if (ini_get_save_handler() != "user") {
    HHVM_FN(ini_set)("session.save_handler", "user");
  }
  return true;
}

String f_session_save_path(const String& newname /* = null_string */) {
  if (!newname.isNull()) {
    if (memchr(newname.data(), '\0', newname.size()) != NULL) {
      raise_warning("The save_path cannot contain NULL characters");
      return false;
    }
    HHVM_FN(ini_set)("session.save_path", newname);
  }
  return String(PS(save_path));
}

String f_session_id(const String& newid /* = null_string */) {
  String ret = PS(id);
  if (ret.isNull()) {
    ret = empty_string;
  }

  if (!newid.isNull()) {
    PS(id) = newid;
  }

  return ret;
}

bool f_session_regenerate_id(bool delete_old_session /* = false */) {
  Transport *transport = g_context->getTransport();
  if (transport && transport->headersSent()) {
    raise_warning("Cannot regenerate session id - headers already sent");
    return false;
  }

  if (PS(session_status) == Session::Active) {
    if (!PS(id).empty()) {
      if (delete_old_session && !PS(mod)->destroy(PS(id).data())) {
        raise_warning("Session object destruction failed");
        return false;
      }
      PS(id).reset();
    }

    PS(id) = PS(mod)->create_sid();
    PS(send_cookie) = 1;
    php_session_reset_id();
    return true;
  }
  return false;
}

String f_session_cache_limiter(const String& new_cache_limiter /* = null_string */) {
  String ret(PS(cache_limiter));
  if (!new_cache_limiter.isNull()) {
    HHVM_FN(ini_set)("session.cache_limiter", new_cache_limiter);
  }
  return ret;
}

int64_t f_session_cache_expire(const String& new_cache_expire /* = null_string */) {
  int64_t ret = PS(cache_expire);
  if (!new_cache_expire.isNull()) {
    HHVM_FN(ini_set)("session.cache_expire", new_cache_expire.toInt64());
  }
  return ret;
}

Variant f_session_encode() {
  String ret = php_session_encode();
  if (ret.isNull()) {
    return false;
  }
  return ret;
}

bool f_session_decode(const String& data) {
  if (PS(session_status) != Session::None) {
    php_session_decode(data);
    return true;
  }
  return false;
}

const StaticString
  s_REQUEST_URI("REQUEST_URI"),
  s_HTTP_REFERER("HTTP_REFERER");

bool f_session_start() {
  PS(apply_trans_sid) = PS(use_trans_sid);

  String value;
  switch (PS(session_status)) {
  case Session::Active:
    raise_notice("A session had already been started - "
                 "ignoring session_start()");
    return false;
  case Session::Disabled:
    {
      if (!PS(mod) && IniSetting::Get("session.save_handler", value)) {
        PS(mod) = SessionModule::Find(value.data());
        if (!PS(mod)) {
          raise_warning("Cannot find save handler '%s' - "
                        "session startup failed", value.data());
          return false;
        }
      }
      if (!PS(serializer) &&
          IniSetting::Get("session.serialize_handler", value)) {
        PS(serializer) = SessionSerializer::Find(value.data());
        if (!PS(serializer)) {
          raise_warning("Cannot find serialization handler '%s' - "
                        "session startup failed", value.data());
          return false;
        }
      }
      PS(session_status) = Session::None;
      /* fallthrough */
    }
  default:
    assert(PS(session_status) == Session::None);
    PS(define_sid) = 1;
    PS(send_cookie) = 1;
  }

  /*
   * Cookies are preferred, because initially
   * cookie and get variables will be available.
   */
  GlobalVariables *g = get_global_variables();
  if (PS(id).empty()) {
    if (PS(use_cookies)) {
      auto cookies = g->get(s__COOKIE).toArray();
      if (cookies.exists(String(PS(session_name)))) {
        PS(id) = cookies[String(PS(session_name))].toString();
        PS(apply_trans_sid) = 0;
        PS(send_cookie) = 0;
        PS(define_sid) = 0;
      }
    }

    if (!PS(use_only_cookies) && !PS(id)) {
      auto get = g->get(s__GET).toArray();
      if (get.exists(String(PS(session_name)))) {
        PS(id) = get[String(PS(session_name))].toString();
        PS(send_cookie) = 0;
      }
    }

    if (!PS(use_only_cookies) && !PS(id)) {
      auto post = g->get(s__POST).toArray();
      if (post.exists(String(PS(session_name)))) {
        PS(id) = post[String(PS(session_name))].toString();
        PS(send_cookie) = 0;
      }
    }
  }

  int lensess = PS(session_name).size();

  /* check the REQUEST_URI symbol for a string of the form
     '<session-name>=<session-id>' to allow URLs of the form
     http://yoursite/<session-name>=<session-id>/script.php */
  if (!PS(use_only_cookies) && PS(id).empty()) {
    value = g->get(s__SERVER).toArray()[s_REQUEST_URI].toString();
    const char *p = strstr(value.data(), PS(session_name).c_str());
    if (p && p[lensess] == '=') {
      p += lensess + 1;
      const char *q;
      if ((q = strpbrk(p, "/?\\"))) {
        PS(id) = String(p, q - p, CopyString);
        PS(send_cookie) = 0;
      }
    }
  }

  /* check whether the current request was referred to by
     an external site which invalidates the previously found id */
  if (!PS(id).empty() && PS(extern_referer_chk)[0] != '\0') {
    value = g->get(s__SERVER).toArray()[s_HTTP_REFERER].toString();
    if (strstr(value.data(), PS(extern_referer_chk).c_str()) == NULL) {
      PS(id).reset();
      PS(send_cookie) = 1;
      if (PS(use_trans_sid)) {
        PS(apply_trans_sid) = 1;
      }
    }
  }

  php_session_initialize();

  if (!PS(use_cookies) && PS(send_cookie)) {
    if (PS(use_trans_sid)) {
      PS(apply_trans_sid) = 1;
    }
    PS(send_cookie) = 0;
  }

  php_session_reset_id();

  PS(session_status) = Session::Active;

  php_session_cache_limiter();

  if (mod_is_open() && PS(gc_probability) > 1) {
    int nrdels = -1;

    int nrand = (int) ((float) PS(gc_divisor) * math_combined_lcg());
    if (nrand < PS(gc_probability)) {
      PS(mod)->gc(PS(gc_maxlifetime), &nrdels);
    }
  }

  if (PS(session_status) != Session::Active) {
    return false;
  }
  return true;
}

bool f_session_destroy() {
  bool retval = true;

  if (PS(session_status) != Session::Active) {
    raise_warning("Trying to destroy uninitialized session");
    return false;
  }

  if (!PS(mod)->destroy(PS(id).data())) {
    retval = false;
    raise_warning("Session object destruction failed");
  }

  s_session->requestShutdownImpl();
  s_session->destroy();

  return retval;
}

Variant f_session_unset() {
  if (PS(session_status) == Session::None) {
    return false;
  }
  GlobalVariables *g = get_global_variables();
  g->getRef(s__SESSION) = Variant();
  return uninit_null();
}

void f_session_write_close() {
  if (PS(session_status) == Session::Active) {
    PS(session_status) = Session::None;
    php_session_save_current_state();
  }
}

void f_session_commit() {
  f_session_write_close();
}

bool f_session_register(int _argc, const Variant& var_names,
                        const Array& _argv /* = null_array */) {
  throw NotSupportedException
    (__func__, "Deprecated as of PHP 5.3.0 and REMOVED as of PHP 6.0.0. "
     "Relying on this feature is highly discouraged.");
}

bool f_session_unregister(const String& varname) {
  throw NotSupportedException
    (__func__, "Deprecated as of PHP 5.3.0 and REMOVED as of PHP 6.0.0. "
     "Relying on this feature is highly discouraged.");
}

bool f_session_is_registered(const String& varname) {
  throw NotSupportedException
    (__func__, "Deprecated as of PHP 5.3.0 and REMOVED as of PHP 6.0.0. "
     "Relying on this feature is highly discouraged.");
}

static bool HHVM_METHOD(SessionHandler, hhopen,
                        const String& save_path, const String& session_id) {
  return PS(default_mod) &&
    PS(default_mod)->open(save_path.data(), session_id.data());
}

static bool HHVM_METHOD(SessionHandler, hhclose) {
  return PS(default_mod) && PS(default_mod)->close();
}

static String HHVM_METHOD(SessionHandler, hhread, const String& session_id) {
  String value;
  if (PS(default_mod) && PS(default_mod)->read(PS(id).data(), value)) {
    php_session_decode(value);
    return value;
  }
  return uninit_null();
}

static bool HHVM_METHOD(SessionHandler, hhwrite,
                        const String& session_id, const String& session_data) {
  return PS(default_mod) &&
    PS(default_mod)->write(session_id.data(), session_data.data());
}

static bool HHVM_METHOD(SessionHandler, hhdestroy, const String& session_id) {
  return PS(default_mod) && PS(default_mod)->destroy(session_id.data());
}

static bool HHVM_METHOD(SessionHandler, hhgc, int maxlifetime) {
  int nrdels = -1;
  return PS(default_mod) && PS(default_mod)->gc(maxlifetime, &nrdels);
}

///////////////////////////////////////////////////////////////////////////////

static class SessionExtension : public Extension {
 public:
  SessionExtension() : Extension("session", NO_EXTENSION_VERSION_YET) { }
  virtual void moduleInit() {
    HHVM_ME(SessionHandler, hhopen);
    HHVM_ME(SessionHandler, hhclose);
    HHVM_ME(SessionHandler, hhread);
    HHVM_ME(SessionHandler, hhwrite);
    HHVM_ME(SessionHandler, hhdestroy);
    HHVM_ME(SessionHandler, hhgc);
    HHVM_NAMED_FE(__SystemLib\\session_set_save_handler,
                  HHVM_FN(session_set_save_handler)
    );
    loadSystemlib();
  }

  virtual void threadInit() {
    Extension* ext = Extension::GetExtension(s_session_ext_name);
    assert(ext);
    IniSetting::Bind(ext, IniSetting::PHP_INI_ALL,
                     "session.save_path",               "",
                     IniSetting::SetAndGet<std::string>(
                       ini_on_update_save_dir, nullptr
                     ),
                     &PS(save_path));
    IniSetting::Bind(ext, IniSetting::PHP_INI_ALL,
                     "session.name",                    "PHPSESSID",
                     &PS(session_name));
    IniSetting::Bind(ext, IniSetting::PHP_INI_ALL,
                     "session.save_handler",            "files",
                     IniSetting::SetAndGet<std::string>(
                       ini_on_update_save_handler, ini_get_save_handler
                     ));
    IniSetting::Bind(ext, IniSetting::PHP_INI_ALL,
                     "session.auto_start",              "0",
                     &PS(auto_start));
    IniSetting::Bind(ext, IniSetting::PHP_INI_ALL,
                     "session.gc_probability",          "1",
                     &PS(gc_probability));
    IniSetting::Bind(ext, IniSetting::PHP_INI_ALL,
                     "session.gc_divisor",              "100",
                     &PS(gc_divisor));
    IniSetting::Bind(ext, IniSetting::PHP_INI_ALL,
                     "session.gc_maxlifetime",          "1440",
                     &PS(gc_maxlifetime));
    IniSetting::Bind(ext, IniSetting::PHP_INI_ALL,
                     "session.serialize_handler",       "php",
                     IniSetting::SetAndGet<std::string>(
                       ini_on_update_serializer, ini_get_serializer
                     ));
    IniSetting::Bind(ext, IniSetting::PHP_INI_ALL,
                     "session.cookie_lifetime",         "0",
                     &PS(cookie_lifetime));
    IniSetting::Bind(ext, IniSetting::PHP_INI_ALL,
                     "session.cookie_path",             "/",
                     &PS(cookie_path));
    IniSetting::Bind(ext, IniSetting::PHP_INI_ALL,
                     "session.cookie_domain",           "",
                     &PS(cookie_domain));
    IniSetting::Bind(ext, IniSetting::PHP_INI_ALL,
                     "session.cookie_secure",           "",
                     &PS(cookie_secure));
    IniSetting::Bind(ext, IniSetting::PHP_INI_ALL,
                     "session.cookie_httponly",         "",
                     &PS(cookie_httponly));
    IniSetting::Bind(ext, IniSetting::PHP_INI_ALL,
                     "session.use_cookies",             "1",
                     &PS(use_cookies));
    IniSetting::Bind(ext, IniSetting::PHP_INI_ALL,
                     "session.use_only_cookies",        "1",
                     &PS(use_only_cookies));
    IniSetting::Bind(ext, IniSetting::PHP_INI_ALL,
                     "session.referer_check",           "",
                     &PS(extern_referer_chk));
    IniSetting::Bind(ext, IniSetting::PHP_INI_ALL,
                     "session.entropy_file",            "",
                     &PS(entropy_file));
    IniSetting::Bind(ext, IniSetting::PHP_INI_ALL,
                     "session.entropy_length",          "0",
                     &PS(entropy_length));
    IniSetting::Bind(ext, IniSetting::PHP_INI_ALL,
                     "session.cache_limiter",           "nocache",
                     &PS(cache_limiter));
    IniSetting::Bind(ext, IniSetting::PHP_INI_ALL,
                     "session.cache_expire",            "180",
                     &PS(cache_expire));
    IniSetting::Bind(ext, IniSetting::PHP_INI_ALL,
                     "session.use_trans_sid",           "0",
                     IniSetting::SetAndGet<bool>(
                       ini_on_update_trans_sid, nullptr
                     ),
                     &PS(use_trans_sid));
    IniSetting::Bind(ext, IniSetting::PHP_INI_ALL,
                     "session.hash_function",           "0",
                     &PS(hash_func));
    IniSetting::Bind(ext, IniSetting::PHP_INI_ALL,
                     "session.hash_bits_per_character", "4",
                     &PS(hash_bits_per_character));
  }

  virtual void requestInit() {
    // warm up the session data
    s_session->requestInit();
  }
} s_session_extension;

///////////////////////////////////////////////////////////////////////////////
}
