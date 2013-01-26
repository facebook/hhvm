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

#include <libelf.h>
#include <gelf.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>

#include <runtime/vm/embedded_repo.h>
#include <util/embedded_vfs.h>

namespace HPHP {

std::string get_embedded_repo() {
  GElf_Shdr shdr;
  size_t shstrndx;
  char *name;
  Elf_Scn *scn;

  if (elf_version(EV_CURRENT) == EV_NONE) return "";

  int fd = open("/proc/self/exe", O_RDONLY, 0);
  if (fd < 0) return "";
  Elf* e = elf_begin(fd, ELF_C_READ, NULL);

  if (!e ||
      elf_kind(e) != ELF_K_ELF ||
      !elf_getshstrndx(e, &shstrndx)) {
    return "";
  }
  scn = NULL;
  while ((scn = elf_nextscn(e, scn)) != NULL) {
    if (gelf_getshdr(scn, &shdr) != &shdr ||
        !(name = elf_strptr(e, shstrndx , shdr.sh_name))) {
      return "";
    }
    if (!strcmp("repo", name)) {
      GElf_Shdr ghdr;
      if (gelf_getshdr(scn, &ghdr) != &ghdr) return "";
      char buf[512];
      sprintf(buf, "/proc/self/exe:%lu:%lu", ghdr.sh_offset, ghdr.sh_size);
      sqlite3_embedded_initialize(NULL, true);
      return buf;
    }
  }
  return "";
}

}
