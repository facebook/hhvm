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

#include "hphp/runtime/ext/ext_posix.h"
#include "hphp/runtime/base/file.h"
#include "folly/String.h"
#include <sys/times.h>
#include <sys/utsname.h>
#include <sys/types.h>
#ifdef __FreeBSD__
#include <sys/param.h>
#endif
#include <sys/time.h>
#include <pwd.h>
#include <memory>

namespace HPHP {
IMPLEMENT_DEFAULT_EXTENSION(posix);
///////////////////////////////////////////////////////////////////////////////

const int64_t k_POSIX_S_IFMT = S_IFMT;
const int64_t k_POSIX_S_IFSOCK = S_IFSOCK;
const int64_t k_POSIX_S_IFLNK = S_IFLNK;
const int64_t k_POSIX_S_IFREG = S_IFREG;
const int64_t k_POSIX_S_IFBLK = S_IFBLK;
const int64_t k_POSIX_S_IFDIR = S_IFDIR;
const int64_t k_POSIX_S_IFCHR = S_IFCHR;
const int64_t k_POSIX_S_IFIFO = S_IFIFO;
const int64_t k_POSIX_S_ISUID = S_ISUID;
const int64_t k_POSIX_S_ISGID = S_ISGID;
const int64_t k_POSIX_S_ISVTX = S_ISVTX;
const int64_t k_POSIX_S_IRWXU = S_IRWXU;
const int64_t k_POSIX_S_IRUSR = S_IRUSR;
const int64_t k_POSIX_S_IWUSR = S_IWUSR;
const int64_t k_POSIX_S_IXUSR = S_IXUSR;
const int64_t k_POSIX_S_IRWXG = S_IRWXG;
const int64_t k_POSIX_S_IRGRP = S_IRGRP;
const int64_t k_POSIX_S_IWGRP = S_IWGRP;
const int64_t k_POSIX_S_IXGRP = S_IXGRP;
const int64_t k_POSIX_S_IRWXO = S_IRWXO;
const int64_t k_POSIX_S_IROTH = S_IROTH;
const int64_t k_POSIX_S_IWOTH = S_IWOTH;
const int64_t k_POSIX_S_IXOTH = S_IXOTH;
const int64_t k_POSIX_F_OK = F_OK;
const int64_t k_POSIX_X_OK = X_OK;
const int64_t k_POSIX_W_OK = W_OK;
const int64_t k_POSIX_R_OK = R_OK;

bool f_posix_access(const String& file, int mode /* = 0 */) {
  String path = File::TranslatePath(file);
  if (path.empty()) {
    return false;
  }
  return !access(path.data(), mode);
}

String f_posix_ctermid() {
  String s = String(L_ctermid, ReserveString);
  char *buffer = s.bufferSlice().ptr;
  ctermid(buffer);
  return s.setSize(strlen(buffer));
}

int64_t f_posix_get_last_error() {
  return errno;
}

String f_posix_getcwd() {
  String s = String(PATH_MAX, ReserveString);
  char *buffer = s.bufferSlice().ptr;
  if (getcwd(buffer, PATH_MAX) == NULL) {
    return "/";
  }
  return s.setSize(strlen(buffer));
}

int64_t f_posix_getegid() {
  return getegid();
}

int64_t f_posix_geteuid() {
  return geteuid();
}

int64_t f_posix_getgid() {
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
  // passed, then the gid values will override the game values,
  // but it will otherwise function just fine.
  // The assert() clause above should prevent that, however.
  if ((gname.size() > 0) &&
      getgrnam_r(gname.data(), &gr, grbuf.get(), grbuflen, &retgrptr)) {
    return false;
  } else if ((gid >= 0) &&
      getgrgid_r(gid, &gr, grbuf.get(), grbuflen, &retgrptr)) {
    return false;
  }

  Array members;
  for (int count=0; gr.gr_mem[count] != NULL; count++) {
    members.append(String(gr.gr_mem[count], CopyString));
  }

  ArrayInit ret(4);
  ret.set(s_name, String(gr.gr_name, CopyString));
  ret.set(s_passwd, String(gr.gr_passwd, CopyString));
  ret.set(s_members, members);
  ret.set(s_gid, (int)gr.gr_gid);
  return ret.create();
}

Variant f_posix_getgrgid(int gid) {
  return php_posix_group_to_array(gid);
}

Variant f_posix_getgrnam(const String& name) {
  return php_posix_group_to_array(-1, name.data());
}

Variant f_posix_getgroups() {
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

Variant f_posix_getlogin() {
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

Variant f_posix_getpgid(int pid) {
  int ret = getpgid(pid);
  if (ret < 0) return false;
  return ret;
}

int64_t f_posix_getpgrp() {
  return getpgrp();
}

int64_t f_posix_getpid() {
  return getpid();
}

int64_t f_posix_getppid() {
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

  ArrayInit ret(7);
  ret.set(s_name,   String(pw.pw_name,   CopyString));
  ret.set(s_passwd, String(pw.pw_passwd, CopyString));
  ret.set(s_uid,    (int)pw.pw_uid);
  ret.set(s_gid,    (int)pw.pw_gid);
  ret.set(s_gecos,  String(pw.pw_gecos,  CopyString));
  ret.set(s_dir,    String(pw.pw_dir,    CopyString));
  ret.set(s_shell,  String(pw.pw_shell,  CopyString));
  return ret.create();
}

Variant f_posix_getpwnam(const String& username) {
  return php_posix_passwd_to_array(-1, username);
}

Variant f_posix_getpwuid(int uid) {
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
  { RLIMIT_RSS,     "rss" },
  { RLIMIT_NPROC,   "maxproc" },
  { RLIMIT_MEMLOCK, "memlock" },
  { RLIMIT_CPU,     "cpu" },
  { RLIMIT_FSIZE,   "filesize" },
  { RLIMIT_NOFILE,  "openfiles" },
  { 0, NULL }
};

Variant f_posix_getrlimit() {
  Array ret;
  for (struct limitlist *l = limits; l->name; l++) {
    if (!posix_addlimit(l->limit, l->name, ret)) {
      return false;
    }
  }
  return ret;
}

Variant f_posix_getsid(int pid) {
  int ret = getsid(pid);
  if (ret < 0) return false;
  return ret;
}

int64_t f_posix_getuid() {
  return getuid();
}

bool f_posix_initgroups(const String& name, int base_group_id) {
  if (name.empty()) return false;
  return !initgroups(name.data(), base_group_id);
}

static int php_posix_get_fd(CVarRef fd) {
  int nfd;
  if (fd.isResource()) {
    File *f = fd.toResource().getTyped<File>();
    if (!f) {
      return false;
    }
    nfd = f->fd();
  } else {
    nfd = fd.toInt32();
  }
  return nfd;
}

bool f_posix_isatty(CVarRef fd) {
  return isatty(php_posix_get_fd(fd));
}

bool f_posix_kill(int pid, int sig) {
  return kill(pid, sig) >= 0;
}

bool f_posix_mkfifo(const String& pathname, int mode) {
  return mkfifo(pathname.data(), mode) >= 0;
}

bool f_posix_mknod(const String& pathname, int mode, int major /* = 0 */,
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

bool f_posix_setegid(int gid) {
  return setegid(gid);
}

bool f_posix_seteuid(int uid) {
  return seteuid(uid);
}

bool f_posix_setgid(int gid) {
  return setgid(gid);
}

bool f_posix_setpgid(int pid, int pgid) {
  return setpgid(pid, pgid) >= 0;
}

int64_t f_posix_setsid() {
  return setsid();
}

bool f_posix_setuid(int uid) {
  return setuid(uid);
}

String f_posix_strerror(int errnum) {
  return String(folly::errnoStr(errnum).toStdString());
}

const StaticString
  s_ticks("ticks"),
  s_utime("utime"),
  s_stime("stime"),
  s_cutime("cutime"),
  s_cstime("cstime");

Variant f_posix_times() {
  struct tms t;
  clock_t ticks = times(&t);
  if (ticks == -1) {
    return false;
  }

  ArrayInit ret(5);
  ret.set(s_ticks,  (int)ticks);        /* clock ticks */
  ret.set(s_utime,  (int)t.tms_utime);  /* user time */
  ret.set(s_stime,  (int)t.tms_stime);  /* system time */
  ret.set(s_cutime, (int)t.tms_cutime); /* user time of children */
  ret.set(s_cstime, (int)t.tms_cstime); /* system time of children */
  return ret.create();
}

Variant f_posix_ttyname(CVarRef fd) {
  int ttyname_maxlen = sysconf(_SC_TTY_NAME_MAX);
  if (ttyname_maxlen <= 0) {
    return false;
  }

  String ttyname(ttyname_maxlen, ReserveString);
  char *p = ttyname.bufferSlice().ptr;
  if (ttyname_r(php_posix_get_fd(fd), p, ttyname_maxlen)) {
    return false;
  }
  return ttyname.setSize(strlen(p));
}

const StaticString
  s_sysname("sysname"),
  s_nodename("nodename"),
  s_release("release"),
  s_version("version"),
  s_machine("machine"),
  s_domainname("domainname");

Variant f_posix_uname() {
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
