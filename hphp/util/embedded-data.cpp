/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-present Facebook, Inc. (http://www.facebook.com)  |
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
#include <folly/Range.h>
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
#include <vector>

#ifdef __APPLE__
#include <mach-o/getsect.h>
#elif defined(_MSC_VER)
#include <windows.h>
#include <winuser.h>
#else
#include <folly/experimental/symbolizer/Elf.h>
#endif

namespace HPHP {

///////////////////////////////////////////////////////////////////////////////

namespace {

/*
 * Temporary files created by dlopen_embedded_data().
 */
std::vector<std::string> s_tmp_files;

/*
 * Lock around accesses to s_tmp_files.
 */
std::mutex s_tmp_files_lock;

const char* kBuildIdPlaceholder = "%{buildid}";

std::string insertBuildId(std::string path, const std::string& buildId) {
  assert(strstr(buildId.c_str(), kBuildIdPlaceholder) == nullptr);
  auto const idx = path.find(kBuildIdPlaceholder);
  // Placeholder is required in the path
  always_assert(idx != std::string::npos);
  path.replace(idx, strlen(kBuildIdPlaceholder), buildId);
  return path;
}

}

///////////////////////////////////////////////////////////////////////////////

bool get_embedded_data(const char* section, embedded_data* desc,
                       const std::string& filename /*= "" */) {
  auto const fname = filename.empty() ? current_executable_path() : filename;

#ifdef _MSC_VER
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

#ifdef _MSC_VER

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

void* dlopen_embedded_data(const embedded_data& desc,
                           std::string extractPath,
                           std::string fallbackPath,
                           const std::string& buildId,
                           bool trust) {
  // Replace build-id placeholders in the paths with the actual build-ids.
  if (!extractPath.empty()) {
    extractPath = insertBuildId(extractPath, buildId);
  }
  if (!fallbackPath.empty()) {
    fallbackPath = insertBuildId(fallbackPath, buildId);
  }

  auto const filename = [&]{
    // First check if there's an existing file at extractPath if we're trusting.
    if (trust && !extractPath.empty() &&
        ::access(extractPath.c_str(), R_OK) == 0) {
      return extractPath;
    }

    // There's no existing file or we're not trusting. In either case, we're
    // going to need the embedded data, so read it out of the section.
    auto const sourceFile = ::open(desc.m_filename.c_str(), O_RDONLY);
    if (sourceFile < 0) {
      Logger::Error("dlopen_embedded_data: Unable to open '%s': %s",
                    desc.m_filename.c_str(), folly::errnoStr(errno).c_str());
      return std::string{};
    }
    SCOPE_EXIT { ::close(sourceFile); };

    if (::lseek(sourceFile, desc.m_start, SEEK_SET) < 0) {
      Logger::Error("dlopen_embedded_data: Unable to seek to section: %s",
                    folly::errnoStr(errno).c_str());
      return std::string{};
    }

    std::unique_ptr<unsigned char[]> data(new unsigned char[desc.m_len]);
    auto const amountRead =
      folly::readFull(sourceFile, data.get(), desc.m_len);
    if (amountRead != desc.m_len) {
      Logger::Error("dlopen_embedded_data: Unable to read section fully");
      return std::string{};
    }

    if (!extractPath.empty()) {
      // If extractPath exists, read it and verify it has the same contents as
      // the embedded data.
      if (::access(extractPath.c_str(), R_OK) == 0) {
        auto const fd = ::open(extractPath.c_str(), O_RDONLY);
        if (fd != -1) {
          SCOPE_EXIT { ::close(fd); };
          std::vector<unsigned char> contents;
          if (folly::readFile(fd, contents) &&
              folly::ByteRange{contents.data(), contents.size()} ==
              folly::ByteRange{data.get(), desc.m_len}) {
            return extractPath;
          }
        }
      }

      // extractPath doesn't exist, or it has the wrong contents. Create a new
      // file in its place with the correct contents.
      try {
        folly::writeFileAtomic(
          extractPath,
          folly::ByteRange{data.get(), desc.m_len},
          0644
        );
        return extractPath;
      } catch (const std::system_error& ex) {
      }
    }

    // We can't use extractPath, so try fallbackPath now. Create a temporary
    // file and fill it with the embedded data contents.

    if (fallbackPath.empty()) {
      Logger::Error(
        "dlopen_embedded_data: Unable to write to file and no fallback"
      );
      return std::string{};
    }

    auto const fallbackFile = ::mkstemp(&fallbackPath[0]);
    if (fallbackFile < 0) {
      Logger::Error("dlopen_embedded_data: Unable to create temporary file: %s",
                    folly::errnoStr(errno).c_str());
      return std::string{};
    }
    SCOPE_EXIT { ::close(fallbackFile); };

    {
      // We don't unlink these files here because doing so can cause gdb to
      // segfault or fail in other mysterious ways.  Some bug reports suggest
      // that to work around this, we need to use some sort of JIT extension
      // live, which we don't want to do:
      //    - https://sourceware.org/ml/gdb-prs/2014-q1/msg00178.html
      //    - https://gcc.gnu.org/bugzilla/show_bug.cgi?id=64206
      //
      // So instead, we just hope that HHVM shuts down gracefully and that when
      // it doesn't, some external job will clear it out for us.
      std::lock_guard<std::mutex> l(s_tmp_files_lock);
      s_tmp_files.push_back(fallbackPath);
    }

    folly::writeFull(fallbackFile, data.get(), desc.m_len);
    return fallbackPath;
  }();
  if (filename.empty()) return nullptr;

  // Finished copying the file; now load it.
  auto const handle = dlopen(filename.c_str(), RTLD_NOW);
  if (!handle) {
    Logger::Error("dlopen_embedded_data: dlopen failed: %s", dlerror());
    return nullptr;
  }
  return handle;
}

#endif

void embedded_data_cleanup() {
  std::lock_guard<std::mutex> l(s_tmp_files_lock);

  for (auto const& filename : s_tmp_files) {
    ::unlink(filename.c_str());
  }
}

///////////////////////////////////////////////////////////////////////////////

extern "C" {

ssize_t hphp_read_embedded_data(const char* section, char* buf, size_t len) {
  embedded_data data;
  if (get_embedded_data(section, &data)) {
    auto str = read_embedded_data(data);
    auto data_len = str.length();
    auto real_len = data_len < len ? data_len : len;
    memcpy(buf, str.data(), real_len * sizeof(char));
    return real_len;
  } else {
    return -1;
  }
}

}

} // namespace HPHP
