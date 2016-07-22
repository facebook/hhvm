/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2016 Facebook, Inc. (http://www.facebook.com)     |
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

#include "hphp/util/current-executable.h"

#include <folly/ScopeGuard.h>
#include <folly/portability/Unistd.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <cstdio>
#include <cstring>
#include <fcntl.h>

#include <fstream>
#include <memory>

#ifdef __APPLE__
#include <mach-o/getsect.h>
#elif defined(__CYGWIN__) || defined(__MINGW__) || defined(_MSC_VER)
#include <windows.h>
#include <winuser.h>
#else
#include <folly/experimental/symbolizer/Elf.h>
#endif

namespace HPHP {

bool get_embedded_data(const char* section, embedded_data* desc,
                       const std::string& filename /*= "" */) {
  auto const fname = filename.empty() ? current_executable_path() : filename;

#if defined(__CYGWIN__) || defined(__MINGW__) || defined(_MSC_VER)
  HMODULE moduleHandle = GetModuleHandleA(fname.data());
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
  folly::symbolizer::ElfFile file;
  if (file.openNoThrow(fname.c_str()) != 0) return false;

  auto const shdr = file.getSectionByName(section);
  if (shdr == nullptr) return false;

  desc->m_filename = fname;
  desc->m_start = shdr->sh_offset;
  desc->m_len = shdr->sh_size;
  return true;
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

std::string read_embedded_data(const embedded_data& desc) {
#if (defined(__CYGWIN__) || defined(__MINGW__) || defined(_MSC_VER))
  return std::string((const char*)LockResource(desc.m_handle), desc.m_len);
#else
  std::ifstream ifs(desc.m_filename);
  if (!ifs.good()) return "";
  ifs.seekg(desc.m_start, std::ios::beg);
  std::unique_ptr<char[]> data(new char[desc.m_len]);
  ifs.read(data.get(), desc.m_len);
  return std::string(data.get(), desc.m_len);
#endif
}

}
