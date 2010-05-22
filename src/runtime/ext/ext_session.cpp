/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010 Facebook, Inc. (http://www.facebook.com)          |
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

#include <runtime/ext/ext_session.h>
#include <runtime/ext/ext_options.h>
#include <runtime/ext/ext_hash.h>
#include <runtime/ext/ext_function.h>
#include <runtime/base/builtin_functions.h>
#include <runtime/base/zend/zend_math.h>
#include <runtime/base/util/string_buffer.h>
#include <runtime/base/util/request_local.h>
#include <runtime/base/ini_setting.h>
#include <runtime/base/time/datetime.h>
#include <util/lock.h>
#include <util/compatibility.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <dirent.h>

using namespace std;

namespace HPHP {
IMPLEMENT_DEFAULT_EXTENSION(session);
///////////////////////////////////////////////////////////////////////////////

bool ini_on_update_save_handler(CStrRef value, void *p);
bool ini_on_update_serializer(CStrRef value, void *p);
bool ini_on_update_trans_sid(CStrRef value, void *p);
bool ini_on_update_save_dir(CStrRef value, void *p);

///////////////////////////////////////////////////////////////////////////////
// global data

class SessionModule;
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
  int64       m_entropy_length;
  std::string m_cache_limiter;
  int64       m_cookie_lifetime;
  std::string m_cookie_path;
  std::string m_cookie_domain;
  bool        m_cookie_secure;
  bool        m_cookie_httponly;

  SessionModule *m_mod;

  Status m_session_status;
  int64  m_gc_probability;
  int64  m_gc_divisor;
  int64  m_gc_maxlifetime;
  int    m_module_number;
  int64  m_cache_expire;

  std::string m_ps_open;
  std::string m_ps_close;
  std::string m_ps_read;
  std::string m_ps_write;
  std::string m_ps_destroy;
  std::string m_ps_gc;

  SessionSerializer *m_serializer;

  bool m_auto_start;
  bool m_use_cookies;
  bool m_use_only_cookies;
  bool m_use_trans_sid;   // contains the INI value of whether to use trans-sid
  bool m_apply_trans_sid; // whether to enable trans-sid for current request

  std::string m_hash_func;
  int64 m_hash_bits_per_character;

  int  m_send_cookie;
  int  m_define_sid;
  bool m_invalid_session_id;  /* allows the driver to report about an invalid
                                 session id and request id regeneration */

  Session()
    : m_entropy_length(0), m_cookie_lifetime(0), m_cookie_secure(false),
      m_cookie_httponly(false), m_mod(NULL), m_session_status(None),
      m_gc_probability(0), m_gc_divisor(0), m_gc_maxlifetime(0),
      m_module_number(0), m_cache_expire(0), m_serializer(NULL),
      m_auto_start(false), m_use_cookies(false), m_use_only_cookies(false),
      m_use_trans_sid(false), m_apply_trans_sid(false),
      m_hash_bits_per_character(0), m_send_cookie(0), m_define_sid(0),
      m_invalid_session_id(false) {
  }
};

class SessionRequestData : public RequestEventHandler, public Session {
public:
  SessionRequestData() : m_threadInited(false) {}

  virtual void requestInit() {
    if (!m_threadInited) {
      m_threadInited = true;
      threadInit();
    }
    m_id.reset();
    m_session_status = Session::None;
  }

  void requestShutdownImpl();
  virtual void requestShutdown() {
    f_session_write_close();
    requestShutdownImpl();
  }

public:
  bool m_threadInited;
  String m_id;

  void threadInit() {
    IniSetting::Bind("session.save_path",          "",
                     ini_on_update_save_dir,       &m_save_path);
    IniSetting::Bind("session.name",               "PHPSESSID",
                     ini_on_update_string,         &m_session_name);
    IniSetting::Bind("session.save_handler",       "files",
                     ini_on_update_save_handler);
    IniSetting::Bind("session.auto_start",         "0",
                     ini_on_update_bool,           &m_auto_start);
    IniSetting::Bind("session.gc_probability",     "1",
                     ini_on_update_long,           &m_gc_probability);
    IniSetting::Bind("session.gc_divisor",         "100",
                     ini_on_update_long,           &m_gc_divisor);
    IniSetting::Bind("session.gc_maxlifetime",     "1440",
                     ini_on_update_long,           &m_gc_maxlifetime);
    IniSetting::Bind("session.serialize_handler",  "php",
                     ini_on_update_serializer);
    IniSetting::Bind("session.cookie_lifetime",    "0",
                     ini_on_update_long,           &m_cookie_lifetime);
    IniSetting::Bind("session.cookie_path",        "/",
                     ini_on_update_string,         &m_cookie_path);
    IniSetting::Bind("session.cookie_domain",      "",
                     ini_on_update_string,         &m_cookie_domain);
    IniSetting::Bind("session.cookie_secure",      "",
                     ini_on_update_bool,           &m_cookie_secure);
    IniSetting::Bind("session.cookie_httponly",    "",
                     ini_on_update_bool,           &m_cookie_httponly);
    IniSetting::Bind("session.use_cookies",        "1",
                     ini_on_update_bool,           &m_use_cookies);
    IniSetting::Bind("session.use_only_cookies",   "1",
                     ini_on_update_bool,           &m_use_only_cookies);
    IniSetting::Bind("session.referer_check",      "",
                     ini_on_update_string,         &m_extern_referer_chk);
    IniSetting::Bind("session.entropy_file",       "",
                     ini_on_update_string,         &m_entropy_file);
    IniSetting::Bind("session.entropy_length",     "0",
                     ini_on_update_long,           &m_entropy_length);
    IniSetting::Bind("session.cache_limiter",      "nocache",
                     ini_on_update_string,         &m_cache_limiter);
    IniSetting::Bind("session.cache_expire",       "180",
                     ini_on_update_long,           &m_cache_expire);
    IniSetting::Bind("session.use_trans_sid",      "0",
                     ini_on_update_trans_sid);
    IniSetting::Bind("session.hash_function",      "0",
                     ini_on_update_string,         &m_hash_func);
    IniSetting::Bind("session.hash_bits_per_character", "4",
                     ini_on_update_long,           &m_hash_bits_per_character);
  }
};
IMPLEMENT_STATIC_REQUEST_LOCAL(SessionRequestData, s_session);
#define PS(name) s_session->m_ ## name

///////////////////////////////////////////////////////////////////////////////
// SessionModule

class SessionModule {
public:
  enum {
    md5,
    sha1,
  };

public:
  SessionModule(const char *name) : m_name(name) {
    RegisteredModules.push_back(this);
  }
  virtual ~SessionModule() {}

  const char *getName() const { return m_name;}

  virtual bool open(const char *save_path, const char *session_name) = 0;
  virtual bool close() = 0;
  virtual bool read(const char *key, String &value) = 0;
  virtual bool write(const char *key, CStrRef value) = 0;
  virtual bool destroy(const char *key) = 0;
  virtual bool gc(int maxlifetime, int *nrdels) = 0;
  virtual String create_sid();

public:
  static SessionModule *Find(const char *name) {
    for (unsigned int i = 0; i < RegisteredModules.size(); i++) {
      SessionModule *mod = RegisteredModules[i];
      if (mod && strcasecmp(name, mod->m_name) == 0) {
        return mod;
      }
    }
    return NULL;
  }

private:
  static std::vector<SessionModule*> RegisteredModules;

  const char *m_name;
};
std::vector<SessionModule*> SessionModule::RegisteredModules;

void SessionRequestData::requestShutdownImpl() {
  if (m_mod) {
    try {
      m_mod->close();
    } catch (...) {}
  }
  m_id.reset();
}

/*
 * Note that we cannot use the BASE64 alphabet here, because
 * it contains "/" and "+": both are unacceptable for simple inclusion
 * into URLs.
 */
static char hexconvtab[] =
  "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ,-";

/* returns a pointer to the byte after the last valid character in out */
static void bin_to_readable(CStrRef in, StringBuffer &out, char nbits) {
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

String SessionModule::create_sid() {
  SystemGlobals *g = (SystemGlobals*)get_global_variables();
  String remote_addr = g->gv__SERVER["REMOTE_ADDR"].toString();

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

  Variant context = f_hash_init(PS(hash_func));
  if (same(context, false)) {
    Logger::Error("Invalid session hash function: %s", PS(hash_func).c_str());
    return String();
  }
  if (!f_hash_update(context, buf.detach())) {
    Logger::Error("hash_update() failed");
    return String();
  }

  if (PS(entropy_length) > 0) {
    int fd = open(PS(entropy_file).c_str(), O_RDONLY);
    if (fd >= 0) {
      unsigned char rbuf[2048];
      int n;
      int to_read = PS(entropy_length);
      while (to_read > 0) {
        n = ::read(fd, rbuf, (to_read < (int)sizeof(rbuf) ?
                              to_read : (int)sizeof(buf)));
        if (n <= 0) break;
        if (!f_hash_update(context,
                           String((const char *)rbuf, n, AttachLiteral))) {
          Logger::Error("hash_update() failed");
          ::close(fd);
          return String();
        }
        to_read -= n;
      }
      ::close(fd);
    }
  }

  String hashed = f_hash_final(context);

  if (PS(hash_bits_per_character) < 4 || PS(hash_bits_per_character) > 6) {
    PS(hash_bits_per_character) = 4;
    raise_warning("The ini setting hash_bits_per_character is out of range "
                  "(should be 4, 5, or 6) - using 4 for now");
  }

  StringBuffer readable;
  bin_to_readable(hashed, readable, PS(hash_bits_per_character));
  return readable;
}

///////////////////////////////////////////////////////////////////////////////
// FileSessionModule

class FileSessionModule : public SessionModule {
public:
  FileSessionModule()
    : SessionModule("files"), m_fd(-1), m_dirdepth(0), m_st_size(0),
      m_filemode(0600) {
  }

  virtual bool open(const char *save_path, const char *session_name) {
    Lock lock(m_mutex);

    String tmpdir;
    if (*save_path == '\0') {
      tmpdir = f_sys_get_temp_dir();
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
    return true;
  }

  virtual bool close() {
    Lock lock(m_mutex);
    closeImpl();
    m_lastkey.clear();
    m_basedir.clear();
    return true;
  }

  virtual bool read(const char *key, String &value) {
    Lock lock(m_mutex);
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

    char *val = (char*)malloc(m_st_size + 1);
    val[m_st_size] = '\0';

#if defined(HAVE_PREAD)
    long n = pread(m_fd, val, m_st_size, 0);
#else
    lseek(m_fd, 0, SEEK_SET);
    long n = ::read(m_fd, val, m_st_size);
#endif

    if (n != (int)m_st_size) {
      free(val);
      if (n == -1) {
        raise_warning("read failed: %s (%d)", strerror(errno), errno);
      } else {
        raise_warning("read returned less bytes than requested");
      }
      return false;
    }

    value = String(val, n, AttachString);
    return true;
  }

  virtual bool write(const char *key, CStrRef value) {
    Lock lock(m_mutex);
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
        raise_warning("truncate failed: %s (%d)", strerror(errno), errno);
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
        raise_warning("write failed: %s (%d)", strerror(errno), errno);
      } else {
        raise_warning("write wrote less bytes than requested");
      }
      return false;
    }

    return true;
  }

  virtual bool destroy(const char *key) {
    Lock lock(m_mutex);
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

  virtual bool gc(int maxlifetime, int *nrdels) {
    Lock lock(m_mutex);
    /* we don't perform any cleanup, if dirdepth is larger than 0.
       we return true, since all cleanup should be handled by
       an external entity (i.e. find -ctime x | xargs rm) */
    if (m_dirdepth == 0) {
      *nrdels = CleanupDir(m_basedir.c_str(), maxlifetime);
    }
    return true;
  }

private:
  Mutex m_mutex;
  int m_fd;
  string m_lastkey;
  string m_basedir;
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
                        m_fd, strerror(errno), errno);
        }
#endif
      } else {
        raise_warning("open(%s, O_RDWR) failed: %s (%d)", buf,
                      strerror(errno), errno);
      }
    }
  }

  static int CleanupDir(const char *dirname, int maxlifetime) {
    DIR *dir = opendir(dirname);
    if (!dir) {
      raise_notice("ps_files_cleanup_dir: opendir(%s) failed: %s (%d)",
                   dirname, strerror(errno), errno);
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
static FileSessionModule s_file_session_module;

///////////////////////////////////////////////////////////////////////////////
// UserSessionModule

class UserSessionModule : public SessionModule {
public:
  UserSessionModule() : SessionModule("user") {}

  virtual bool open(const char *save_path, const char *session_name) {
    return f_call_user_func_array
      (String(PS(ps_open)),
       CREATE_VECTOR2(String(save_path, CopyString),
                      String(session_name, CopyString)));
  }

  virtual bool close() {
    return f_call_user_func_array
      (String(PS(ps_close)),
       Array::Create());
  }

  virtual bool read(const char *key, String &value) {
    Variant ret = f_call_user_func_array
      (String(PS(ps_read)),
       CREATE_VECTOR1(String(key, CopyString)));
    if (ret.isString()) {
      value = ret.toString();
      return true;
    }
    return false;
  }

  virtual bool write(const char *key, CStrRef value) {
    return f_call_user_func_array
      (String(PS(ps_write)),
       CREATE_VECTOR2(String(key, CopyString), value));
  }

  virtual bool destroy(const char *key) {
    return f_call_user_func_array
      (String(PS(ps_destroy)),
       CREATE_VECTOR1(String(key, CopyString)));
  }

  virtual bool gc(int maxlifetime, int *nrdels) {
    return f_call_user_func_array
      (String(PS(ps_gc)),
       CREATE_VECTOR1((int64)maxlifetime));
  }
};

///////////////////////////////////////////////////////////////////////////////
// session serializers

class SessionSerializer {
public:
  SessionSerializer(const char *name) : m_name(name) {
    RegisteredSerializers.push_back(this);
  }
  virtual ~SessionSerializer() {}

  virtual String encode() = 0;
  virtual bool decode(CStrRef value) = 0;

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
    SystemGlobals *g = (SystemGlobals*)get_global_variables();
    for (ArrayIter iter(g->gv__SESSION); iter; ++iter) {
      Variant key = iter.first();
      if (key.isString()) {
        String skey = key.toString();
        if (skey.size() <= PS_BIN_MAX) {
          buf.append((unsigned char)skey.size());
          buf.append(skey);
          buf.append(f_serialize(iter.second()));
        }
      } else {
        raise_notice("Skipping numeric key %lld", key.toInt64());
      }
    }
    return buf;
  }

  virtual bool decode(CStrRef value) {
    const char *endptr = value.data() + value.size();
    SystemGlobals *g = (SystemGlobals*)get_global_variables();
    for (const char *p = value.data(); p < endptr; ) {
      int namelen = ((unsigned char)(*p)) & (~PS_BIN_UNDEF);
      if (namelen < 0 || namelen > PS_BIN_MAX || (p + namelen) >= endptr) {
        return false;
      }

      int has_value = *p & PS_BIN_UNDEF ? 0 : 1;
      String key(p + 1, namelen, CopyString);
      p += namelen + 1;
      if (has_value) {
        g->gv__SESSION.set(key, f_unserialize(String(p, AttachLiteral)));
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
    SystemGlobals *g = (SystemGlobals*)get_global_variables();
    for (ArrayIter iter(g->gv__SESSION); iter; ++iter) {
      Variant key = iter.first();
      if (key.isString()) {
        String skey = key.toString();
        buf.append(skey);
        if (skey.find(PS_DELIMITER) >= 0) {
          return String();
        }
        buf.append(PS_DELIMITER);
        buf.append(f_serialize(iter.second()));
      } else {
        raise_notice("Skipping numeric key %lld", key.toInt64());
      }
    }
    return buf;
  }

  virtual bool decode(CStrRef value) {
    const char *p = value.data();
    const char *endptr = value.data() + value.size();
    SystemGlobals *g = (SystemGlobals*)get_global_variables();
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
        g->gv__SESSION.set(key, f_unserialize(String(q, AttachLiteral)));
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

bool ini_on_update_save_handler(CStrRef value, void *p) {
  SESSION_CHECK_ACTIVE_STATE;
  PS(mod) = SessionModule::Find(value.data());
  return true;
}

bool ini_on_update_serializer(CStrRef value, void *p) {
  SESSION_CHECK_ACTIVE_STATE;
  PS(serializer) = SessionSerializer::Find(value.data());
  return true;
}

bool ini_on_update_trans_sid(CStrRef value, void *p) {
  SESSION_CHECK_ACTIVE_STATE;
  if (!strncasecmp(value.data(), "on", sizeof("on"))) {
    PS(use_trans_sid) = true;
  } else {
    PS(use_trans_sid) = value.toBoolean();
  }
  return true;
}

bool ini_on_update_save_dir(CStrRef value, void *p) {
  if (value.find('\0') >= 0) {
    return false;
  }
  const char *path = value.data() + value.rfind(';') + 1;
  if (File::TranslatePath(path).empty()) {
    return false;
  }
  return ini_on_update_string(value, p);
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
  s_session->requestInit();

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

static void php_session_decode(CStrRef value) {
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
  SystemGlobals *g = (SystemGlobals*)get_global_variables();
  g->gv__SESSION.reset();

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
  if (PS(mod)) {
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
  if (PS(mod)) {
    PS(mod)->close();
  }
}

///////////////////////////////////////////////////////////////////////////////
// Cookie Management

#define COOKIE_SET_COOKIE "Set-Cookie: "
#define COOKIE_EXPIRES    "; expires="
#define COOKIE_PATH       "; path="
#define COOKIE_DOMAIN     "; domain="
#define COOKIE_SECURE     "; secure"
#define COOKIE_HTTPONLY   "; HttpOnly"

static void php_session_send_cookie() {
  Transport *transport = g_context->getTransport();
  if (!transport) return;

  if (transport->headersSent()) {
    raise_warning("Cannot send session cookie - headers already sent");
    return;
  }

  /* URL encode session_name and id because they might be user supplied */
  String session_name =
    StringUtil::UrlEncode(String(PS(session_name).c_str()));
  String id = StringUtil::UrlEncode(PS(id));

  StringBuffer ncookie;
  ncookie.append(COOKIE_SET_COOKIE);
  ncookie.append(session_name);
  ncookie.append('=');
  ncookie.append(id);

  if (PS(cookie_lifetime) > 0) {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    time_t t = tv.tv_sec + PS(cookie_lifetime);
    if (t > 0) {
      ncookie.append(COOKIE_EXPIRES);
      ncookie.append(DateTime(t).toString(DateTime::Cookie));
    }
  }
  if (!PS(cookie_path).empty()) {
    ncookie.append(COOKIE_PATH);
    ncookie.append(PS(cookie_path));
  }
  if (!PS(cookie_domain).empty()) {
    ncookie.append(COOKIE_DOMAIN);
    ncookie.append(PS(cookie_domain));
  }
  if (PS(cookie_secure)) {
    ncookie.append(COOKIE_SECURE);
  }
  if (PS(cookie_httponly)) {
    ncookie.append(COOKIE_HTTPONLY);
  }

  transport->addHeader(ncookie.detach());
}

static void php_session_reset_id() {
  if (PS(use_cookies) && PS(send_cookie)) {
    php_session_send_cookie();
    PS(send_cookie) = 0;
  }

  SystemGlobals *g = (SystemGlobals*)get_global_variables();
  if (PS(define_sid)) {
    StringBuffer var;
    var.append(String(PS(session_name)));
    var.append('=');
    var.append(PS(id));
    g->declareConstant("SID", g->k_SID, var.detach());
  } else {
    g->declareConstant("SID", g->k_SID, String(""));
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

static inline void last_modified() {
  SystemGlobals *g = (SystemGlobals*)get_global_variables();
  String path = g->gv__SERVER["PATH_TRANSLATED"].toString();
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

  snprintf(buf, sizeof(buf) , "Cache-Control: public, max-age=%lld",
           PS(cache_expire) * 60); /* SAFE */
  ADD_HEADER(buf);

  last_modified();
}

CACHE_LIMITER_FUNC(private_no_expire) {
  char buf[MAX_STR + 1];

  snprintf(buf, sizeof(buf), "Cache-Control: private, max-age=%lld, "
           "pre-check=%lld", PS(cache_expire) * 60,
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

void f_session_set_cookie_params(int64 lifetime,
                                 CStrRef path /* = null_string */,
                                 CStrRef domain /* = null_string */,
                                 CVarRef secure /* = null */,
                                 CVarRef httponly /* = null */) {
  if (PS(use_cookies)) {
    IniSetting::Set("session.cookie_lifetime", lifetime);
    if (!path.isNull()) {
      IniSetting::Set("session.cookie_path", path);
    }
    if (!domain.isNull()) {
      IniSetting::Set("session.cookie_domain", domain);
    }
    if (!secure.isNull()) {
      IniSetting::Set("session.cookie_secure", secure.toBoolean());
    }
    if (!httponly.isNull()) {
      IniSetting::Set("session.cookie_httponly", httponly.toBoolean());
    }
  }
}

Array f_session_get_cookie_params() {
  Array ret = Array::Create();
  ret.set("lifetime", PS(cookie_lifetime));
  ret.set("path",     String(PS(cookie_path)));
  ret.set("domain",   String(PS(cookie_domain)));
  ret.set("secure",   PS(cookie_secure));
  ret.set("httponly", PS(cookie_httponly));
  return ret;
}

String f_session_name(CStrRef newname /* = null_string */) {
  String oldname = String(PS(session_name));
  if (!newname.isNull()) {
    IniSetting::Set("session.name", newname);
  }
  return oldname;
}

Variant f_session_module_name(CStrRef newname /* = null_string */) {
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
    if (PS(mod)) {
      PS(mod)->close();
    }
    PS(mod) = NULL;

    IniSetting::Set("session.save_handler", newname);
  }

  return oldname;
}

static bool check_handler(const char *name, std::string &member) {
  if (!f_is_callable(name)) {
    raise_warning("Argument '%s' is not a valid callback", name);
    return false;
  }
  member = name;
  return true;
}

bool f_session_set_save_handler(CStrRef open, CStrRef close, CStrRef read,
                                CStrRef write, CStrRef destroy, CStrRef gc) {
  if (PS(session_status) != Session::None) {
    return false;
  }

  if (!check_handler(open.data(),    PS(ps_open)))    return false;
  if (!check_handler(close.data(),   PS(ps_close)))   return false;
  if (!check_handler(read.data(),    PS(ps_read)))    return false;
  if (!check_handler(write.data(),   PS(ps_write)))   return false;
  if (!check_handler(destroy.data(), PS(ps_destroy))) return false;
  if (!check_handler(gc.data(),      PS(ps_gc)))      return false;

  IniSetting::Set("session.save_handler", "user");
  return true;
}

String f_session_save_path(CStrRef newname /* = null_string */) {
  if (!newname.isNull()) {
    if (memchr(newname.data(), '\0', newname.size()) != NULL) {
      raise_warning("The save_path cannot contain NULL characters");
      return false;
    }
    IniSetting::Set("session.save_path", newname);
  }
  return String(PS(save_path));
}

String f_session_id(CStrRef newid /* = null_string */) {
  String ret = PS(id);
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

String f_session_cache_limiter(CStrRef new_cache_limiter /* = null_string */) {
  String ret(PS(cache_limiter));
  if (!new_cache_limiter.isNull()) {
    IniSetting::Set("session.cache_limiter", new_cache_limiter);
  }
  return ret;
}

int64 f_session_cache_expire(CStrRef new_cache_expire /* = null_string */) {
  int64 ret = PS(cache_expire);
  if (!new_cache_expire.isNull()) {
    IniSetting::Set("session.cache_expire", new_cache_expire.toInt64());
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

bool f_session_decode(CStrRef data) {
  if (PS(session_status) != Session::None) {
    php_session_decode(data);
    return true;
  }
  return false;
}

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
    ASSERT(PS(session_status) == Session::None);
    PS(define_sid) = 1;
    PS(send_cookie) = 1;
  }

  /*
   * Cookies are preferred, because initially
   * cookie and get variables will be available.
   */
  SystemGlobals *g = (SystemGlobals*)get_global_variables();
  if (PS(id).empty()) {
    if (PS(use_cookies) && g->gv__COOKIE.toArray().exists(PS(session_name))) {
      PS(id) = g->gv__COOKIE[PS(session_name)].toString();
      PS(apply_trans_sid) = 0;
      PS(send_cookie) = 0;
      PS(define_sid) = 0;
    }

    if (!PS(use_only_cookies) && !PS(id) &&
        g->gv__GET.toArray().exists(PS(session_name))) {
      PS(id) = g->gv__GET[PS(session_name)].toString();
      PS(send_cookie) = 0;
    }

    if (!PS(use_only_cookies) && !PS(id) &&
        g->gv__POST.toArray().exists(PS(session_name))) {
      PS(id) = g->gv__POST[PS(session_name)].toString();
      PS(send_cookie) = 0;
    }
  }

  int lensess = PS(session_name).size();

  /* check the REQUEST_URI symbol for a string of the form
     '<session-name>=<session-id>' to allow URLs of the form
     http://yoursite/<session-name>=<session-id>/script.php */
  if (!PS(use_only_cookies) && PS(id).empty()) {
    value = g->gv__SERVER["REQUEST_URI"].toString();
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
    value = g->gv__SERVER["HTTP_REFERER"].toString();
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

  if (PS(mod) && PS(gc_probability) > 0) {
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
  s_session->requestInit();

  return retval;
}

Variant f_session_unset() {
  if (PS(session_status) == Session::None) {
    return false;
  }
  SystemGlobals *g = (SystemGlobals*)get_global_variables();
  g->gv__SESSION.reset();
  return null;
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

bool f_session_register(int _argc, CVarRef var_names,
                        CArrRef _argv /* = null_array */) {
  throw NotSupportedException
    (__func__, "Deprecated as of PHP 5.3.0 and REMOVED as of PHP 6.0.0. "
     "Relying on this feature is highly discouraged.");
}

bool f_session_unregister(CStrRef varname) {
  throw NotSupportedException
    (__func__, "Deprecated as of PHP 5.3.0 and REMOVED as of PHP 6.0.0. "
     "Relying on this feature is highly discouraged.");
}

bool f_session_is_registered(CStrRef varname) {
  throw NotSupportedException
    (__func__, "Deprecated as of PHP 5.3.0 and REMOVED as of PHP 6.0.0. "
     "Relying on this feature is highly discouraged.");
}

///////////////////////////////////////////////////////////////////////////////
}
