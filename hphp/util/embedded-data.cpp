/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2014 Facebook, Inc. (http://www.facebook.com)     |
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

#include "hphp/util/embedded-data.h"

#if (defined(__CYGWIN__) || defined(__MINGW__) || defined(_MSC_VER))
#include <windows.h>
#include <winuser.h>
#endif

#include "hphp/util/current-executable.h"

#include <folly/ScopeGuard.h>

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>

#ifdef __APPLE__
#include <mach-o/getsect.h>
#else
#include <libelf.h>
#include <gelf.h>
#endif

namespace HPHP {

bool get_embedded_data(const char *section, embedded_data* desc,
                       const std::string &filename /*= "" */) {
  std::string fname(filename.empty() ? current_executable_path() : filename);

#if (defined(__CYGWIN__) || defined(__MINGW__) || defined(_MSC_VER))
  HMODULE moduleHandle = GetModuleHandle(nullptr);
  HGLOBAL loadedResource;
  HRSRC   resourceInfo;
  DWORD   resourceSize;

  resourceInfo = FindResource(moduleHandle, section, RT_RCDATA);

  if (!resourceInfo) {
    return false;
  }

  loadedResource = LoadResource(moduleHandle, resourceInfo);

  if (!loadedResource) {
    return false;
  }

  resourceSize = SizeofResource(moduleHandle, resourceInfo);

  desc->m_filename = fname;
  desc->m_handle = loadedResource;
  desc->m_len = resourceSize;

  return true;

#elif !defined(__APPLE__) // LINUX/ELF
  GElf_Shdr shdr;
  size_t shstrndx = -1;
  char *name;
  Elf_Scn *scn;

  if (elf_version(EV_CURRENT) == EV_NONE) return false;

  int fd = open(fname.c_str(), O_RDONLY, 0);
  if (fd < 0) return false;
  SCOPE_EXIT { close(fd); };

  Elf* e = elf_begin(fd, ELF_C_READ, nullptr);
  SCOPE_EXIT { elf_end(e); };
  if (e == nullptr || elf_kind(e) != ELF_K_ELF) {
    return false;
  }

  auto get_shstrndx =
#ifdef HAVE_ELF_GETSHDRSTRNDX
    elf_getshdrstrndx;
#else
    elf_getshstrndx;
#endif

  int stat = get_shstrndx(e, &shstrndx);
  if (stat < 0 || shstrndx == size_t(-1)) {
    return false;
  }

  scn = nullptr;
  while ((scn = elf_nextscn(e, scn)) != nullptr) {
    if (gelf_getshdr(scn, &shdr) != &shdr ||
        !(name = elf_strptr(e, shstrndx , shdr.sh_name))) {
      return false;
    }
    if (!strcmp(section, name)) {
      GElf_Shdr ghdr;
      if (gelf_getshdr(scn, &ghdr) != &ghdr) return false;
      desc->m_filename = fname;
      desc->m_start = ghdr.sh_offset;
      desc->m_len = ghdr.sh_size;
      return true;
    }
  }
#else // __APPLE__
  const struct section_64 *sect = getsectbyname("__text", section);
  if (sect) {
    desc->m_filename = fname;
    desc->m_start = sect->offset;
    desc->m_len = sect->size;
    return !desc->m_filename.empty();
  }
#endif // __APPLE__
  return false;
}

}
