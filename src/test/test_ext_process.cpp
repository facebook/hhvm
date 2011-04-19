/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010- Facebook, Inc. (http://www.facebook.com)         |
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

#include <test/test_ext_process.h>
#include <runtime/ext/ext_process.h>
#include <runtime/ext/ext_file.h>
#include <runtime/base/file/file.h>
#include <runtime/base/util/string_buffer.h>
#include <runtime/base/runtime_option.h>
#include <util/light_process.h>

using namespace std;

///////////////////////////////////////////////////////////////////////////////

bool TestExtProcess::RunTests(const std::string &which) {
  bool ret = true;

  RUN_TEST(test_pcntl_alarm);
  //RUN_TEST(test_pcntl_exec); // this has to run manually
  RUN_TEST(test_pcntl_fork);
  RUN_TEST(test_pcntl_getpriority);
  RUN_TEST(test_pcntl_setpriority);
  RUN_TEST(test_pcntl_signal);
  RUN_TEST(test_pcntl_wait);
  RUN_TEST(test_pcntl_waitpid);
  RUN_TEST(test_pcntl_wexitstatus);
  RUN_TEST(test_pcntl_wifexited);
  RUN_TEST(test_pcntl_wifsignaled);
  RUN_TEST(test_pcntl_wifstopped);
  RUN_TEST(test_pcntl_wstopsig);
  RUN_TEST(test_pcntl_wtermsig);
  RUN_TEST(test_pcntl_signal_dispatch);
  RUN_TEST(test_shell_exec);
  RUN_TEST(test_exec);
  RUN_TEST(test_passthru);
  RUN_TEST(test_system);
  RUN_TEST(test_proc_open);
  RUN_TEST(test_proc_terminate);
  RUN_TEST(test_proc_close);
  RUN_TEST(test_proc_get_status);
  RUN_TEST(test_proc_nice);
  RUN_TEST(test_escapeshellarg);
  RUN_TEST(test_escapeshellcmd);

  LightProcess::Initialize(RuntimeOption::LightProcessFilePrefix,
                           RuntimeOption::LightProcessCount);
  RUN_TEST(test_pcntl_alarm);
  //RUN_TEST(test_pcntl_exec); // this has to run manually
  RUN_TEST(test_pcntl_fork);
  RUN_TEST(test_pcntl_getpriority);
  RUN_TEST(test_pcntl_setpriority);
  RUN_TEST(test_pcntl_signal);
  RUN_TEST(test_pcntl_wait);
  RUN_TEST(test_pcntl_waitpid);
  RUN_TEST(test_pcntl_wexitstatus);
  RUN_TEST(test_pcntl_wifexited);
  RUN_TEST(test_pcntl_wifsignaled);
  RUN_TEST(test_pcntl_wifstopped);
  RUN_TEST(test_pcntl_wstopsig);
  RUN_TEST(test_pcntl_wtermsig);
  RUN_TEST(test_pcntl_signal_dispatch);
  RUN_TEST(test_shell_exec);
  RUN_TEST(test_exec);
  RUN_TEST(test_passthru);
  RUN_TEST(test_system);
  RUN_TEST(test_proc_open);
  RUN_TEST(test_proc_terminate);
  RUN_TEST(test_proc_close);
  RUN_TEST(test_proc_get_status);
  RUN_TEST(test_proc_nice);
  LightProcess::Close();

  return ret;
}

///////////////////////////////////////////////////////////////////////////////

bool TestExtProcess::test_pcntl_alarm() {
  //f_pcntl_alarm(1);
  return Count(true);
}

bool TestExtProcess::test_pcntl_exec() {
  f_pcntl_exec("/bin/sh",
               CREATE_VECTOR1("test/test_pcntl_exec.sh"),
               CREATE_MAP1("name", "value"));
  return Count(true);
}

bool TestExtProcess::test_pcntl_fork() {
  int pid = f_pcntl_fork();
  if (pid == 0) {
    exit(123);
  }
  Variant status;
  f_pcntl_wait(ref(status));
  return Count(true);
}

bool TestExtProcess::test_pcntl_getpriority() {
  VS(f_pcntl_getpriority(), 0);
  return Count(true);
}

bool TestExtProcess::test_pcntl_setpriority() {
  VERIFY(f_pcntl_setpriority(0));
  return Count(true);
}

bool TestExtProcess::test_pcntl_signal() {
  f_pcntl_signal(k_SIGALRM, "test", true);
  return Count(true);
}

bool TestExtProcess::test_pcntl_wait() {
  int pid = f_pcntl_fork();
  if (pid == 0) {
    exit(0x12);
  }
  Variant status;
  f_pcntl_wait(ref(status));
  VS(status, 0x1200);
  return Count(true);
}

bool TestExtProcess::test_pcntl_waitpid() {
  int pid = f_pcntl_fork();
  if (pid == 0) {
    exit(0x12);
  }
  Variant status;
  f_pcntl_waitpid(0, ref(status));
  VS(status, 0x1200);
  return Count(true);
}

bool TestExtProcess::test_pcntl_wexitstatus() {
  int pid = f_pcntl_fork();
  if (pid == 0) {
    exit(0x80);
  }
  Variant status;
  f_pcntl_waitpid(0, ref(status));
  VS(f_pcntl_wexitstatus(status), 0x80);
  return Count(true);
}

bool TestExtProcess::test_pcntl_wifexited() {
  int pid = f_pcntl_fork();
  if (pid == 0) {
    exit(0x12);
  }
  Variant status;
  f_pcntl_waitpid(0, ref(status));
  VERIFY(f_pcntl_wifexited(status));
  return Count(true);
}

bool TestExtProcess::test_pcntl_wifsignaled() {
  int pid = f_pcntl_fork();
  if (pid == 0) {
    exit(0x12);
  }
  Variant status;
  f_pcntl_waitpid(0, ref(status));
  VERIFY(!f_pcntl_wifsignaled(status));
  return Count(true);
}

bool TestExtProcess::test_pcntl_wifstopped() {
  int pid = f_pcntl_fork();
  if (pid == 0) {
    exit(0x12);
  }
  Variant status;
  f_pcntl_waitpid(0, ref(status));
  VERIFY(!f_pcntl_wifstopped(status));
  return Count(true);
}

bool TestExtProcess::test_pcntl_wstopsig() {
  int pid = f_pcntl_fork();
  if (pid == 0) {
    exit(0x12);
  }
  Variant status;
  f_pcntl_waitpid(0, ref(status));
  VS(f_pcntl_wstopsig(status), 0x12);
  return Count(true);
}

bool TestExtProcess::test_pcntl_wtermsig() {
  int pid = f_pcntl_fork();
  if (pid == 0) {
    exit(0x12);
  }
  Variant status;
  f_pcntl_waitpid(0, ref(status));
  VS(f_pcntl_wtermsig(status), 0);
  return Count(true);
}

bool TestExtProcess::test_shell_exec() {
  Variant output = f_shell_exec("echo hello");
  VS(output, "hello\n");

  string cur_cwd = Process::GetCurrentDirectory();
  f_chdir("/tmp/");
  VS(f_shell_exec("/bin/pwd"), "/tmp\n");
  f_chdir(String(cur_cwd));
  return Count(true);
}

bool TestExtProcess::test_pcntl_signal_dispatch() {
  f_pcntl_signal_dispatch();
  return Count(true);
}

bool TestExtProcess::test_exec() {
  Variant output, ret;
  String last_line = f_exec("echo hello; echo world;", ref(output), ref(ret));
  VS(output, CREATE_VECTOR2("hello", "world"));
  VS(last_line, "world");
  VS(ret, 0);

  string cur_cwd = Process::GetCurrentDirectory();
  f_chdir("/tmp/");
  VS(f_exec("/bin/pwd"), "/tmp");
  f_chdir(String(cur_cwd));
  return Count(true);
}

bool TestExtProcess::test_passthru() {
  g_context->obStart();
  Variant ret;
  f_passthru("echo hello; echo world;", ref(ret));
  String output = g_context->obCopyContents();
  g_context->obEnd();

  VS(output, "hello\nworld\n");
  VS(ret, 0);


  string cur_cwd = Process::GetCurrentDirectory();
  f_chdir("/tmp/");
  g_context->obStart();
  f_passthru("/bin/pwd");
  output = g_context->obCopyContents();
  g_context->obEnd();
  VS(output, "/tmp\n");
  f_chdir(String(cur_cwd));

  return Count(true);
}

bool TestExtProcess::test_system() {
  g_context->obStart();
  Variant ret;
  String last_line = f_system("echo hello; echo world;", ref(ret));
  String output = g_context->obCopyContents();
  g_context->obEnd();

  VS(output, "hello\nworld\n");
  VS(last_line, "world");
  VS(ret, 0);

  string cur_cwd = Process::GetCurrentDirectory();
  f_chdir("/tmp/");
  VS(f_system("/bin/pwd"), "/tmp");
  f_chdir(String(cur_cwd));
  return Count(true);
}

bool TestExtProcess::test_proc_open() {
  Array descriptorspec =
    CREATE_MAP3(0, CREATE_VECTOR2("pipe", "r"),
                1, CREATE_VECTOR2("pipe", "w"),
                2, CREATE_VECTOR3("file", "/tmp/error-output.txt", "a"));
  String cwd = "/tmp";
  Array env = CREATE_MAP1("some_option", "aeiou");

  Variant pipes;
  Variant process = f_proc_open("php", descriptorspec, ref(pipes), cwd, env);
  VERIFY(!same(process, false));

  {
    File *f = pipes[0].toObject().getTyped<File>();
    VERIFY(f->valid());
    String s("<?php print(getenv('some_option')); ?>", AttachLiteral);
    f->write(s);
    f->close();
  }
  {
    File *f = pipes[1].toObject().getTyped<File>();
    VERIFY(f->valid());
    StringBuffer sbuf;
    sbuf.read(f);
    f->close();
    VS(sbuf.detach(), "aeiou");
  }

  VS(f_proc_close(process.toObject()), 0);
  return Count(true);
}

bool TestExtProcess::test_proc_terminate() {
  Array descriptorspec =
    CREATE_MAP3(0, CREATE_VECTOR2("pipe", "r"),
                1, CREATE_VECTOR2("pipe", "w"),
                2, CREATE_VECTOR3("file", "/tmp/error-output.txt", "a"));
  Variant pipes;
  Variant process = f_proc_open("php", descriptorspec, ref(pipes));
  VERIFY(!same(process, false));
  VERIFY(f_proc_terminate(process.toObject()));
  // still need to close it, not to leave a zombie behind
  f_proc_close(process.toObject());
  return Count(true);
}

bool TestExtProcess::test_proc_close() {
  return test_proc_open();
}

bool TestExtProcess::test_proc_get_status() {
  Array descriptorspec =
    CREATE_MAP3(0, CREATE_VECTOR2("pipe", "r"),
                1, CREATE_VECTOR2("pipe", "w"),
                2, CREATE_VECTOR3("file", "/tmp/error-output.txt", "a"));
  Variant pipes;
  Variant process = f_proc_open("php", descriptorspec, ref(pipes));
  VERIFY(!same(process, false));
  Array ret = f_proc_get_status(process.toObject());
  VS(ret["command"], "php");
  VERIFY(ret["pid"].toInt32() > 0);
  VERIFY(ret["running"]);
  VERIFY(!ret["signaled"]);
  VS(ret["exitcode"], -1);
  VS(ret["termsig"], 0);
  VS(ret["stopsig"], 0);

  {
    File *f = pipes[0].toObject().getTyped<File>();
    VERIFY(f->valid());
    f->close();
  }
  {
    File *f = pipes[1].toObject().getTyped<File>();
    VERIFY(f->valid());
    f->close();
  }
  VS(f_proc_close(process.toObject()), 0);
  return Count(true);
}

bool TestExtProcess::test_proc_nice() {
  VERIFY(f_proc_nice(0));
  return Count(true);
}

bool TestExtProcess::test_escapeshellarg() {
  VS(f_escapeshellarg("\""), "'\"'");
  return Count(true);
}

bool TestExtProcess::test_escapeshellcmd() {
  VS(f_escapeshellcmd("perl \""), "perl \\\"");
  return Count(true);
}
