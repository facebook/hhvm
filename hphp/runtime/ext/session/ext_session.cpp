/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2014 Facebook, Inc. (http://www.facebook.com)     |
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
#include <sys/file.h>
#include <fcntl.h>
#include <dirent.h>
#include <vector>

#include <folly/String.h>

#include "hphp/util/lock.h"
#include "hphp/util/logger.h"
#include "hphp/util/compatibility.h"

#include "hphp/runtime/base/array-iterator.h"
#include "hphp/runtime/base/builtin-functions.h"
#include "hphp/runtime/base/comparisons.h"
#include "hphp/runtime/base/datetime.h"
#include "hphp/runtime/base/file.h"
#include "hphp/runtime/base/ini-setting.h"
#include "hphp/runtime/base/object-data.h"
#include "hphp/runtime/base/request-local.h"
#include "hphp/runtime/base/string-buffer.h"
#include "hphp/runtime/base/variable-serializer.h"
#include "hphp/runtime/base/variable-unserializer.h"
#include "hphp/runtime/base/php-globals.h"
#include "hphp/runtime/base/zend-math.h"
#include "hphp/runtime/ext/std/ext_std_function.h"
#include "hphp/runtime/ext/hash/ext_hash.h"
#include "hphp/runtime/ext/extension-registry.h"
#include "hphp/runtime/ext/std/ext_std_misc.h"
#include "hphp/runtime/ext/std/ext_std_options.h"
#include "hphp/runtime/ext/wddx/ext_wddx.h"
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
struct Session {
  enum Status {
    Disabled,
    None,
    Active
  };

  std::string save_path;
  std::string session_name;
  std::string extern_referer_chk;
  std::string entropy_file;
  int64_t     entropy_length{0};
  std::string cache_limiter;
  int64_t     cookie_lifetime{0};
  std::string cookie_path;
  std::string cookie_domain;
  Status      session_status{None};
  bool        cookie_secure{false};
  bool        cookie_httponly{false};
  bool        mod_data{false};
  bool        mod_user_implemented{false};

  SessionModule* mod{nullptr};
  SessionModule* default_mod{nullptr};

  int64_t  gc_probability{0};
  int64_t  gc_divisor{0};
  int64_t  gc_maxlifetime{0};
  int64_t  cache_expire{0};

  Object ps_session_handler;
  SessionSerializer* serializer{nullptr};

  bool auto_start{false};
  bool use_cookies{false};
  bool use_only_cookies{false};
  bool use_trans_sid{false}; // contains INI value of whether to use trans-sid
  bool apply_trans_sid{false}; // whether to enable trans-sid for current req
  bool send_cookie{false};
  bool define_sid{false};
  bool invalid_session_id{false};  /* allows the driver to report about an
                              invalid session id and request id regeneration */

  std::string hash_func;
  int64_t hash_bits_per_character{0};
};

const int64_t k_PHP_SESSION_DISABLED = Session::Disabled;
const int64_t k_PHP_SESSION_NONE     = Session::None;
const int64_t k_PHP_SESSION_ACTIVE   = Session::Active;
const StaticString s_session_ext_name("session");

struct SessionRequestData final : Session {
  void init() {
    id.detach();
    session_status = Session::None;
    ps_session_handler = nullptr;
  }

  void destroy() {
    id.reset();
    session_status = Session::None;
    // Note: we should not destroy user save handler here
    // (if the session is restarted during request, the handler
    // should be alive), it's destroyed only in the request shutdown.
  }

  void requestShutdownImpl();

public:
  String id;
};

static __thread SessionRequestData* s_session;

void SessionRequestData::requestShutdownImpl() {
  if (mod_is_open()) {
    try {
      mod->close();
    } catch (...) {}
  }
  ps_session_handler = nullptr;
  id.reset();
}

std::vector<SessionModule*> SessionModule::RegisteredModules;

/*
 * Note that we cannot use the BASE64 alphabet here, because
 * it contains "/" and "+": both are unacceptable for simple inclusion
 * into URLs.
 */
static char hexconvtab[] =
  "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ,-";

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
  String remote_addr = php_global(s__SERVER)
    .toArray()[s_REMOTE_ADDR].toString();

  struct timeval tv;
  gettimeofday(&tv, nullptr);

  StringBuffer buf;
  buf.printf("%.15s%ld%ld%0.8F", remote_addr.data(),
             tv.tv_sec, (long int)tv.tv_usec, math_combined_lcg() * 10);

  if (String(s_session->hash_func).isNumeric()) {
    switch (String(s_session->hash_func).toInt64()) {
    case md5:  s_session->hash_func = "md5";  break;
    case sha1: s_session->hash_func = "sha1"; break;
    }
  }

  Variant context = HHVM_FN(hash_init)(s_session->hash_func);
  if (same(context, false)) {
    Logger::Error("Invalid session hash function: %s",
                  s_session->hash_func.c_str());
    return String();
  }
  if (!HHVM_FN(hash_update)(context.toResource(), buf.detach())) {
    Logger::Error("hash_update() failed");
    return String();
  }

  if (s_session->entropy_length > 0) {
    int fd = ::open(s_session->entropy_file.c_str(), O_RDONLY);
    if (fd >= 0) {
      unsigned char rbuf[2048];
      int n;
      int to_read = s_session->entropy_length;
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

  String hashed = HHVM_FN(hash_final)(context.toResource(), /* raw */ true);

  if (s_session->hash_bits_per_character < 4 ||
      s_session->hash_bits_per_character > 6) {
    s_session->hash_bits_per_character = 4;
    raise_warning("The ini setting hash_bits_per_character is out of range "
                  "(should be 4, 5, or 6) - using 4 for now");
  }

  StringBuffer readable;
  bin_to_readable(hashed, readable, s_session->hash_bits_per_character);
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

LowPtr<Class> SystemlibSessionModule::s_SHIClass = nullptr;

/**
 * Relies on the fact that only one SessionModule will be active
 * in a given thread at any one moment.
 */
IMPLEMENT_REQUEST_LOCAL(SystemlibSessionInstance,
                        SystemlibSessionModule::s_obj);

Func* SystemlibSessionModule::lookupFunc(Class *cls, StringData *fname) {
  Func *f = cls->lookupMethod(fname);
  if (!f) {
    raise_error("class %s must declare method %s()",
                m_classname, fname->data());
  }

  if (f->attrs() & AttrStatic) {
    raise_error("%s::%s() must not be declared static",
                m_classname, fname->data());
  }

  if (f->attrs() & (AttrPrivate|AttrProtected|AttrAbstract)) {
    raise_error("%s::%s() must be declared public",
                m_classname, fname->data());
  }

  return f;
}

void SystemlibSessionModule::lookupClass() {
  Class *cls;
  if (!(cls = Unit::loadClass(String(m_classname, CopyString).get()))) {
    raise_error("Unable to locate systemlib class '%s'", m_classname);
  }

  if (cls->attrs() & (AttrTrait|AttrInterface)) {
    raise_error("'%s' must be a real class, not an interface or trait",
      m_classname);
  }

  if (!s_SHIClass) {
    s_SHIClass = Unit::lookupClass(s_SessionHandlerInterface.get());
    if (!s_SHIClass) {
      raise_error("Unable to locate '%s' interface",
                  s_SessionHandlerInterface.data());
    }
  }

  if (!cls->classof(s_SHIClass)) {
    raise_error("SystemLib session module '%s' must implement '%s'",
                m_classname,
                s_SessionHandlerInterface.data());
  }

  if (LookupResult::MethodFoundWithThis !=
      g_context->lookupCtorMethod(m_ctor, cls)) {
    raise_error("Unable to call %s's constructor", m_classname);
  }

  m_open    = lookupFunc(cls, s_open.get());
  m_close   = lookupFunc(cls, s_close.get());
  m_read    = lookupFunc(cls, s_read.get());
  m_write   = lookupFunc(cls, s_write.get());
  m_gc      = lookupFunc(cls, s_gc.get());
  m_destroy = lookupFunc(cls, s_destroy.get());
  m_cls = cls;
}

const Object& SystemlibSessionModule::getObject() {
  if (const auto& o = s_obj->getObject()) {
    return o;
  }

  VMRegAnchor _;
  Variant ret;

  if (!m_cls) {
    lookupClass();
  }
  s_obj->setObject(Object{m_cls});
  const auto& obj = s_obj->getObject();
  g_context->invokeFuncFew(ret.asTypedValue(), m_ctor, obj.get());
  return obj;
}

bool SystemlibSessionModule::open(const char *save_path,
                                  const char *session_name) {
  const auto& obj = getObject();

  Variant savePath = String(save_path, CopyString);
  Variant sessionName = String(session_name, CopyString);
  Variant ret;
  TypedValue args[2] = { *savePath.asCell(), *sessionName.asCell() };
  g_context->invokeFuncFew(ret.asTypedValue(), m_open, obj.get(),
                           nullptr, 2, args);

  if (ret.isBoolean() && ret.toBoolean()) {
    s_session->mod_data = true;
    return true;
  }

  raise_warning("Failed calling %s::open()", m_classname);
  return false;
}

bool SystemlibSessionModule::close() {
  const auto& obj = s_obj->getObject();
  if (!obj) {
    // close() can be called twice in some circumstances
    s_session->mod_data = false;
    return true;
  }

  Variant ret;
  g_context->invokeFuncFew(ret.asTypedValue(), m_close, obj.get());
  s_obj->destroy();

  if (ret.isBoolean() && ret.toBoolean()) {
    return true;
  }

  raise_warning("Failed calling %s::close()", m_classname);
  return false;
}

bool SystemlibSessionModule::read(const char *key, String &value) {
  const auto& obj = getObject();

  Variant sessionKey = String(key, CopyString);
  Variant ret;
  g_context->invokeFuncFew(ret.asTypedValue(), m_read, obj.get(),
                             nullptr, 1, sessionKey.asCell());

  if (ret.isString()) {
    value = ret.toString();
    return true;
  }

  raise_warning("Failed calling %s::read()", m_classname);
  return false;
}

bool SystemlibSessionModule::write(const char *key, const String& value) {
  const auto& obj = getObject();

  Variant sessionKey = String(key, CopyString);
  Variant sessionVal = value;
  Variant ret;
  TypedValue args[2] = { *sessionKey.asCell(), *sessionVal.asCell() };
  g_context->invokeFuncFew(ret.asTypedValue(), m_write, obj.get(),
                             nullptr, 2, args);

  if (ret.isBoolean() && ret.toBoolean()) {
    return true;
  }

  raise_warning("Failed calling %s::write()", m_classname);
  return false;
}

bool SystemlibSessionModule::destroy(const char *key) {
  const auto& obj = getObject();

  Variant sessionKey = String(key, CopyString);
  Variant ret;
  g_context->invokeFuncFew(ret.asTypedValue(), m_destroy, obj.get(),
                             nullptr, 1, sessionKey.asCell());

  if (ret.isBoolean() && ret.toBoolean()) {
    return true;
  }

  raise_warning("Failed calling %s::destroy()", m_classname);
  return false;
}

bool SystemlibSessionModule::gc(int maxlifetime, int *nrdels) {
  const auto& obj = getObject();

  Variant maxLifeTime = maxlifetime;
  Variant ret;
  g_context->invokeFuncFew(ret.asTypedValue(), m_gc, obj.get(),
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

static class MemcacheSessionModule : public SystemlibSessionModule {
 public:
  MemcacheSessionModule() :
    SystemlibSessionModule("memcache", "MemcacheSessionModule") { }
} s_memcache_session_module;

static class MemcachedSessionModule : public SystemlibSessionModule {
 public:
  MemcachedSessionModule() :
    SystemlibSessionModule("memcached", "MemcachedSessionModule") { }
} s_memcached_session_module;

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
      m_dirdepth = (size_t) strtol(argv[0], nullptr, 10);
      if (errno == ERANGE) {
        raise_warning("The first parameter in session.save_path is invalid");
        return false;
      }
    }

    if (argc > 2) {
      errno = 0;
      m_filemode = strtol(argv[1], nullptr, 8);
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
    s_session->mod_data = true;
    return true;
  }

  bool close() {
    closeImpl();
    m_lastkey.clear();
    m_basedir.clear();
    s_session->mod_data = false;
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
    char *val = s.mutableData();

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
        s_session->invalid_session_id = true;
        return;
      }

      char buf[PATH_MAX];
      if (!createPath(buf, sizeof(buf), key)) {
        return;
      }

      m_lastkey = key;
      m_fd = ::open(buf, O_CREAT | O_RDWR | O_BINARY, m_filemode);

      if (m_fd != -1) {
        flock(m_fd, LOCK_EX);

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

  bool open(const char *save_path, const char *session_name) override {
    auto func = make_packed_array(s_session->ps_session_handler, s_open);
    auto args = make_packed_array(String(save_path), String(session_name));

    auto res = vm_call_user_func(func, args);
    s_session->mod_user_implemented = true;
    return handleReturnValue(res);
  }

  bool close() override {
    auto func = make_packed_array(s_session->ps_session_handler, s_close);
    auto args = Array::Create();

    auto res = vm_call_user_func(func, args);
    s_session->mod_user_implemented = false;
    return handleReturnValue(res);
  }

  bool read(const char *key, String &value) override {
    Variant ret = vm_call_user_func(
       make_packed_array(s_session->ps_session_handler, s_read),
       make_packed_array(String(key))
    );
    if (ret.isString()) {
      value = ret.toString();
      return true;
    }
    return false;
  }

  bool write(const char *key, const String& value) override {
    return handleReturnValue(vm_call_user_func(
       make_packed_array(s_session->ps_session_handler, s_write),
       make_packed_array(String(key, CopyString), value)
    ));
  }

  bool destroy(const char *key) override {
    return handleReturnValue(vm_call_user_func(
       make_packed_array(s_session->ps_session_handler, s_destroy),
       make_packed_array(String(key))
    ));
  }

  bool gc(int maxlifetime, int *nrdels) override {
    return handleReturnValue(vm_call_user_func(
       make_packed_array(s_session->ps_session_handler, s_gc),
       make_packed_array((int64_t)maxlifetime)
    ));
  }

 private:
  bool handleReturnValue(const Variant& ret) {
    if (ret.isBoolean()) {
      return ret.toBoolean();
    }
    if (ret.isInteger()) {
      // BC fallbacks for values which work in PHP5 & PHP7
      auto i = ret.toInt64();
      if (i == -1) return false;
      if (i ==  0) return true;
    }
    raise_warning("Session callback expects true/false return value");
    return false;
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
    return nullptr;
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
    VariableSerializer vs(VariableSerializer::Type::Serialize);
    for (ArrayIter iter(php_global(s__SESSION).toArray()); iter; ++iter) {
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
    VariableUnserializer vu(nullptr, 0, VariableUnserializer::Type::Serialize);
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
          auto sess = php_global_exchange(s__SESSION, init_null());
          forceToArray(sess).set(key, vu.unserialize());
          php_global_set(s__SESSION, std::move(sess));
          p = vu.head();
        } catch (const ResourceExceededException&) {
          throw;
        } catch (const Exception&) {}
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
    VariableSerializer vs(VariableSerializer::Type::Serialize);
    for (ArrayIter iter(php_global(s__SESSION).toArray()); iter; ++iter) {
      Variant key = iter.first();
      if (key.isString()) {
        String skey = key.toString();
        buf.append(skey);
        if (skey.find(PS_DELIMITER) >= 0 || skey.find(PS_UNDEF_MARKER) >= 0) {
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
    VariableUnserializer vu(nullptr, 0, VariableUnserializer::Type::Serialize);
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
          auto sess = php_global_exchange(s__SESSION, init_null());
          forceToArray(sess).set(key, vu.unserialize());
          php_global_set(s__SESSION, std::move(sess));
          q = vu.head();
        } catch (const ResourceExceededException&) {
          throw;
        } catch (const Exception&) {}
      }
      p = q;
    }
    return true;
  }
};
static PhpSessionSerializer s_php_session_serializer;

class PhpSerializeSessionSerializer : public SessionSerializer {
public:
  PhpSerializeSessionSerializer() : SessionSerializer("php_serialize") {}

  virtual String encode() {
    VariableSerializer vs(VariableSerializer::Type::Serialize);
    return vs.serialize(php_global(s__SESSION).toArray(), true, true);
  }

  virtual bool decode(const String& value) {
    VariableUnserializer vu(value.data(), value.size(),
                            VariableUnserializer::Type::Serialize);

    try {
      auto sess = vu.unserialize();
      php_global_set(s__SESSION, std::move(sess.toArray()));
    } catch (const ResourceExceededException&) {
      throw;
    } catch (const Exception&) {}

    return true;
  }
};
static PhpSerializeSessionSerializer s_php_serialize_session_serializer;

class WddxSessionSerializer : public SessionSerializer {
public:
  WddxSessionSerializer() : SessionSerializer("wddx") {}

  virtual String encode() {
    auto wddxPacket = req::make<WddxPacket>(empty_string_variant_ref,
                                               true, true);
    for (ArrayIter iter(php_global(s__SESSION).toArray()); iter; ++iter) {
      Variant key = iter.first();
      if (key.isString()) {
        wddxPacket->recursiveAddVar(key.toString(), iter.second(), true);
      } else {
        raise_notice("Skipping numeric key %" PRId64, key.toInt64());
      }
    }

    return wddxPacket->packet_end();
  }

  virtual bool decode(const String& value) {
    Array params = Array::Create();
    params.append(value);
    Variant ret = vm_call_user_func("wddx_deserialize", params, true);
    if (ret.isArray()) {
      Array arr = ret.toArray();
      auto session = php_global_exchange(s__SESSION, init_null());
      for (ArrayIter iter(arr); iter; ++iter) {
        auto const key = iter.first();
        auto const value = iter.second();
        forceToArray(session).set(key, value);
      }
      php_global_set(s__SESSION, std::move(session));
    }

    return true;
  }
};
static WddxSessionSerializer s_wddx_session_serializer;

///////////////////////////////////////////////////////////////////////////////

static bool session_check_active_state() {
  if (s_session->session_status == Session::Active) {
    raise_warning("A session is active. You cannot change the session"
                  " module's ini settings at this time");
    return false;
  }
  return true;
}

static bool mod_is_open() {
  return s_session->mod_data || s_session->mod_user_implemented;
}

static bool ini_on_update_save_handler(const std::string& value) {
  if (!session_check_active_state()) return false;
  s_session->mod = SessionModule::Find(value.c_str());
  return true;
}

static std::string ini_get_save_handler() {
  auto &mod = s_session->mod;
  if (mod == nullptr) {
    return "";
  }
  return mod->getName();
}

static bool ini_on_update_serializer(const std::string& value) {
  if (!session_check_active_state()) return false;
  SessionSerializer *serializer = SessionSerializer::Find(value.data());
  if (serializer == nullptr) {
    raise_warning("ini_set(): Cannot find serialization handler '%s'",
                  value.data());
    return false;
  }
  s_session->serializer = serializer;
  return true;
}

static std::string ini_get_serializer() {
  auto &serializer = s_session->serializer;
  if (serializer == nullptr) {
    return "";
  }
  return serializer->getName();
}

static bool ini_on_update_trans_sid(const bool& value) {
  return session_check_active_state();
}

static bool ini_on_update_save_dir(const std::string& value) {
  if (value.find('\0') != std::string::npos) {
    return false;
  }
  const char *path = value.data() + value.rfind(';') + 1;
  if (File::TranslatePath(path).empty()) {
    return false;
  }
  s_session->save_path = path;
  return true;
}

///////////////////////////////////////////////////////////////////////////////

static bool php_session_destroy() {
  bool retval = true;

  if (s_session->session_status != Session::Active) {
    raise_warning("Trying to destroy uninitialized session");
    return false;
  }

  if (s_session->mod->destroy(s_session->id.data()) == false) {
    retval = false;
    raise_warning("Session object destruction failed");
  }

  if (mod_is_open()) {
    s_session->mod->close();
  }

  s_session->destroy();

  return retval;
}

static String php_session_encode() {
  if (!s_session->serializer) {
    raise_warning("Unknown session.serialize_handler. "
                  "Failed to encode session object");
    return String();
  }
  return s_session->serializer->encode();
}

static void php_session_decode(const String& value) {
  if (!s_session->serializer) {
    raise_warning("Unknown session.serialize_handler. "
                  "Failed to decode session object");
    return;
  }
  if (!s_session->serializer->decode(value)) {
    php_session_destroy();
    raise_warning("Failed to decode session object. "
                  "Session has been destroyed");
  }
}

static void php_session_initialize() {
  /* check session name for invalid characters */
  if (strpbrk(s_session->id.data(), "\r\n\t <>'\"\\")) {
    s_session->id.reset();
  }

  if (!s_session->mod) {
    raise_error("No storage module chosen - failed to initialize session");
    return;
  }

  /* Open session handler first */
  if (!s_session->mod->open(s_session->save_path.c_str(),
                            s_session->session_name.c_str())) {
    raise_error("Failed to initialize storage module: %s (path: %s)",
                s_session->mod->getName(), s_session->save_path.c_str());
    return;
  }

  /* If there is no ID, use session module to create one */
  int attempts = 3;
  if (s_session->id.empty()) {
new_session:
    s_session->id = s_session->mod->create_sid();
    if (s_session->id.empty()) {
      raise_error("Failed to create session id: %s", s_session->mod->getName());
      return;
    }
    if (s_session->use_cookies) {
      s_session->send_cookie = true;
    }
  }

  /* Read data */
  /* Question: if you create a SID here, should you also try to read data?
   * I'm not sure, but while not doing so will remove one session operation
   * it could prove useful for those sites which wish to have "default"
   * session information
   */

  /* Unconditionally destroy existing arrays -- possible dirty data */
  php_global_set(s__SESSION, staticEmptyArray());

  s_session->invalid_session_id = false;
  String value;
  if (s_session->mod->read(s_session->id.data(), value)) {
    php_session_decode(value);
  } else if (s_session->invalid_session_id) {
    /* address instances where the session read fails due to an invalid id */
    s_session->invalid_session_id = false;
    s_session->id.reset();
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
      ret = s_session->mod->write(s_session->id.data(), value);
    }
  }
  if (!ret) {
    raise_warning("Failed to write session data (%s). Please verify that the "
                  "current setting of session.save_path is correct (%s)",
                  s_session->mod->getName(), s_session->save_path.c_str());
  }
  if (mod_is_open()) {
    s_session->mod->close();
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
  if (s_session->cookie_lifetime > 0) {
    struct timeval tv;
    gettimeofday(&tv, nullptr);
    expire = tv.tv_sec + s_session->cookie_lifetime;
  }
  transport->setCookie(s_session->session_name,
                       s_session->id,
                       expire,
                       s_session->cookie_path,
                       s_session->cookie_domain,
                       s_session->cookie_secure,
                       s_session->cookie_httponly, true);
}

static void php_session_reset_id() {
  if (s_session->use_cookies && s_session->send_cookie) {
    php_session_send_cookie();
    s_session->send_cookie = false;
  }

  if (s_session->define_sid) {
    StringBuffer var;
    var.append(String(s_session->session_name));
    var.append('=');
    var.append(s_session->id);
    Variant v = var.detach();

    static const auto s_SID = makeStaticString("SID");
    auto const handle = lookupCnsHandle(s_SID);
    if (!handle) {
      f_define(s_SID, v);
    } else {
      TypedValue* cns = &rds::handleToRef<TypedValue>(handle);
      v.setEvalScalar();
      cns->m_data = v.asTypedValue()->m_data;
      cns->m_type = v.asTypedValue()->m_type;
    }
  }
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
  String path = php_global(s__SERVER).toArray()[s_PATH_TRANSLATED].toString();
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

  gettimeofday(&tv, nullptr);
  now = tv.tv_sec + s_session->cache_expire * 60;
  memcpy(buf, EXPIRES, sizeof(EXPIRES) - 1);
  strcpy_gmt(buf + sizeof(EXPIRES) - 1, &now);
  ADD_HEADER(buf);

  snprintf(buf, sizeof(buf) , "Cache-Control: public, max-age=%" PRId64,
           s_session->cache_expire * 60); /* SAFE */
  ADD_HEADER(buf);

  last_modified();
}

CACHE_LIMITER_FUNC(private_no_expire) {
  char buf[MAX_STR + 1];

  snprintf(buf, sizeof(buf), "Cache-Control: private, max-age=%" PRId64 ", "
           "pre-check=%" PRId64, s_session->cache_expire * 60,
           s_session->cache_expire * 60); /* SAFE */
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
  if (s_session->cache_limiter[0] == '\0') return 0;

  Transport *transport = g_context->getTransport();
  if (transport) {
    if (transport->headersSent()) {
      raise_warning("Cannot send session cache limiter - "
                    "headers already sent");
      return -2;
    }

    php_session_cache_limiter_t *lim;
    for (lim = php_session_cache_limiters; lim->name; lim++) {
      if (!strcasecmp(lim->name, s_session->cache_limiter.c_str())) {
        lim->func();
        return 0;
      }
    }
  }

  return -1;
}

///////////////////////////////////////////////////////////////////////////////

static int64_t HHVM_FUNCTION(session_status) {
  return s_session->session_status;
}

static Variant HHVM_FUNCTION(session_module_name,
                const Variant& newname /* = null_string */) {
  String oldname;
  if (s_session->mod && s_session->mod->getName()) {
    oldname = String(s_session->mod->getName(), CopyString);
  }

  if (!newname.isNull()) {
    if (!SessionModule::Find(newname.toString().data())) {
      raise_warning("Cannot find named PHP session module (%s)",
                    newname.toString().data());
      return false;
    }
    if (mod_is_open()) {
      s_session->mod->close();
    }
    s_session->mod_data = false;

    HHVM_FN(ini_set)("session.save_handler", newname.toString());
  }

  return oldname;
}

const StaticString s_session_write_close("session_write_close");

static bool HHVM_FUNCTION(session_set_save_handler,
    const Object& sessionhandler,
    bool register_shutdown /* = true */) {

  if (s_session->mod &&
      s_session->session_status != Session::None &&
      s_session->mod != &s_user_session_module) {
    return false;
  }

  if (s_session->session_status == Session::Active) {
    return false;
  }

  if (s_session->default_mod == nullptr) {
    s_session->default_mod = s_session->mod;
  }

  s_session->ps_session_handler = sessionhandler;

  // remove previous shutdown function
  g_context->removeShutdownFunction(s_session_write_close,
                                    ExecutionContext::ShutDown);
  if (register_shutdown) {
    HHVM_FN(register_shutdown_function)(s_session_write_close);
  }

  if (ini_get_save_handler() != "user") {
    HHVM_FN(ini_set)("session.save_handler", "user");
  }
  return true;
}

static String HHVM_FUNCTION(session_id,
                            const Variant& newid /* = null_string */) {
  String ret = s_session->id;
  if (ret.isNull()) {
    ret = empty_string();
  }

  if (!newid.isNull()) {
    s_session->id = newid;
  }

  return ret;
}

static bool HHVM_FUNCTION(session_regenerate_id,
                          bool delete_old_session /* = false */) {
  Transport *transport = g_context->getTransport();
  if (transport && transport->headersSent()) {
    raise_warning("Cannot regenerate session id - headers already sent");
    return false;
  }

  if (s_session->session_status == Session::Active) {
    if (!s_session->id.empty()) {
      if (delete_old_session &&
          !s_session->mod->destroy(s_session->id.data())) {
        raise_warning("Session object destruction failed");
        return false;
      }
      s_session->id.reset();
    }

    s_session->id = s_session->mod->create_sid();
    s_session->send_cookie = true;
    php_session_reset_id();
    return true;
  }
  return false;
}

static Variant HHVM_FUNCTION(session_encode) {
  String ret = php_session_encode();
  if (ret.isNull()) {
    return false;
  }
  return ret;
}

static bool HHVM_FUNCTION(session_decode, const String& data) {
  if (s_session->session_status != Session::None) {
    php_session_decode(data);
    return true;
  }
  return false;
}

const StaticString
  s_REQUEST_URI("REQUEST_URI"),
  s_HTTP_REFERER("HTTP_REFERER");

static bool HHVM_FUNCTION(session_start) {
  s_session->apply_trans_sid = s_session->use_trans_sid;

  String value;
  switch (s_session->session_status) {
  case Session::Active:
    raise_notice("A session had already been started - "
                 "ignoring session_start()");
    return false;
  case Session::Disabled:
    {
      if (!s_session->mod && IniSetting::Get("session.save_handler", value)) {
        s_session->mod = SessionModule::Find(value.data());
        if (!s_session->mod) {
          raise_warning("Cannot find save handler '%s' - "
                        "session startup failed", value.data());
          return false;
        }
      }
      if (!s_session->serializer &&
          IniSetting::Get("session.serialize_handler", value)) {
        s_session->serializer = SessionSerializer::Find(value.data());
        if (!s_session->serializer) {
          raise_warning("Cannot find serialization handler '%s' - "
                        "session startup failed", value.data());
          return false;
        }
      }
      s_session->session_status = Session::None;
      /* fallthrough */
    }
  default:
    assert(s_session->session_status == Session::None);
    s_session->define_sid = true;
    s_session->send_cookie = true;
  }

  /*
   * Cookies are preferred, because initially
   * cookie and get variables will be available.
   */
  if (s_session->id.empty()) {
    if (s_session->use_cookies) {
      auto cookies = php_global(s__COOKIE).toArray();
      if (cookies.exists(String(s_session->session_name))) {
        s_session->id = cookies[String(s_session->session_name)].toString();
        s_session->apply_trans_sid = false;
        s_session->send_cookie = false;
        s_session->define_sid = false;
      }
    }

    if (!s_session->use_only_cookies && !s_session->id) {
      auto get = php_global(s__GET).toArray();
      if (get.exists(String(s_session->session_name))) {
        s_session->id = get[String(s_session->session_name)].toString();
        s_session->send_cookie = false;
      }
    }

    if (!s_session->use_only_cookies && !s_session->id) {
      auto post = php_global(s__POST).toArray();
      if (post.exists(String(s_session->session_name))) {
        s_session->id = post[String(s_session->session_name)].toString();
        s_session->send_cookie = false;
      }
    }
  }

  int lensess = s_session->session_name.size();

  /* check the REQUEST_URI symbol for a string of the form
     '<session-name>=<session-id>' to allow URLs of the form
     http://yoursite/<session-name>=<session-id>/script.php */
  if (!s_session->use_only_cookies && s_session->id.empty()) {
    value = php_global(s__SERVER).toArray()[s_REQUEST_URI].toString();
    const char *p = strstr(value.data(), s_session->session_name.c_str());
    if (p && p[lensess] == '=') {
      p += lensess + 1;
      const char *q;
      if ((q = strpbrk(p, "/?\\"))) {
        s_session->id = String(p, q - p, CopyString);
        s_session->send_cookie = false;
      }
    }
  }

  /* check whether the current request was referred to by
     an external site which invalidates the previously found id */
  if (!s_session->id.empty() && s_session->extern_referer_chk[0] != '\0') {
    value = php_global(s__SERVER).toArray()[s_HTTP_REFERER].toString();
    if (!strstr(value.data(), s_session->extern_referer_chk.c_str())) {
      s_session->id.reset();
      s_session->send_cookie = true;
      if (s_session->use_trans_sid) {
        s_session->apply_trans_sid = true;
      }
    }
  }

  php_session_initialize();

  if (!s_session->use_cookies && s_session->send_cookie) {
    if (s_session->use_trans_sid) {
      s_session->apply_trans_sid = true;
    }
    s_session->send_cookie = false;
  }

  php_session_reset_id();

  s_session->session_status = Session::Active;

  php_session_cache_limiter();

  if (mod_is_open() && s_session->gc_probability > 0) {
    int nrdels = -1;

    int nrand = (int) ((float) s_session->gc_divisor * math_combined_lcg());
    if (nrand < s_session->gc_probability) {
      s_session->mod->gc(s_session->gc_maxlifetime, &nrdels);
    }
  }

  if (s_session->session_status != Session::Active) {
    return false;
  }
  return true;
}

static bool HHVM_FUNCTION(session_destroy) {
  return php_session_destroy();
}

static void HHVM_FUNCTION(session_unset) {
  if (s_session->session_status == Session::None) {
    return;
  }
  php_global_set(s__SESSION, empty_array());
  return;
}

static void HHVM_FUNCTION(session_write_close) {
  if (s_session->session_status == Session::Active) {
    s_session->session_status = Session::None;
    php_session_save_current_state();
  }
}

static bool HHVM_METHOD(SessionHandler, hhopen,
                        const String& save_path, const String& session_id) {
  return s_session->default_mod &&
    s_session->default_mod->open(save_path.data(), session_id.data());
}

static bool HHVM_METHOD(SessionHandler, hhclose) {
  return s_session->default_mod && s_session->default_mod->close();
}

static Variant HHVM_METHOD(SessionHandler, hhread, const String& session_id) {
  String value;
  if (s_session->default_mod &&
      s_session->default_mod->read(s_session->id.data(), value)) {
    php_session_decode(value);
    return value;
  }
  return init_null();
}

static bool HHVM_METHOD(SessionHandler, hhwrite,
                        const String& session_id, const String& session_data) {
  return s_session->default_mod &&
    s_session->default_mod->write(session_id.data(), session_data);
}

static bool HHVM_METHOD(SessionHandler, hhdestroy, const String& session_id) {
  return s_session->default_mod &&
    s_session->default_mod->destroy(session_id.data());
}

static bool HHVM_METHOD(SessionHandler, hhgc, int maxlifetime) {
  int nrdels = -1;
  return s_session->default_mod &&
    s_session->default_mod->gc(maxlifetime, &nrdels);
}

void ext_session_request_shutdown() {
  HHVM_FN(session_write_close)();
  s_session->requestShutdownImpl();
}

///////////////////////////////////////////////////////////////////////////////

const StaticString s_PHP_SESSION_DISABLED("PHP_SESSION_DISABLED");
const StaticString s_PHP_SESSION_NONE("PHP_SESSION_NONE");
const StaticString s_PHP_SESSION_ACTIVE("PHP_SESSION_ACTIVE");

static class SessionExtension final : public Extension {
 public:
  SessionExtension() : Extension("session", NO_EXTENSION_VERSION_YET) { }
  void moduleInit() override {
    Native::registerConstant<KindOfInt64>(
      s_PHP_SESSION_DISABLED.get(), k_PHP_SESSION_DISABLED
    );
    Native::registerConstant<KindOfInt64>(
      s_PHP_SESSION_NONE.get(), k_PHP_SESSION_NONE
    );
    Native::registerConstant<KindOfInt64>(
      s_PHP_SESSION_ACTIVE.get(), k_PHP_SESSION_ACTIVE
    );

    HHVM_FE(session_status);
    HHVM_FE(session_module_name);
    HHVM_FE(session_id);
    HHVM_FE(session_regenerate_id);
    HHVM_FE(session_encode);
    HHVM_FE(session_decode);
    HHVM_FE(session_start);
    HHVM_FE(session_destroy);
    HHVM_FE(session_unset);
    HHVM_FE(session_write_close);

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

  void threadInit() override {
    // TODO: t5226715 We shouldn't need to check s_session here, but right now
    // this is called for every request.
    if (s_session) return;
    s_session = new SessionRequestData;
    Extension* ext = ExtensionRegistry::get(s_session_ext_name);
    assert(ext);
    IniSetting::Bind(ext, IniSetting::PHP_INI_ALL,
                     "session.save_path",               "",
                     IniSetting::SetAndGet<std::string>(
                       ini_on_update_save_dir, nullptr
                     ),
                     &s_session->save_path);
    IniSetting::Bind(ext, IniSetting::PHP_INI_ALL,
                     "session.name",                    "PHPSESSID",
                     &s_session->session_name);
    IniSetting::Bind(ext, IniSetting::PHP_INI_ALL,
                     "session.save_handler",            "files",
                     IniSetting::SetAndGet<std::string>(
                       ini_on_update_save_handler, ini_get_save_handler
                     ));
    IniSetting::Bind(ext, IniSetting::PHP_INI_ALL,
                     "session.auto_start",              "0",
                     &s_session->auto_start);
    IniSetting::Bind(ext, IniSetting::PHP_INI_ALL,
                     "session.gc_probability",          "1",
                     &s_session->gc_probability);
    IniSetting::Bind(ext, IniSetting::PHP_INI_ALL,
                     "session.gc_divisor",              "100",
                     &s_session->gc_divisor);
    IniSetting::Bind(ext, IniSetting::PHP_INI_ALL,
                     "session.gc_maxlifetime",          "1440",
                     &s_session->gc_maxlifetime);
    IniSetting::Bind(ext, IniSetting::PHP_INI_ALL,
                     "session.serialize_handler",       "php",
                     IniSetting::SetAndGet<std::string>(
                       ini_on_update_serializer, ini_get_serializer
                     ));
    IniSetting::Bind(ext, IniSetting::PHP_INI_ALL,
                     "session.cookie_lifetime",         "0",
                     &s_session->cookie_lifetime);
    IniSetting::Bind(ext, IniSetting::PHP_INI_ALL,
                     "session.cookie_path",             "/",
                     &s_session->cookie_path);
    IniSetting::Bind(ext, IniSetting::PHP_INI_ALL,
                     "session.cookie_domain",           "",
                     &s_session->cookie_domain);
    IniSetting::Bind(ext, IniSetting::PHP_INI_ALL,
                     "session.cookie_secure",           "",
                     &s_session->cookie_secure);
    IniSetting::Bind(ext, IniSetting::PHP_INI_ALL,
                     "session.cookie_httponly",         "",
                     &s_session->cookie_httponly);
    IniSetting::Bind(ext, IniSetting::PHP_INI_ALL,
                     "session.use_cookies",             "1",
                     &s_session->use_cookies);
    IniSetting::Bind(ext, IniSetting::PHP_INI_ALL,
                     "session.use_only_cookies",        "1",
                     &s_session->use_only_cookies);
    IniSetting::Bind(ext, IniSetting::PHP_INI_ALL,
                     "session.referer_check",           "",
                     &s_session->extern_referer_chk);
    IniSetting::Bind(ext, IniSetting::PHP_INI_ALL,
                     "session.entropy_file",            "",
                     &s_session->entropy_file);
    IniSetting::Bind(ext, IniSetting::PHP_INI_ALL,
                     "session.entropy_length",          "0",
                     &s_session->entropy_length);
    IniSetting::Bind(ext, IniSetting::PHP_INI_ALL,
                     "session.cache_limiter",           "nocache",
                     &s_session->cache_limiter);
    IniSetting::Bind(ext, IniSetting::PHP_INI_ALL,
                     "session.cache_expire",            "180",
                     &s_session->cache_expire);
    IniSetting::Bind(ext, IniSetting::PHP_INI_ALL,
                     "session.use_trans_sid",           "0",
                     IniSetting::SetAndGet<bool>(
                       ini_on_update_trans_sid, nullptr
                     ),
                     &s_session->use_trans_sid);
    IniSetting::Bind(ext, IniSetting::PHP_INI_ALL,
                     "session.hash_function",           "0",
                     &s_session->hash_func);
    IniSetting::Bind(ext, IniSetting::PHP_INI_ALL,
                     "session.hash_bits_per_character", "4",
                     &s_session->hash_bits_per_character);
  }

  void threadShutdown() override {
    delete s_session;
    s_session = nullptr;
  }

  void requestInit() override {
    s_session->init();
  }

  /*
    No need for requestShutdown; its handled explicitly by a call to
    ext_session_request_shutdown()
  */
} s_session_extension;

///////////////////////////////////////////////////////////////////////////////
}
