/**
 * Copyright (c) 2014, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the "hack" directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 *
 */

#include <caml/memory.h>
#include <caml/alloc.h>
#include <fcntl.h>
#include <gelf.h>
#include <libelf.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#define NONE Val_int(0)

static value SOME(value v) {
  CAMLparam1(v);
  CAMLlocal1(result);

  result = caml_alloc(1, 0);
  Store_field(result, 0, v);

  CAMLreturn(result);
}

/**
 * Look for a magic "hhi" elf section and read it out, if it exists. Most of
 * this code adapted from hphp/util/embedded-data.cpp.
 */
value get_embedded_hhi_data(value filename) {
  CAMLparam1(filename);
  CAMLlocal1(result);

  if (elf_version(EV_CURRENT) == EV_NONE) {
    goto fail_early;
  }

  int fd = open(String_val(filename), O_RDONLY);
  if (fd < 0) {
    goto fail_early;
  }

  Elf* e = elf_begin(fd, ELF_C_READ, 0);
  if (!e || elf_kind(e) != ELF_K_ELF) {
    goto fail_after_open;
  }

  size_t shstrndx = -1;
#ifdef HAVE_ELF_GETSHDRSTRNDX
  int stat = elf_getshdrstrndx(e, &shstrndx);
#else
  int stat = elf_getshstrndx(e, &shstrndx);
#endif
  if (stat < 0 || shstrndx == -1) {
    goto fail_after_elf_begin;
  }

  Elf_Scn *scn = 0;
  while ((scn = elf_nextscn(e, scn))) {
    GElf_Shdr shdr;
    if (gelf_getshdr(scn, &shdr) != &shdr) {
      goto fail_after_elf_begin;
    }

    char *name = elf_strptr(e, shstrndx, shdr.sh_name);
    if (!name) {
      goto fail_after_elf_begin;
    }

    if (!strcmp("hhi", name)) {
      GElf_Shdr ghdr;
      if (gelf_getshdr(scn, &ghdr) != &ghdr) {
        goto fail_after_elf_begin;
      }

      size_t offset = ghdr.sh_offset;
      size_t size = ghdr.sh_size;

      elf_end(e);

      lseek(fd, offset, SEEK_SET);
      result = caml_alloc_string(size);
      read(fd, String_val(result), size);

      close(fd);
      CAMLreturn(SOME(result));
    }
  }

fail_after_elf_begin:
  elf_end(e);
fail_after_open:
  close(fd);
fail_early:
  CAMLreturn(NONE);
}
