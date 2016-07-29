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
#include "hphp/util/logger.h"

#include <folly/FileUtil.h>
#include <folly/ScopeGuard.h>
#include <folly/String.h>
#include <folly/portability/Unistd.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <cstdio>
#include <cstring>
#include <dlfcn.h>
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

///////////////////////////////////////////////////////////////////////////////

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

#if (defined(__CYGWIN__) || defined(__MINGW__) || defined(_MSC_VER))

std::string read_embedded_data(const embedded_data& desc) {
  return std::string((const char*)LockResource(desc.m_handle), desc.m_len);
}
void* dlopen_embedded_data(const embedded_data&, char*) {
  return nullptr;
}

#else

std::string read_embedded_data(const embedded_data& desc) {
  std::ifstream ifs(desc.m_filename);
  if (!ifs.good()) return "";
  ifs.seekg(desc.m_start, std::ios::beg);
  std::unique_ptr<char[]> data(new char[desc.m_len]);
  ifs.read(data.get(), desc.m_len);
  return std::string(data.get(), desc.m_len);
}

void* dlopen_embedded_data(const embedded_data& desc, char* tmp_filename) {
  auto const source_file = ::open(desc.m_filename.c_str(), O_RDONLY);
  if (source_file < 0) {
    Logger::Error("dlopen_embedded_data: Unable to open '%s': %s",
                  desc.m_filename.c_str(), folly::errnoStr(errno).c_str());
    return nullptr;
  }
  SCOPE_EXIT { ::close(source_file); };

  if (::lseek(source_file, desc.m_start, SEEK_SET) < 0) {
    Logger::Error("dlopen_embedded_data: Unable to seek to section: %s",
                  folly::errnoStr(errno).c_str());
    return nullptr;
  }

  auto const dest_file = ::mkstemp(tmp_filename);
  if (dest_file < 0) {
    Logger::Error("dlopen_embedded_data: Unable to create temporary file: %s",
                  folly::errnoStr(errno).c_str());
    return nullptr;
  }
  SCOPE_EXIT { ::unlink(tmp_filename); };
  SCOPE_EXIT { ::close(dest_file); };

  char buffer[64*1024];
  std::size_t to_read = desc.m_len;

  while (to_read > 0) {
    auto const read = folly::readNoInt(source_file, buffer,
                                       std::min(sizeof(buffer), to_read));
    if (read <= 0) {
      Logger::Error("dlopen_embedded_data: Error reading from section: %s",
                    folly::errnoStr(errno).c_str());
      return nullptr;
    }
    if (folly::writeFull(dest_file, buffer, read) <= 0) {
      Logger::Error("dlopen_embedded_data: Error writing to temporary file: %s",
                    folly::errnoStr(errno).c_str());
      return nullptr;
    }
    to_read -= read;
  }

  // Finished copying the file; now load it.
  auto const handle = dlopen(tmp_filename, RTLD_NOW);
  if (!handle) {
    Logger::Error("dlopen_embedded_data: dlopen failed: %s", dlerror());
    return nullptr;
  }
  return handle;
}

#endif

///////////////////////////////////////////////////////////////////////////////

}
