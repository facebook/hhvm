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
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>

#ifdef _MSC_VER
#include <io.h>
#include <Windows.h>
#elif defined(__APPLE__)
#include <unistd.h>
#include <mach-o/getsect.h>
#else
#include <unistd.h>
#include <gelf.h>
#include <libelf.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

#define NONE Val_int(0)

static value SOME(value v) {
  CAMLparam1(v);
  CAMLlocal1(result);

  result = caml_alloc(1, 0);
  Store_field(result, 0, v);

  CAMLreturn(result);
}

#ifdef _MSC_VER

value get_embedded_hhi_data(value filename) {
  CAMLparam1(filename);
  CAMLlocal1(result);

  int fd = _open(String_val(filename), O_RDONLY);
  if (fd < 0) {
    goto fail_early;
  }

  IMAGE_DOS_HEADER dosHeader;
  if (_read(fd, &dosHeader, sizeof(dosHeader)) != sizeof(dosHeader))
    goto fail_after_open;
  if (dosHeader.e_magic != IMAGE_DOS_SIGNATURE)
    goto fail_after_open;

  _lseek(fd, dosHeader.e_lfanew, SEEK_SET);
  IMAGE_NT_HEADERS ntHeader;
  if (_read(fd, &ntHeader, sizeof(ntHeader)) != sizeof(ntHeader))
    goto fail_after_open;

  _lseek(fd, ntHeader.FileHeader.SizeOfOptionalHeader, SEEK_CUR);
  for (WORD i = 0; i < ntHeader.FileHeader.NumberOfSections; i++) {
    IMAGE_SECTION_HEADER sectionHeader;
    if (_read(fd, &sectionHeader, sizeof(sectionHeader)) != sizeof(sectionHeader))
      goto fail_after_open;

    if (!memcmp(sectionHeader.Name, "hhi", 4)) {
      _lseek(fd, sectionHeader.VirtualAddress, SEEK_SET);
      result = caml_alloc_string(sectionHeader.Misc.VirtualSize);
      if (_read(fd, String_val(result), sectionHeader.Misc.VirtualSize) != sectionHeader.Misc.VirtualSize) {
        goto fail_after_open;
      }
      _close(fd);
      CAMLreturn(SOME(result));
    }
  }

fail_after_open:
  _close(fd);
fail_early:
  CAMLreturn(NONE);
}

#elif !defined(__APPLE__)
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

  size_t shstrndx = 0;
#ifdef HAVE_ELF_GETSHDRSTRNDX
  int stat = elf_getshdrstrndx(e, &shstrndx);
#else
  int stat = elf_getshstrndx(e, &shstrndx);
#endif
  if (stat < 0 || shstrndx == 0) {
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
      ssize_t ret = read(fd, String_val(result), size);
      if (ret != (ssize_t)size) {
        goto fail_after_elf_begin;
      }

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

#else

/**
 * Beware! The getsect* functions do NOT play well with ASLR, so we cannot just
 * copy the data out of the memory address at sect->addr. We could link this
 * with -Wl,-no_pie, but it is easier to just open the binary and read it from
 * disk.
 */
value get_embedded_hhi_data(value filename) {
  CAMLparam1(filename);
  CAMLlocal1(result);

  const struct section_64 *sect = getsectbyname("__text", "hhi");
  if (sect == NULL) {
    goto fail_early;
  }

  int fd = open(String_val(filename), O_RDONLY);
  if (fd < 0) {
    goto fail_early;
  }

  lseek(fd, sect->offset, SEEK_SET);

  result = caml_alloc_string(sect->size);
  if (read(fd, String_val(result), sect->size) != sect->size) {
    goto fail_after_open;
  }
  close(fd);
  CAMLreturn(SOME(result));

fail_after_open:
  close(fd);
fail_early:
  CAMLreturn(NONE);
}

#endif

#ifdef __cplusplus
}
#endif