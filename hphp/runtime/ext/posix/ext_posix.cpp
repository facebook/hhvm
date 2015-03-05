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
#include "hphp/runtime/ext/posix/ext_posix.h"

#include <memory>

#include <sys/times.h>
#include <sys/time.h>
#include <sys/utsname.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/resource.h>
#ifdef __FreeBSD__
#include <sys/param.h>
#endif
#include <unistd.h>
#include <pwd.h>

#include <folly/String.h>

#include "hphp/runtime/base/array-init.h"
#include "hphp/runtime/base/file.h"

namespace HPHP {

#define DEFINE_POSIX_CONSTANT(name)                                            \
  const int64_t k_POSIX_##name = name;                                         \
  static const StaticString s_POSIX_##name("POSIX_"#name)                      \

DEFINE_POSIX_CONSTANT(S_IFMT);
DEFINE_POSIX_CONSTANT(S_IFSOCK);
DEFINE_POSIX_CONSTANT(S_IFLNK);
DEFINE_POSIX_CONSTANT(S_IFREG);
DEFINE_POSIX_CONSTANT(S_IFBLK);
DEFINE_POSIX_CONSTANT(S_IFDIR);
DEFINE_POSIX_CONSTANT(S_IFCHR);
DEFINE_POSIX_CONSTANT(S_IFIFO);
DEFINE_POSIX_CONSTANT(S_ISUID);
DEFINE_POSIX_CONSTANT(S_ISGID);
DEFINE_POSIX_CONSTANT(S_ISVTX);
DEFINE_POSIX_CONSTANT(S_IRWXU);
DEFINE_POSIX_CONSTANT(S_IRUSR);
DEFINE_POSIX_CONSTANT(S_IWUSR);
DEFINE_POSIX_CONSTANT(S_IXUSR);
DEFINE_POSIX_CONSTANT(S_IRWXG);
DEFINE_POSIX_CONSTANT(S_IRGRP);
DEFINE_POSIX_CONSTANT(S_IWGRP);
DEFINE_POSIX_CONSTANT(S_IXGRP);
DEFINE_POSIX_CONSTANT(S_IRWXO);
DEFINE_POSIX_CONSTANT(S_IROTH);
DEFINE_POSIX_CONSTANT(S_IWOTH);
DEFINE_POSIX_CONSTANT(S_IXOTH);
DEFINE_POSIX_CONSTANT(F_OK);
DEFINE_POSIX_CONSTANT(X_OK);
DEFINE_POSIX_CONSTANT(W_OK);
DEFINE_POSIX_CONSTANT(R_OK);

///////////////////////////////////////////////////////////////////////////////

#define REGISTER_POSIX_CONSTANT(name)                                          \
  Native::registerConstant<KindOfInt64>(s_POSIX_##name.get(), k_POSIX_##name)  \

static class POSIXExtension final : public Extension {
public:
  POSIXExtension() : Extension("posix", NO_EXTENSION_VERSION_YET) {}
  void moduleInit() override {
    REGISTER_POSIX_CONSTANT(S_IFMT);
    REGISTER_POSIX_CONSTANT(S_IFSOCK);
    REGISTER_POSIX_CONSTANT(S_IFLNK);
    REGISTER_POSIX_CONSTANT(S_IFREG);
    REGISTER_POSIX_CONSTANT(S_IFBLK);
    REGISTER_POSIX_CONSTANT(S_IFDIR);
    REGISTER_POSIX_CONSTANT(S_IFCHR);
    REGISTER_POSIX_CONSTANT(S_IFIFO);
    REGISTER_POSIX_CONSTANT(S_ISUID);
    REGISTER_POSIX_CONSTANT(S_ISGID);
    REGISTER_POSIX_CONSTANT(S_ISVTX);
    REGISTER_POSIX_CONSTANT(S_IRWXU);
    REGISTER_POSIX_CONSTANT(S_IRUSR);
    REGISTER_POSIX_CONSTANT(S_IWUSR);
    REGISTER_POSIX_CONSTANT(S_IXUSR);
    REGISTER_POSIX_CONSTANT(S_IRWXG);
    REGISTER_POSIX_CONSTANT(S_IRGRP);
    REGISTER_POSIX_CONSTANT(S_IWGRP);
    REGISTER_POSIX_CONSTANT(S_IXGRP);
    REGISTER_POSIX_CONSTANT(S_IRWXO);
    REGISTER_POSIX_CONSTANT(S_IROTH);
    REGISTER_POSIX_CONSTANT(S_IWOTH);
    REGISTER_POSIX_CONSTANT(S_IXOTH);
    REGISTER_POSIX_CONSTANT(F_OK);
    REGISTER_POSIX_CONSTANT(X_OK);
    REGISTER_POSIX_CONSTANT(W_OK);
    REGISTER_POSIX_CONSTANT(R_OK);

    HHVM_FE(posix_access);
    HHVM_FE(posix_ctermid);
    HHVM_FE(posix_get_last_error);
    HHVM_FE(posix_errno);
    HHVM_FE(posix_getcwd);
    HHVM_FE(posix_getegid);
    HHVM_FE(posix_geteuid);
    HHVM_FE(posix_getgid);
    HHVM_FE(posix_getgrgid);
    HHVM_FE(posix_getgrnam);
    HHVM_FE(posix_getgroups);
    HHVM_FE(posix_getlogin);
    HHVM_FE(posix_getpgid);
    HHVM_FE(posix_getpgrp);
    HHVM_FE(posix_getpid);
    HHVM_FE(posix_getppid);
    HHVM_FE(posix_getpwnam);
    HHVM_FE(posix_getpwuid);
    HHVM_FE(posix_getrlimit);
    HHVM_FE(posix_getsid);
    HHVM_FE(posix_getuid);
    HHVM_FE(posix_initgroups);
    HHVM_FE(posix_isatty);
    HHVM_FE(posix_kill);
    HHVM_FE(posix_mkfifo);
    HHVM_FE(posix_mknod);
    HHVM_FE(posix_setegid);
    HHVM_FE(posix_seteuid);
    HHVM_FE(posix_setgid);
    HHVM_FE(posix_setpgid);
    HHVM_FE(posix_setsid);
    HHVM_FE(posix_setuid);
    HHVM_FE(posix_strerror);
    HHVM_FE(posix_times);
    HHVM_FE(posix_ttyname);
    HHVM_FE(posix_uname);

    loadSystemlib();
  }
} s_posix_extension;

///////////////////////////////////////////////////////////////////////////////

bool HHVM_FUNCTION(posix_access,
                   const String& file,
                   int mode /* = 0 */) {
  String path = File::TranslatePath(file);
  if (path.empty()) {
    return false;
  }
  return !access(path.data(), mode);
}

String HHVM_FUNCTION(posix_ctermid) {
  String s = String(L_ctermid, ReserveString);
  char *buffer = s.mutableData();
  ctermid(buffer);
  s.setSize(strlen(buffer));
  return s;
}

int64_t HHVM_FUNCTION(posix_get_last_error) {
  return errno;
}

int64_t HHVM_FUNCTION(posix_errno) {
  return errno;
}

String HHVM_FUNCTION(posix_getcwd) {
  String s = String(PATH_MAX, ReserveString);
  char *buffer = s.mutableData();
  if (getcwd(buffer, PATH_MAX) == NULL) {
    return "/";
  }
  s.setSize(strlen(buffer));
  return s;
}

int64_t HHVM_FUNCTION(posix_getegid) {
  return getegid();
}

int64_t HHVM_FUNCTION(posix_geteuid) {
  return geteuid();
}

int64_t HHVM_FUNCTION(posix_getgid) {
  return getgid();
}

const StaticString
  s_name("name"),
  s_passwd("passwd"),
  s_members("members"),
  s_uid("uid"),
  s_gid("gid"),
  s_gecos("gecos"),
  s_dir("dir"),
  s_shell("shell");

static Variant php_posix_group_to_array(int gid,
                   const String& gname = null_variant.toString()) {
  // Don't pass a gid *and* a gname to this.
  assert((gid <  0) || gname.size() == 0);

  if ((gid < 0) && (gname.size() == 0)) {
    return false;
  }

  int grbuflen = sysconf(_SC_GETGR_R_SIZE_MAX);
  if (grbuflen < 1) {
    return false;
  }

  std::unique_ptr<char[]> grbuf(new char[grbuflen]);
  struct group gr;
  struct group *retgrptr = NULL;

  // If we somehow reach this point and both gname and gid were
  // passed, then the gid values will override the gname values,
  // but it will otherwise function just fine.
  // The assert() clause above should prevent that, however.
  if ((gname.size() > 0) &&
      (getgrnam_r(gname.data(), &gr, grbuf.get(), grbuflen, &retgrptr) != 0 ||
      retgrptr == nullptr)) {
    return false;
  } else if ((gid >= 0) &&
      (getgrgid_r(gid, &gr, grbuf.get(), grbuflen, &retgrptr) != 0 ||
      retgrptr == nullptr)) {
    return false;
  }

  Array members = Array::Create();
  for (int count=0; gr.gr_mem[count] != NULL; count++) {
    members.append(String(gr.gr_mem[count], CopyString));
  }

  return make_map_array(
    s_name, String(gr.gr_name, CopyString),
    s_passwd, String(gr.gr_passwd, CopyString),
    s_members, members,
    s_gid, (int)gr.gr_gid
  );
}

Variant HHVM_FUNCTION(posix_getgrgid,
                      int gid) {
  return php_posix_group_to_array(gid);
}

Variant HHVM_FUNCTION(posix_getgrnam,
                      const String& name) {
  return php_posix_group_to_array(-1, name.data());
}

Variant HHVM_FUNCTION(posix_getgroups) {
  gid_t gidlist[NGROUPS_MAX];
  int result = getgroups(NGROUPS_MAX, gidlist);
  if (result < 0) {
    return false;
  }

  Array ret;
  for (int i = 0; i < result; i++) {
    ret.append((int)gidlist[i]);
  }
  return ret;
}

Variant HHVM_FUNCTION(posix_getlogin) {
#if !defined(__APPLE__) && !defined(__FreeBSD__)
  char buf[L_cuserid];
#else
  char buf[MAXLOGNAME];
#endif
  if (!getlogin_r(buf, sizeof(buf) - 1)) {
    return String(buf, CopyString);
  }
  return false;
}

Variant HHVM_FUNCTION(posix_getpgid,
                      int pid) {
  int ret = getpgid(pid);
  if (ret < 0) return false;
  return ret;
}

int64_t HHVM_FUNCTION(posix_getpgrp) {
  return getpgrp();
}

int64_t HHVM_FUNCTION(posix_getpid) {
  return getpid();
}

int64_t HHVM_FUNCTION(posix_getppid) {
  return getppid();
}

static Variant php_posix_passwd_to_array(int uid,
                   const String& name = null_variant.toString()) {
  // Don't pass a uid *and* a name to this.
  assert((uid <  0) || name.size() == 0);

  if ((uid < 0) && name.size() == 0) {
    return false;
  }

  int pwbuflen = sysconf(_SC_GETPW_R_SIZE_MAX);
  if (pwbuflen < 1) {
    return false;
  }

  std::unique_ptr<char[]> pwbuf(new char[pwbuflen]);
  struct passwd pw;
  struct passwd *retpwptr = NULL;

  // If we somehow reach this point and both name and uid were
  // passed, then the uid values will override the name values,
  // but it will otherwise function just fine.
  // The assert() clauses above should prevent that, however.
  if ((name.size() > 0) &&
      getpwnam_r(name.data(), &pw, pwbuf.get(), pwbuflen, &retpwptr)) {
    return false;
  } else if ((uid >= 0) &&
      getpwuid_r(uid, &pw, pwbuf.get(), pwbuflen, &retpwptr)) {
    return false;
  }

  if (!retpwptr) return false;

  return make_map_array(
    s_name,   String(pw.pw_name,   CopyString),
    s_passwd, String(pw.pw_passwd, CopyString),
    s_uid,    (int)pw.pw_uid,
    s_gid,    (int)pw.pw_gid,
    s_gecos,  String(pw.pw_gecos,  CopyString),
    s_dir,    String(pw.pw_dir,    CopyString),
    s_shell,  String(pw.pw_shell,  CopyString)
  );
}

Variant HHVM_FUNCTION(posix_getpwnam,
                      const String& username) {
  return php_posix_passwd_to_array(-1, username);
}

Variant HHVM_FUNCTION(posix_getpwuid,
                      int uid) {
  return php_posix_passwd_to_array(uid);
}

static bool posix_addlimit(int limit, const char *name, Array &ret) {
  char hard[80]; snprintf(hard, 80, "hard %s", name);
  char soft[80]; snprintf(soft, 80, "soft %s", name);

  struct rlimit rl;
  int result = getrlimit(limit, &rl);
  if (result < 0) {
    return false;
  }

  String softStr(soft, CopyString);
  String hardStr(hard, CopyString);

  if (rl.rlim_cur == RLIM_INFINITY) {
    ret.set(softStr, "unlimited");
  } else {
    ret.set(softStr, (int)rl.rlim_cur);
  }

  if (rl.rlim_max == RLIM_INFINITY) {
    ret.set(hardStr, "unlimited");
  } else {
    ret.set(hardStr, (int)rl.rlim_max);
  }

  return true;
}

static struct limitlist {
  int limit;
  const char *name;
} limits[] = {
  { RLIMIT_CORE,    "core" },
  { RLIMIT_DATA,    "data" },
  { RLIMIT_STACK,   "stack" },
  //{ RLIMIT_VMEM,    "virtualmem" },
  { RLIMIT_AS,      "totalmem" },
#ifdef RLIMIT_RSS
  { RLIMIT_RSS,     "rss" },
#endif
#ifdef RLIMIT_NPROC
  { RLIMIT_NPROC,   "maxproc" },
#endif
#ifdef RLIMIT_MEMLOCK
  { RLIMIT_MEMLOCK, "memlock" },
#endif
  { RLIMIT_CPU,     "cpu" },
  { RLIMIT_FSIZE,   "filesize" },
  { RLIMIT_NOFILE,  "openfiles" },
  { 0, NULL }
};

Variant HHVM_FUNCTION(posix_getrlimit) {
  Array ret;
  for (struct limitlist *l = limits; l->name; l++) {
    if (!posix_addlimit(l->limit, l->name, ret)) {
      return false;
    }
  }
  return ret;
}

Variant HHVM_FUNCTION(posix_getsid,
                      int pid) {
  int ret = getsid(pid);
  if (ret < 0) return false;
  return ret;
}

int64_t HHVM_FUNCTION(posix_getuid) {
  return getuid();
}

bool HHVM_FUNCTION(posix_initgroups,
                   const String& name,
                   int base_group_id) {
  if (name.empty()) return false;
  return !initgroups(name.data(), base_group_id);
}

static int php_posix_get_fd(const Variant& fd) {
  int nfd;
  if (fd.isResource()) {
    auto f = cast<File>(fd);
    if (!f) {
      return false;
    }
    nfd = f->fd();
  } else {
    nfd = fd.toInt32();
  }
  return nfd;
}

bool HHVM_FUNCTION(posix_isatty,
                   const Variant& fd) {
  return isatty(php_posix_get_fd(fd));
}

bool HHVM_FUNCTION(posix_kill,
                   int pid,
                   int sig) {
  return kill(pid, sig) >= 0;
}

bool HHVM_FUNCTION(posix_mkfifo,
                   const String& pathname,
                   int mode) {
  return mkfifo(pathname.data(), mode) >= 0;
}

bool HHVM_FUNCTION(posix_mknod,
                   const String& pathname,
                   int mode,
                   int major /* = 0 */,
                   int minor /* = 0 */) {
  dev_t php_dev = 0;
  if ((mode & S_IFCHR) || (mode & S_IFBLK)) {
    if (major == 0 && minor == 0) {
      raise_warning("For S_IFCHR and S_IFBLK you need to pass "
                      "a major device kernel identifier");
      return false;
    }
    if (major == 0) {
      raise_warning("Expects argument 3 to be non-zero for "
                      "POSIX_S_IFCHR and POSIX_S_IFBLK");
      return false;
    }
    php_dev = makedev(major, minor);
  }

  return mknod(pathname.data(), mode, php_dev) >= 0;
}

bool HHVM_FUNCTION(posix_setegid,
                   int gid) {
  return setegid(gid);
}

bool HHVM_FUNCTION(posix_seteuid,
                   int uid) {
  return seteuid(uid);
}

bool HHVM_FUNCTION(posix_setgid,
                   int gid) {
  return setgid(gid);
}

bool HHVM_FUNCTION(posix_setpgid,
                   int pid,
                   int pgid) {
  return setpgid(pid, pgid) >= 0;
}

int64_t HHVM_FUNCTION(posix_setsid) {
  return setsid();
}

bool HHVM_FUNCTION(posix_setuid,
                   int uid) {
  return setuid(uid);
}

String HHVM_FUNCTION(posix_strerror,
                     int errnum) {
  return String(folly::errnoStr(errnum).toStdString());
}

const StaticString
  s_ticks("ticks"),
  s_utime("utime"),
  s_stime("stime"),
  s_cutime("cutime"),
  s_cstime("cstime");

Variant HHVM_FUNCTION(posix_times) {
  struct tms t;
  clock_t ticks = times(&t);
  if (ticks == -1) {
    return false;
  }

  return make_map_array(
    s_ticks,  (int)ticks,        /* clock ticks */
    s_utime,  (int)t.tms_utime,  /* user time */
    s_stime,  (int)t.tms_stime,  /* system time */
    s_cutime, (int)t.tms_cutime, /* user time of children */
    s_cstime, (int)t.tms_cstime  /* system time of children */
  );
}

Variant HHVM_FUNCTION(posix_ttyname,
                      const Variant& fd) {
  int ttyname_maxlen = sysconf(_SC_TTY_NAME_MAX);
  if (ttyname_maxlen <= 0) {
    return false;
  }

  String ttyname(ttyname_maxlen, ReserveString);
  char *p = ttyname.mutableData();
  if (ttyname_r(php_posix_get_fd(fd), p, ttyname_maxlen)) {
    return false;
  }
  ttyname.setSize(strlen(p));
  return ttyname;
}

const StaticString
  s_sysname("sysname"),
  s_nodename("nodename"),
  s_release("release"),
  s_version("version"),
  s_machine("machine"),
  s_domainname("domainname");

Variant HHVM_FUNCTION(posix_uname) {
  struct utsname u;
  if (uname(&u) < 0) {
    return false;
  }

  Array ret;
  ret.set(s_sysname,    String(u.sysname,    CopyString));
  ret.set(s_nodename,   String(u.nodename,   CopyString));
  ret.set(s_release,    String(u.release,    CopyString));
  ret.set(s_version,    String(u.version,    CopyString));
  ret.set(s_machine,    String(u.machine,    CopyString));
#if defined(_GNU_SOURCE)
  ret.set(s_domainname, String(u.domainname, CopyString));
#endif
  return ret;
}

///////////////////////////////////////////////////////////////////////////////
}
