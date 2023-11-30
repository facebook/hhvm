/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <errno.h>
#include <fmt/core.h>
#include <stdio.h>
#include <stdlib.h>
#include <windows.h>

// Super simple utility to suspend or resume all threads in a target process.
// We use this in place of `kill -STOP` and `kill -CONT`

typedef LONG(NTAPI* sus_res_func)(HANDLE proc);

const char* win32_strerror(DWORD err) {
  static char msgbuf[1024];
  FormatMessageA(
      FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
      NULL,
      err,
      MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
      msgbuf,
      sizeof(msgbuf) - 1,
      NULL);
  return msgbuf;
}

int apply(DWORD pid, BOOL suspend) {
  sus_res_func func;
  const char* name = suspend ? "NtSuspendProcess" : "NtResumeProcess";
  HANDLE proc;
  DWORD res;

  func = (sus_res_func)GetProcAddress(GetModuleHandle("ntdll"), name);

  if (!func) {
    fmt::print(
        "Failed to GetProcAddress({}): {}\n",
        name,
        win32_strerror(GetLastError()));
    return 1;
  }

  proc = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
  if (proc == INVALID_HANDLE_VALUE) {
    fmt::print(
        "Failed to OpenProcess({}): {}\n", pid, win32_strerror(GetLastError()));
    return 1;
  }

  res = func(proc);

  if (res) {
    fmt::print(
        "{}({}) returns {:x}: {}\n", name, pid, res, win32_strerror(res));
  }

  CloseHandle(proc);

  return res == 0 ? 0 : 1;
}

void usage() {
  fmt::print(
      "Usage: susres suspend [pid]\n"
      "       susres resume  [pid\n");
  exit(1);
}

int main(int argc, char** argv) {
  DWORD pid;
  BOOL suspend;

  if (argc != 3) {
    usage();
  }

  if (!strcmp(argv[1], "suspend")) {
    suspend = TRUE;
  } else if (!strcmp(argv[1], "resume")) {
    suspend = FALSE;
  } else {
    usage();
  }

  pid = _atoi64(argv[2]);

  return apply(pid, suspend);
}

/* vim:ts=2:sw=2:et:
 */
