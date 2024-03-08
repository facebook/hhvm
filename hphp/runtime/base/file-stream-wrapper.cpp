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

#include "hphp/runtime/base/file-stream-wrapper.h"
#include "hphp/runtime/base/unit-cache.h"
#include "hphp/runtime/base/runtime-error.h"
#include "hphp/runtime/base/plain-file.h"
#include "hphp/runtime/base/directory.h"
#include "hphp/runtime/server/static-content-cache.h"
#include "hphp/runtime/base/file-util.h"
#include "hphp/runtime/base/string-util.h"
#include "hphp/runtime/vm/native.h"
#include "hphp/runtime/ext/stream/ext_stream.h"

#include <filesystem>
#include <memory>

#include <folly/portability/Stdlib.h>
#include <folly/portability/SysStat.h>

#include <sys/xattr.h>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

FileStreamWrapper s_file_stream_wrapper;

req::ptr<MemFile> FileStreamWrapper::openFromCache(const String& filename,
                                                   const String& mode) {
  if (!StaticContentCache::TheFileCache) {
    return nullptr;
  }

  String path = File::TranslatePath(filename);
  auto file = req::make<MemFile>();
  bool ret = file->open(path, mode);
  if (ret) {
    return file;
  }
  return nullptr;
}

req::ptr<File>
FileStreamWrapper::open(const String& filename, const String& mode, int options,
                        const req::ptr<StreamContext>& /*context*/) {
  String fname;
  if (StringUtil::IsFileUrl(filename)) {
    fname = StringUtil::DecodeFileUrl(filename);
    if (fname.empty()) {
      raise_warning("invalid file:// URL");
      return nullptr;
    }
  } else {
    fname = filename;
  }

  if (auto file = openFromCache(fname, mode)) {
    return file;
  }

  if (options & File::USE_INCLUDE_PATH) {
    struct stat s;
    String resolved_fname = resolveVmInclude(fname.get(), "", &s);
    if (!resolved_fname.isNull()) {
      fname = resolved_fname;
    }
  }

  auto file = req::make<PlainFile>();
  bool ret = file->open(File::TranslatePath(fname), mode);
  if (!ret) {
    raise_warning("%s", file->getLastError().c_str());
    return nullptr;
  }
  return file;
}

req::ptr<Directory> FileStreamWrapper::opendir(const String& path) {
  auto tpath = File::TranslatePath(path);
  if (File::IsVirtualDirectory(tpath)) {
    return req::make<CachedDirectory>(tpath);
  }

  auto dir = req::make<PlainDirectory>(tpath);
  if (!dir->isValid()) {
    raise_warning("%s", dir->getLastError().c_str());
    return nullptr;
  }
  return dir;
}

int FileStreamWrapper::unlink(const String& path) {
  int ret = ::unlink(File::TranslatePath(path).data());
  if (ret != 0) {
    raise_warning(
      "%s(%s): %s",
      __FUNCTION__,
      path.c_str(),
      folly::errnoStr(errno).c_str()
    );
  }
  return ret;
}

int FileStreamWrapper::rename(const String& oldname, const String& newname) {
  int ret =
    Cfg::Server::UseDirectCopy ?
      FileUtil::directRename(File::TranslatePath(oldname).data(),
                             File::TranslatePath(newname).data())
                                 :
      FileUtil::rename(File::TranslatePath(oldname).data(),
                       File::TranslatePath(newname).data());
  return ret;
}

int FileStreamWrapper::mkdir(const String& path, int mode, int options) {
  if (options & k_STREAM_MKDIR_RECURSIVE) {
    ERROR_RAISE_WARNING(mkdir_recursive(path, mode));
    return ret;
  }
  ERROR_RAISE_WARNING(::mkdir(File::TranslatePath(path).data(), mode));
  return ret;
}

int FileStreamWrapper::mkdir_recursive(const String& path, int mode) {
  String fullpath = File::TranslatePath(path);
  if (fullpath.size() > PATH_MAX) {
    errno = ENAMETOOLONG;
    return -1;
  }

  // Check first if the whole path exists
  if (access(fullpath.data(), F_OK) >= 0) {
    errno = EEXIST;
    return -1;
  }

  char dir[PATH_MAX+1];
  char *p;
  strncpy(dir, fullpath.data(), sizeof(dir));

  for (p = dir + 1; *p; p++) {
    if (FileUtil::isDirSeparator(*p)) {
      *p = '\0';
      if (::mkdir(dir, mode) < 0) {
        if (!*(p+1) || errno != EEXIST) return -1;
      }
      *p = FileUtil::getDirSeparator();
    }
  }

  if (::access(dir, F_OK) < 0) {
    if (::mkdir(dir, mode) < 0) {
      return -1;
    }
  }

  return 0;
}

Optional<std::string> FileStreamWrapper::getxattr(const char* path,
                                                  const char* xattr) {
  std::string buf;
  buf.resize(64);

  while (true) {
    auto const ret = ::getxattr(path, xattr, buf.data(), buf.size());
    if (ret >= 0) {
      assertx(ret <= buf.size());
      buf.resize(ret);
      return buf;
    }
    if (errno != ERANGE) break;
    auto const actualSize = ::getxattr(path, xattr, nullptr, 0);
    if (actualSize < 0) break;
    buf.resize(std::max<size_t>(actualSize, buf.size()));
  }
  return std::nullopt;
}

///////////////////////////////////////////////////////////////////////////////
}
