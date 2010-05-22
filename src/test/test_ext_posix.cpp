/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010 Facebook, Inc. (http://www.facebook.com)          |
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

#include <test/test_ext_posix.h>
#include <runtime/ext/ext_posix.h>
#include <runtime/ext/ext_file.h>

///////////////////////////////////////////////////////////////////////////////

bool TestExtPosix::RunTests(const std::string &which) {
  bool ret = true;

  RUN_TEST(test_posix_access);
  RUN_TEST(test_posix_ctermid);
  RUN_TEST(test_posix_get_last_error);
  RUN_TEST(test_posix_getcwd);
  RUN_TEST(test_posix_getegid);
  RUN_TEST(test_posix_geteuid);
  RUN_TEST(test_posix_getgid);
  RUN_TEST(test_posix_getgrgid);
  RUN_TEST(test_posix_getgrnam);
  RUN_TEST(test_posix_getgroups);
  RUN_TEST(test_posix_getlogin);
  RUN_TEST(test_posix_getpgid);
  RUN_TEST(test_posix_getpgrp);
  RUN_TEST(test_posix_getpid);
  RUN_TEST(test_posix_getppid);
  RUN_TEST(test_posix_getpwnam);
  RUN_TEST(test_posix_getpwuid);
  RUN_TEST(test_posix_getrlimit);
  RUN_TEST(test_posix_getsid);
  RUN_TEST(test_posix_getuid);
  RUN_TEST(test_posix_initgroups);
  RUN_TEST(test_posix_isatty);
  RUN_TEST(test_posix_kill);
  RUN_TEST(test_posix_mkfifo);
  RUN_TEST(test_posix_mknod);
  RUN_TEST(test_posix_setegid);
  RUN_TEST(test_posix_seteuid);
  RUN_TEST(test_posix_setgid);
  RUN_TEST(test_posix_setpgid);
  RUN_TEST(test_posix_setsid);
  RUN_TEST(test_posix_setuid);
  RUN_TEST(test_posix_strerror);
  RUN_TEST(test_posix_times);
  RUN_TEST(test_posix_ttyname);
  RUN_TEST(test_posix_uname);

  return ret;
}

///////////////////////////////////////////////////////////////////////////////

bool TestExtPosix::test_posix_access() {
  VERIFY(f_posix_access("test/test_ext_posix.cpp"));
  return Count(true);
}

bool TestExtPosix::test_posix_ctermid() {
  VERIFY(!f_posix_ctermid().empty());
  return Count(true);
}

bool TestExtPosix::test_posix_get_last_error() {
  errno = 0;
  VERIFY(!f_posix_get_last_error());
  return Count(true);
}

bool TestExtPosix::test_posix_getcwd() {
  VERIFY(!f_posix_getcwd().empty());
  return Count(true);
}

bool TestExtPosix::test_posix_getegid() {
  f_posix_getegid();
  return Count(true);
}

bool TestExtPosix::test_posix_geteuid() {
  f_posix_geteuid();
  return Count(true);
}

bool TestExtPosix::test_posix_getgid() {
  f_posix_getgid();
  return Count(true);
}

bool TestExtPosix::test_posix_getgrgid() {
  Variant ret = f_posix_getgrgid(f_posix_getgid());
  VERIFY(!same(ret, false));
  VERIFY(!ret.toArray().empty());
  return Count(true);
}

bool TestExtPosix::test_posix_getgrnam() {
  Variant ret = f_posix_getgrnam("root");
  VERIFY(!same(ret, false));
  VERIFY(!ret.toArray().empty());
  return Count(true);
}

bool TestExtPosix::test_posix_getgroups() {
  Variant ret = f_posix_getgroups();
  VERIFY(!same(ret, false));
  VERIFY(!ret.toArray().empty());
  return Count(true);
}

bool TestExtPosix::test_posix_getlogin() {
  f_posix_getlogin();
  return Count(true);
}

bool TestExtPosix::test_posix_getpgid() {
  VERIFY(f_posix_getpgid(0));
  return Count(true);
}

bool TestExtPosix::test_posix_getpgrp() {
  VERIFY(f_posix_getpgrp());
  return Count(true);
}

bool TestExtPosix::test_posix_getpid() {
  VERIFY(f_posix_getpid());
  return Count(true);
}

bool TestExtPosix::test_posix_getppid() {
  VERIFY(f_posix_getppid());
  return Count(true);
}

bool TestExtPosix::test_posix_getpwnam() {
  Variant ret = f_posix_getpwnam("root");
  VERIFY(!same(ret, false));
  VERIFY(!ret.toArray().empty());
  return Count(true);
}

bool TestExtPosix::test_posix_getpwuid() {
  Variant ret = f_posix_getpwuid(0);
  VERIFY(!same(ret, false));
  VERIFY(!ret.toArray().empty());
  return Count(true);
}

bool TestExtPosix::test_posix_getrlimit() {
  Variant ret = f_posix_getrlimit();
  VERIFY(!same(ret, false));
  VERIFY(!ret.toArray().empty());
  return Count(true);
}

bool TestExtPosix::test_posix_getsid() {
  VERIFY(f_posix_getsid(getpid()));
  return Count(true);
}

bool TestExtPosix::test_posix_getuid() {
  f_posix_getuid();
  return Count(true);
}

bool TestExtPosix::test_posix_initgroups() {
  f_posix_initgroups("root", 100);
  return Count(true);
}

bool TestExtPosix::test_posix_isatty() {
  f_posix_isatty(1);
  return Count(true);
}

bool TestExtPosix::test_posix_kill() {
  //VERIFY(f_posix_kill(-1, 9));
  return Count(true);
}

bool TestExtPosix::test_posix_mkfifo() {
  remove("/tmp/test_posix_mkfifo");
  VERIFY(f_posix_mkfifo("/tmp/test_posix_mkfifo", 0));
  return Count(true);
}

bool TestExtPosix::test_posix_mknod() {
  remove("/tmp/test_posix_mknod");
  VERIFY(f_posix_mknod("/tmp/test_posix_mknod", 0));
  return Count(true);
}

bool TestExtPosix::test_posix_setegid() {
  f_posix_setegid(0);
  return Count(true);
}

bool TestExtPosix::test_posix_seteuid() {
  f_posix_seteuid(0);
  return Count(true);
}

bool TestExtPosix::test_posix_setgid() {
  f_posix_setgid(0);
  return Count(true);
}

bool TestExtPosix::test_posix_setpgid() {
  VERIFY(f_posix_setpgid(0, 0));
  return Count(true);
}

bool TestExtPosix::test_posix_setsid() {
  VERIFY(f_posix_setsid());
  return Count(true);
}

bool TestExtPosix::test_posix_setuid() {
  f_posix_setuid(0);
  return Count(true);
}

bool TestExtPosix::test_posix_strerror() {
  VERIFY(!f_posix_strerror(EPERM).empty());
  return Count(true);
}

bool TestExtPosix::test_posix_times() {
  Variant ret = f_posix_times();
  VERIFY(!same(ret, false));
  VERIFY(!ret.toArray().empty());
  return Count(true);
}

bool TestExtPosix::test_posix_ttyname() {
  f_posix_ttyname(1);
  return Count(true);
}

bool TestExtPosix::test_posix_uname() {
  Variant ret = f_posix_uname();
  VERIFY(!same(ret, false));
  VERIFY(!ret.toArray().empty());
  return Count(true);
}
