/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010- Facebook, Inc. (http://www.facebook.com)         |
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

#include <runtime/ext/ext_posix.h>
#include <runtime/base/file/file.h>
#include <sys/times.h>
#include <sys/utsname.h>
#include <sys/types.h>
#include <sys/time.h>
#include <pwd.h>

namespace HPHP {
IMPLEMENT_DEFAULT_EXTENSION(posix);
///////////////////////////////////////////////////////////////////////////////

bool f_posix_access(CStrRef file, int mode /* = 0 */) {
  String path = File::TranslatePath(file);
  if (path.empty()) {
    return false;
  }
  return !access(path.data(), mode);
}

static Variant php_posix_group_to_array(struct group *g) {
  if (!g) {
    return false;
  }

  Array members;
  for (int count=0; g->gr_mem[count] != NULL; count++) {
    members.append(String(g->gr_mem[count], AttachLiteral));
  }

  Array ret;
  ret.set("name", String(g->gr_name, AttachLiteral));
  ret.set("passwd", String(g->gr_passwd, AttachLiteral));
  ret.set("members", members);
  ret.set("gid", (int)g->gr_gid);
  return ret;
}

Variant f_posix_getgrgid(int gid) {
  return php_posix_group_to_array(getgrgid(gid));
}

Variant f_posix_getgrnam(CStrRef name) {
  return php_posix_group_to_array(getgrnam(name.data()));
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

static Variant php_posix_passwd_to_array(struct passwd *pw) {
  if (!pw) {
    return false;
  }

  Array ret;
  ret.set("name",   String(pw->pw_name,   AttachLiteral));
  ret.set("passwd", String(pw->pw_passwd, AttachLiteral));
  ret.set("uid",    (int)pw->pw_uid);
  ret.set("gid",    (int)pw->pw_gid);
  ret.set("gecos",  String(pw->pw_gecos,  AttachLiteral));
  ret.set("dir",    String(pw->pw_dir,    AttachLiteral));
  ret.set("shell",  String(pw->pw_shell,  AttachLiteral));
  return ret;
}

Variant f_posix_getpwnam(CStrRef username) {
  return php_posix_passwd_to_array(getpwnam(username.data()));
}

Variant f_posix_getpwuid(int uid) {
  return php_posix_passwd_to_array(getpwuid(uid));
}

static bool posix_addlimit(int limit, const char *name, Array &ret) {
  char hard[80]; snprintf(hard, 80, "hard %s", name);
  char soft[80]; snprintf(soft, 80, "soft %s", name);

  struct rlimit rl;
  int result = getrlimit(limit, &rl);
  if (result < 0) {
    return false;
  }

  if (rl.rlim_cur == RLIM_INFINITY) {
    ret.set(soft, "unlimited");
  } else {
    ret.set(soft, (int)rl.rlim_cur);
  }

  if (rl.rlim_max == RLIM_INFINITY) {
    ret.set(hard, "unlimited");
  } else {
    ret.set(hard, (int)rl.rlim_max);
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

static int php_posix_get_fd(CVarRef fd) {
  int nfd;
  if (fd.isResource()) {
    File *f = fd.toObject().getTyped<File>();
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

bool f_posix_mknod(CStrRef pathname, int mode, int major /* = 0 */,
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

Variant f_posix_times() {
  struct tms t;
  clock_t ticks = times(&t);
  if (ticks == -1) {
    return false;
  }

  Array ret;
  ret.set("ticks",  (int)ticks);        /* clock ticks */
  ret.set("utime",  (int)t.tms_utime);  /* user time */
  ret.set("stime",  (int)t.tms_stime);  /* system time */
  ret.set("cutime", (int)t.tms_cutime); /* user time of children */
  ret.set("cstime", (int)t.tms_cstime); /* system time of children */
  return ret;
}

Variant f_posix_ttyname(CVarRef fd) {
  char *p = ttyname(php_posix_get_fd(fd));
  if (!p) {
    return false;
  }
  return String(p, CopyString);
}

Variant f_posix_uname() {
  struct utsname u;
  if (uname(&u) < 0) {
    return false;
  }

  Array ret;
  ret.set("sysname",    String(u.sysname,    CopyString));
  ret.set("nodename",   String(u.nodename,   CopyString));
  ret.set("release",    String(u.release,    CopyString));
  ret.set("version",    String(u.version,    CopyString));
  ret.set("machine",    String(u.machine,    CopyString));
#if defined(_GNU_SOURCE) && !defined(__APPLE__) && !defined(__FREEBSD__)
  ret.set("domainname", String(u.domainname, CopyString));
#endif
  return ret;
}

///////////////////////////////////////////////////////////////////////////////
}
