/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "watchman/fs/FileSystem.h"
#include <fmt/core.h>
#include <folly/String.h>
#include "watchman/fs/FSDetect.h"
#include "watchman/portability/WinError.h"
#include "watchman/watchman_stream.h"
#include "watchman/watchman_string.h"

#ifdef __APPLE__
#include <sys/attr.h> // @manual
#include <sys/utsname.h> // @manual
#include <sys/vnode.h> // @manual
#endif

#if defined(_WIN32) || defined(O_PATH)
#define CAN_OPEN_SYMLINKS 1
#else
#define CAN_OPEN_SYMLINKS 0
#endif

namespace watchman {

namespace {

class RealFileSystem final : public FileSystem {
 public:
  std::unique_ptr<DirHandle> openDir(const char* path, bool strict = true)
      override {
    return watchman::openDir(path, strict);
  }

  FileInformation getFileInformation(
      const char* path,
      CaseSensitivity caseSensitive = CaseSensitivity::Unknown) override {
    return watchman::getFileInformation(path, caseSensitive);
  }

  /**
   * Watchman-specific API for creating an empty file on the filesystem.
   * On unix, the created file will have mode 0700.
   * Throws system_error on failure.
   */
  void touch(const char* path) override {
    if (!w_stm_open(path, O_CREAT | O_TRUNC | O_WRONLY | O_CLOEXEC, 0700)) {
      int err = errno;
      throw std::system_error{
          err,
          std::generic_category(),
          fmt::format("failed to touch {}: {}", path, folly::errnoStr(err))};
    }
  }
};

RealFileSystem gRealFileSystem;

} // namespace

FileSystem& realFileSystem = gRealFileSystem;

#if !CAN_OPEN_SYMLINKS

namespace {

/** Checks that the basename component of the input path exactly
 * matches the canonical case of the path on disk.
 * It only makes sense to call this function on a case insensitive filesystem.
 * If the case does not match, throws an exception. */
void checkCanonicalBaseName(const char* path) {
#ifdef __APPLE__
  struct attrlist attrlist;
  struct {
    uint32_t len;
    attrreference_t ref;
    char canonical_name[WATCHMAN_NAME_MAX];
  } vomit;
  w_string_piece pathPiece(path);
  auto base = pathPiece.baseName();

  memset(&attrlist, 0, sizeof(attrlist));
  attrlist.bitmapcount = ATTR_BIT_MAP_COUNT;
  attrlist.commonattr = ATTR_CMN_NAME;

  if (getattrlist(path, &attrlist, &vomit, sizeof(vomit), FSOPT_NOFOLLOW) ==
      -1) {
    throw std::system_error(
        errno,
        std::generic_category(),
        fmt::format("checkCanonicalBaseName({}): getattrlist failed", path));
  }

  w_string_piece name(((char*)&vomit.ref) + vomit.ref.attr_dataoffset);
  if (name != base) {
    throw std::system_error(
        ENOENT,
        std::generic_category(),
        fmt::format(
            "checkCanonicalBaseName({}): ({}) doesn't match canonical base ({})",
            path,
            name,
            base));
  }
#else
  // Older Linux and BSDish systems are in this category.
  // This is the awful portable fallback used in the absence of
  // a system specific way to detect this.
  w_string_piece pathPiece(path);
  auto parent = pathPiece.dirName().asWString();
  auto dir = realFileSystem.openDir(parent.c_str());
  auto base = pathPiece.baseName();

  while (true) {
    auto ent = dir->readDir();
    if (!ent) {
      // We didn't find an entry that exactly matched -> fail
      throw std::system_error(
          ENOENT,
          std::generic_category(),
          fmt::format(
              "checkCanonicalBaseName({}): no match found in parent dir",
              path));
    }
    // Note: we don't break out early if we get a case-insensitive match
    // because the dir may contain multiple representations of the same
    // name.  For example, Bash-for-Windows has dirs that contain both
    // "pod" and "Pod" dirs in its perl installation.  We want to make
    // sure that we've observed all of the entries in the dir before
    // giving up.
    if (w_string_piece(ent->d_name) == base) {
      // Exact match; all is good!
      return;
    }
  }
#endif
}
} // namespace
#endif

FileDescriptor openFileHandle(
    const char* path,
    const OpenFileHandleOptions& opts) {
#ifndef _WIN32
  int flags = (!opts.followSymlinks ? O_NOFOLLOW : 0) |
      (opts.closeOnExec ? O_CLOEXEC : 0) |
#ifdef O_PATH
      (opts.metaDataOnly ? O_PATH : 0) |
#endif
      ((opts.readContents && opts.writeContents)
           ? O_RDWR
           : (opts.writeContents      ? O_WRONLY
                  : opts.readContents ? O_RDONLY
                                      : 0)) |
      (opts.create ? O_CREAT : 0) | (opts.exclusiveCreate ? O_EXCL : 0) |
      (opts.truncate ? O_TRUNC : 0);

  auto fd = open(path, flags);
  if (fd == -1) {
    int err = errno;
    throw std::system_error(
        err, std::generic_category(), fmt::format("open: {}", path));
  }
  FileDescriptor file(fd, FileDescriptor::FDType::Unknown);
#else // _WIN32
  DWORD access = 0, share = 0, create = 0, attrs = 0;
  DWORD err;
  auto sec = SECURITY_ATTRIBUTES();

  if (!strcmp(path, "/dev/null")) {
    path = "NUL:";
  }

  auto wpath = w_string_piece(path).asWideUNC();

  if (opts.metaDataOnly) {
    access = 0;
  } else {
    if (opts.writeContents) {
      access |= GENERIC_WRITE;
    }
    if (opts.readContents) {
      access |= GENERIC_READ;
    }
  }

  // We want more posix-y behavior by default
  share = FILE_SHARE_DELETE | FILE_SHARE_READ | FILE_SHARE_WRITE;

  sec.nLength = sizeof(sec);
  sec.bInheritHandle = TRUE;
  if (opts.closeOnExec) {
    sec.bInheritHandle = FALSE;
  }

  if (opts.create && opts.exclusiveCreate) {
    create = CREATE_NEW;
  } else if (opts.create && opts.truncate) {
    create = CREATE_ALWAYS;
  } else if (opts.create) {
    create = OPEN_ALWAYS;
  } else if (opts.truncate) {
    create = TRUNCATE_EXISTING;
  } else {
    create = OPEN_EXISTING;
  }

  attrs = FILE_FLAG_POSIX_SEMANTICS | FILE_FLAG_BACKUP_SEMANTICS;
  if (!opts.followSymlinks) {
    attrs |= FILE_FLAG_OPEN_REPARSE_POINT;
  }

  FileDescriptor file(
      intptr_t(CreateFileW(
          wpath.c_str(), access, share, &sec, create, attrs, nullptr)),
      FileDescriptor::FDType::Unknown);
  err = GetLastError();
  if (!file) {
    throw std::system_error(
        err,
        std::system_category(),
        std::string("CreateFileW for openFileHandle: ") + path);
  }

#endif

  if (!opts.strictNameChecks) {
    return file;
  }

  auto opened = file.getOpenedPath();
  if (w_string_piece(opened).pathIsEqual(path)) {
#if !CAN_OPEN_SYMLINKS
    CaseSensitivity caseSensitive = opts.caseSensitive;
    if (caseSensitive == CaseSensitivity::Unknown) {
      caseSensitive = getCaseSensitivityForPath(path);
    }
    if (caseSensitive == CaseSensitivity::CaseInSensitive) {
      // We need to perform one extra check for case-insensitive
      // paths to make sure that we didn't accidentally open
      // the wrong case name.
      checkCanonicalBaseName(path);
    }
#endif
    return file;
  }

  throw std::system_error(
      ENOENT,
      std::generic_category(),
      fmt::format(
          "open({}): opened path doesn't match canonical path {}",
          path,
          opened));
}

FileInformation getFileInformation(
    const char* path,
    CaseSensitivity caseSensitive) {
  auto options = OpenFileHandleOptions::queryFileInfo();
  options.caseSensitive = caseSensitive;
#if defined(_WIN32) || defined(O_PATH)
  // These operating systems allow opening symlink nodes and querying them
  // for stat information
  auto handle = openFileHandle(path, options);
  auto info = handle.getInfo();
  return info;
#else
  // Since the leaf of the path may be a symlink, and this system doesn't
  // allow opening symlinks for stat purposes, we have to resort to performing
  // a relative fstatat() from the parent dir.
  w_string_piece pathPiece(path);
  auto parent = pathPiece.dirName().asWString();
  auto handle = openFileHandle(parent.c_str(), options);
  struct stat st;
  if (fstatat(
          handle.fd(), pathPiece.baseName().data(), &st, AT_SYMLINK_NOFOLLOW)) {
    throw std::system_error(errno, std::generic_category(), "fstatat");
  }

  if (caseSensitive == CaseSensitivity::Unknown) {
    caseSensitive = getCaseSensitivityForPath(path);
  }
  if (caseSensitive == CaseSensitivity::CaseInSensitive) {
    // We need to perform one extra check for case-insensitive
    // paths to make sure that we didn't accidentally open
    // the wrong case name.
    checkCanonicalBaseName(path);
  }

  return FileInformation(st);
#endif
}

#ifdef _WIN32
namespace {

w_string getCurrentDirectory() {
  WCHAR wchar[WATCHMAN_NAME_MAX];
  auto len = GetCurrentDirectoryW(std::size(wchar), wchar);
  auto err = GetLastError();
  // Technically, len > std::size(wchar) is sufficient, because the w_string
  // constructor below will add a trailing zero.
  if (len == 0 || len >= std::size(wchar)) {
    throw std::system_error(
        err, std::system_category(), "GetCurrentDirectoryW");
  }
  // Assumption: that the OS maintains the CWD in canonical form
  return w_string(wchar, len);
}

} // namespace
#endif

w_string realPath(const char* path) {
  auto options = OpenFileHandleOptions::queryFileInfo();
  // Follow symlinks, because that's really the point of this function
  options.followSymlinks = 1;
  options.strictNameChecks = 0;

#ifdef _WIN32
  // Special cases for cwd.
  // On Windows, "" is used to refer to the CWD.
  // We also allow using "." for parity with unix, even though that
  // doesn't generally work for that purpose on windows.
  // This allows `watchman watch-project .` to succeeed on windows.
  if (path[0] == 0 || (path[0] == '.' && path[1] == 0)) {
    return getCurrentDirectory();
  }
#endif

  auto handle = openFileHandle(path, options);
  return handle.getOpenedPath();
}

w_string readSymbolicLink(const char* path) {
#ifndef _WIN32
  std::string result;

  // Speculatively assume that this is large enough to read the
  // symlink text.  This helps to avoid an extra lstat call.
  result.resize(256);

  for (int retry = 0; retry < 2; ++retry) {
    auto len = readlink(path, &result[0], result.size());
    if (len < 0) {
      throw std::system_error(
          errno, std::generic_category(), "readlink for readSymbolicLink");
    }
    if (size_t(len) < result.size()) {
      return w_string(result.data(), len);
    }

    // Truncated read; we need to figure out the right size to use
    struct stat st;
    if (lstat(path, &st)) {
      throw std::system_error(
          errno, std::generic_category(), "lstat for readSymbolicLink");
    }

    result.resize(st.st_size + 1, 0);
  }

  throw std::system_error(
      E2BIG,
      std::generic_category(),
      "readlink for readSymbolicLink: symlink changed while reading it");
#else
  return openFileHandle(path, OpenFileHandleOptions::queryFileInfo())
      .readSymbolicLink();
#endif
}

bool isOnSameMount(
    const FileInformation& root,
    const FileInformation& file,
    const char* file_path) {
#ifdef _WIN32
  (void)root;
  if ((file.fileAttributes & FILE_ATTRIBUTE_REPARSE_POINT) ==
      FILE_ATTRIBUTE_REPARSE_POINT) {
    auto fd = openFileHandle(file_path, OpenFileHandleOptions::queryFileInfo());
    auto reparseTag = fd.getReparseTag();
    return reparseTag != IO_REPARSE_TAG_PROJFS;
  } else {
    return true;
  }
#else
  (void)file_path;
  return root.dev == file.dev;
#endif
}

} // namespace watchman

#ifdef _WIN32

int mkdir(const char* path, int) {
  auto wpath = w_string_piece(path).asWideUNC();
  DWORD err;
  BOOL res;

  res = CreateDirectoryW(wpath.c_str(), nullptr);
  err = GetLastError();

  if (res) {
    return 0;
  }
  errno = map_win32_err(err);
  return -1;
}

#endif
