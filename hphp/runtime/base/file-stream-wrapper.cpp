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

#include "hphp/runtime/base/file-stream-wrapper.h"
#include "hphp/runtime/base/file-repository.h"
#include "hphp/runtime/base/runtime-error.h"
#include "hphp/runtime/base/plain-file.h"
#include "hphp/runtime/base/directory.h"
#include "hphp/runtime/server/static-content-cache.h"
#include "hphp/system/constants.h"
#include "hphp/util/file-util.h"
#include <memory>

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

MemFile* FileStreamWrapper::openFromCache(const String& filename,
                                          const String& mode) {
  if (!StaticContentCache::TheFileCache) {
    return nullptr;
  }

  String relative =
    FileCache::GetRelativePath(File::TranslatePath(filename).c_str());
  std::unique_ptr<MemFile> file(NEWOBJ(MemFile)());
  bool ret = file->open(relative, mode);
  if (ret) {
    return file.release();
  }
  return nullptr;
}

File* FileStreamWrapper::open(const String& filename, const String& mode,
                              int options, const Variant& context) {
  String fname =
    !strncmp(filename.data(), "file://", sizeof("file://") - 1)
    ? filename.substr(sizeof("file://") - 1) : filename;

  if (MemFile *file = openFromCache(fname, mode)) {
    return file;
  }

  if (options & File::USE_INCLUDE_PATH) {
    struct stat s;
    String resolved_fname = Eval::resolveVmInclude(fname.get(), "", &s);
    if (!resolved_fname.isNull()) {
        fname = resolved_fname;
    }
  }

  std::unique_ptr<PlainFile> file(NEWOBJ(PlainFile)());
  bool ret = file->open(File::TranslatePath(fname), mode);
  if (!ret) {
    raise_warning("%s", file->getLastError().c_str());
    return nullptr;
  }
  return file.release();
}

Directory* FileStreamWrapper::opendir(const String& path) {
  std::unique_ptr<PlainDirectory> dir(
    NEWOBJ(PlainDirectory)(File::TranslatePath(path))
  );
  if (!dir->isValid()) {
    raise_warning("%s", dir->getLastError().c_str());
    return nullptr;
  }
  return dir.release();
}

int FileStreamWrapper::rename(const String& oldname, const String& newname) {
  int ret =
    RuntimeOption::UseDirectCopy ?
      FileUtil::directRename(File::TranslatePath(oldname).data(),
                             File::TranslatePath(newname).data())
                                 :
      FileUtil::rename(File::TranslatePath(oldname).data(),
                       File::TranslatePath(newname).data());
  return ret;
}

int FileStreamWrapper::mkdir(const String& path, int mode, int options) {
  if (options & k_STREAM_MKDIR_RECURSIVE)
    return mkdir_recursive(path, mode);
  return ::mkdir(File::TranslatePath(path).data(), mode);
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
    if (*p == '/') {
      *p = '\0';
      if (::access(dir, F_OK) < 0) {
        if (::mkdir(dir, mode) < 0) {
          return -1;
        }
      }
      *p = '/';
    }
  }

  if (::access(dir, F_OK) < 0) {
    if (::mkdir(dir, mode) < 0) {
      return -1;
    }
  }

  return 0;
}

///////////////////////////////////////////////////////////////////////////////
}
