/**
 * Copyright (c) 2015, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the "hack" directory of this source tree.
 *
 */

#define CAML_NAME_SPACE
#include <caml/fail.h>
#include <caml/memory.h>
#include <caml/unixsupport.h>

#include <stdio.h>
#include <errno.h>

#ifndef _WIN32
#include <sys/time.h>
#endif

#ifdef __linux__
#include <linux/magic.h>
#include <sys/vfs.h>
#endif

void hh_lutimes(value filename_v) {
  CAMLparam1(filename_v);
#ifdef _WIN32
  /* Not implemented */
  CAMLreturn0;
#else
  char* filename = String_val(filename_v);
  int success = lutimes(filename, NULL);
  if (success != 0) {
    caml_failwith("lutimes failed");
  }
#endif
  CAMLreturn0;
}

value hh_is_nfs(value filename_v) {
  CAMLparam1(filename_v);
#ifdef __linux__
  struct statfs buf;
  char* filename = String_val(filename_v);
  int success = statfs(filename, &buf);
  if (success != 0) {
    caml_failwith("statfs failed");
  }
  switch (buf.f_type) {
#ifdef CIFS_MAGIC_NUMBER
    case CIFS_MAGIC_NUMBER:
#endif
    case NFS_SUPER_MAGIC:
    case SMB_SUPER_MAGIC:
      CAMLreturn(Val_bool(1));
    default:
      CAMLreturn(Val_bool(0));
  }
#endif
  CAMLreturn(Val_bool(0));
}

// C89 spec: "The primary use of the freopen function is to change the file associated
// with a standard text stream (stderr, stdin, or stdout)" e.g. freopen("newout.txt", "a", stdout)
// It returns NULL upon failure, or the third parameter upon success.
// The right way to use freopen is to ignore the returned value in success case:
// https://stackoverflow.com/questions/584868/rerouting-stdin-and-stdout-from-c/586416#586416
void hh_freopen(value filename_v, value mode_v, value fd_v) {
  CAMLparam3(filename_v, mode_v, fd_v);
  char *filename = String_val(filename_v);
  char *mode = String_val(mode_v);
  int fd = Int_val(fd_v);
  FILE *fp = fdopen(fd, mode);
  if (fp == NULL) {
    unix_error(errno, "fdopen", filename_v); // raises Unix.error
  }
  FILE *r = freopen(filename, mode, fp);
  if (r == NULL) {
    unix_error(errno, "freopen", filename_v); // raises Unix.error
  }
  CAMLreturn0;
}
