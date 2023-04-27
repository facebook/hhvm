/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "watchman/fs/FileDescriptor.h"
#include <folly/ScopeGuard.h>
#include <folly/String.h>
#include <folly/Try.h>
#include <folly/system/Pid.h>
#include <system_error>
#include "watchman/fs/FSDetect.h"
#include "watchman/fs/FileInformation.h"
#include "watchman/fs/WindowsTime.h"
#include "watchman/watchman_string.h"

#ifdef __APPLE__
#include <sys/attr.h> // @manual
#include <sys/utsname.h> // @manual
#include <sys/vnode.h> // @manual
#endif

#ifdef _WIN32
#include <winioctl.h> // @manual
#include <winsock2.h> // @manual
#endif

namespace watchman {

FileDescriptor::~FileDescriptor() {
  close();
}

FileDescriptor::system_handle_type FileDescriptor::normalizeHandleValue(
    system_handle_type h) {
#ifdef _WIN32
  // Windows uses both 0 and INVALID_HANDLE_VALUE as invalid handle values.
  if (h == intptr_t(INVALID_HANDLE_VALUE) || h == 0) {
    return FileDescriptor::kInvalid;
  }
#else
  // Posix defines -1 to be an invalid value, but we'll also recognize and
  // normalize any negative descriptor value.
  if (h < 0) {
    return FileDescriptor::kInvalid;
  }
#endif
  return h;
}

FileDescriptor::FileDescriptor(
    FileDescriptor::system_handle_type fd,
    FDType fdType)
    : fd_(normalizeHandleValue(fd)), fdType_(resolveFDType(fd, fdType)) {}

FileDescriptor::FileDescriptor(
    FileDescriptor::system_handle_type fd,
    const char* operation,
    FDType fdType)
    : fd_(normalizeHandleValue(fd)), fdType_(resolveFDType(fd, fdType)) {
  if (fd_ == kInvalid) {
    throw std::system_error(
        errno,
        std::generic_category(),
        std::string(operation) + ": " + folly::errnoStr(errno));
  }
}

FileDescriptor::FileDescriptor(FileDescriptor&& other) noexcept
    : fd_(other.release()), fdType_(other.fdType_) {}

FileDescriptor& FileDescriptor::operator=(FileDescriptor&& other) noexcept {
  close();
  fd_ = other.fd_;
  fdType_ = other.fdType_;
  other.fd_ = kInvalid;
  return *this;
}

void FileDescriptor::close() {
  if (fd_ != kInvalid) {
#ifndef _WIN32
    ::close(fd_);
#else
    if (fdType_ == FDType::Socket) {
      ::closesocket(fd_);
    } else {
      CloseHandle((HANDLE)fd_);
    }
#endif
    fd_ = kInvalid;
  }
}

FileDescriptor::system_handle_type FileDescriptor::release() {
  system_handle_type result = fd_;
  fd_ = kInvalid;
  return result;
}

FileDescriptor::FDType FileDescriptor::resolveFDType(
    FileDescriptor::system_handle_type fd,
    FDType fdType) {
  if (normalizeHandleValue(fd) == kInvalid) {
    return FDType::Unknown;
  }

  if (fdType != FDType::Unknown) {
    return fdType;
  }

#ifdef _WIN32
  if (GetFileType((HANDLE)fd) == FILE_TYPE_PIPE) {
    // It may be a pipe or a socket.
    // We can decide by asking for the underlying pipe
    // information; anonymous pipes are implemented on
    // top of named pipes so it is fine to use this function:
    DWORD flags = 0;
    DWORD out = 0;
    DWORD in = 0;
    DWORD inst = 0;
    if (GetNamedPipeInfo((HANDLE)fd, &flags, &out, &in, &inst) != 0) {
      return FDType::Pipe;
    }

    // We believe it to be a socket managed by winsock because it wasn't
    // a pipe.  However, when using pipes between WSL and native win32
    // we get here and the handle isn't recognized by winsock either.
    // Let's ask it for the error associated with the handle; if winsock
    // disavows it then we know it isn't a pipe or a socket, but we don't
    // know precisely what it is.
    int err = 0;
    int errsize = sizeof(err);
    if (::getsockopt(
            fd,
            SOL_SOCKET,
            SO_ERROR,
            reinterpret_cast<char*>(&err),
            &errsize) &&
        WSAGetLastError() == WSAENOTSOCK) {
      return FDType::Generic;
    }

    return FDType::Socket;
  }
#endif
  return FDType::Generic;
}

void FileDescriptor::setCloExec() {
#ifndef _WIN32
  ignore_result(fcntl(fd_, F_SETFD, FD_CLOEXEC));
#endif
}

void FileDescriptor::setNonBlock() {
#ifndef _WIN32
  ignore_result(fcntl(fd_, F_SETFL, fcntl(fd_, F_GETFL) | O_NONBLOCK));
#else
  if (fdType_ == FDType::Socket) {
    u_long mode = 1;
    ignore_result(::ioctlsocket(fd_, FIONBIO, &mode));
  }
#endif
}

void FileDescriptor::clearNonBlock() {
#ifndef _WIN32
  ignore_result(fcntl(fd_, F_SETFL, fcntl(fd_, F_GETFL) & ~O_NONBLOCK));
#else
  if (fdType_ == FDType::Socket) {
    u_long mode = 0;
    ignore_result(::ioctlsocket(fd_, FIONBIO, &mode));
  }
#endif
}

FileInformation FileDescriptor::getInfo() const {
#ifndef _WIN32
  struct stat st;
  if (fstat(fd_, &st)) {
    int err = errno;
    throw std::system_error(err, std::generic_category(), "fstat");
  }
  return FileInformation(st);
#else // _WIN32
  FILE_BASIC_INFO binfo;
  FILE_STANDARD_INFO sinfo;

  if (!GetFileInformationByHandleEx(
          (HANDLE)handle(), FileBasicInfo, &binfo, sizeof(binfo))) {
    throw std::system_error(
        GetLastError(),
        std::system_category(),
        "GetFileInformationByHandleEx FileBasicInfo");
  }

  FileInformation info(binfo.FileAttributes);

  FILETIME_LARGE_INTEGER_to_timespec(binfo.CreationTime, &info.ctime);
  FILETIME_LARGE_INTEGER_to_timespec(binfo.LastAccessTime, &info.atime);
  FILETIME_LARGE_INTEGER_to_timespec(binfo.LastWriteTime, &info.mtime);

  if (!GetFileInformationByHandleEx(
          (HANDLE)handle(), FileStandardInfo, &sinfo, sizeof(sinfo))) {
    throw std::system_error(
        GetLastError(),
        std::system_category(),
        "GetFileInformationByHandleEx FileStandardInfo");
  }

  info.size = sinfo.EndOfFile.QuadPart;
  info.nlink = sinfo.NumberOfLinks;

  return info;
#endif
}

w_string FileDescriptor::getOpenedPath() const {
#if defined(F_GETPATH)
  // macOS.  The kernel interface only allows MAXPATHLEN
  char buf[MAXPATHLEN + 1];
  if (fcntl(fd_, F_GETPATH, buf) == -1) {
    throw std::system_error(
        errno, std::generic_category(), "fcntl for getOpenedPath");
  }
  return w_string(buf);
#elif defined(__linux__) || defined(__sun)
  char procpath[1024];
#if defined(__linux__)
  snprintf(
      procpath,
      sizeof(procpath),
      "/proc/%d/fd/%d",
      folly::get_cached_pid(),
      fd_);
#elif defined(__sun)
  snprintf(
      procpath,
      sizeof(procpath),
      "/proc/%d/path/%d",
      folly::get_cached_pid(),
      fd_);
#endif

  // Avoid an extra stat by speculatively attempting to read into
  // a reasonably sized buffer.
  char buf[WATCHMAN_NAME_MAX];
  auto len = readlink(procpath, buf, sizeof(buf));
  if (len == sizeof(buf)) {
    len = -1;
    // We need to stat it to discover the required length
    errno = ENAMETOOLONG;
  }

  if (len >= 0) {
    return w_string(buf, len);
  }

  if (errno == ENOENT) {
    // For this path to not exist must mean that /proc is not mounted.
    // Report this with an actionable message
    throw std::system_error(
        ENOSYS,
        std::generic_category(),
        "getOpenedPath: need /proc to be mounted!");
  }

  if (errno != ENAMETOOLONG) {
    throw std::system_error(
        errno, std::generic_category(), "readlink for getOpenedPath");
  }

  // Figure out how much space we need
  struct stat st;
  if (fstat(fd_, &st)) {
    throw std::system_error(
        errno, std::generic_category(), "fstat for getOpenedPath");
  }
  std::string result;
  result.resize(st.st_size + 1, 0);

  len = readlink(procpath, &result[0], result.size());
  if (len == int(result.size())) {
    // It's longer than we expected; TOCTOU detected!
    throw std::system_error(
        ENAMETOOLONG,
        std::generic_category(),
        "readlinkat: link contents grew while examining file");
  }
  if (len >= 0) {
    return w_string(&result[0], len);
  }

  throw std::system_error(
      errno, std::generic_category(), "readlink for getOpenedPath");
#elif defined(_WIN32)
  std::wstring wchar;
  wchar.resize(WATCHMAN_NAME_MAX);
  auto len = GetFinalPathNameByHandleW(
      (HANDLE)fd_,
      &wchar[0],
      wchar.size(),
      FILE_NAME_NORMALIZED | VOLUME_NAME_DOS);
  auto err = GetLastError();

  if (len >= wchar.size()) {
    // Grow it
    wchar.resize(len);
    len = GetFinalPathNameByHandleW(
        (HANDLE)fd_, &wchar[0], len, FILE_NAME_NORMALIZED | VOLUME_NAME_DOS);
    err = GetLastError();
  }

  if (len == 0) {
    throw std::system_error(
        GetLastError(), std::system_category(), "GetFinalPathNameByHandleW");
  }

  return w_string(wchar.data(), len);
#else
  throw std::system_error(
      ENOSYS,
      std::generic_category(),
      "getOpenedPath not implemented on this platform");
#endif
}

w_string FileDescriptor::readSymbolicLink() const {
#ifndef _WIN32
  struct stat st;
  if (fstat(fd_, &st)) {
    throw std::system_error(
        errno, std::generic_category(), "fstat for readSymbolicLink");
  }
  std::string result;
  result.resize(st.st_size + 1, 0);

#ifdef __linux__
  // Linux 2.6.39 and later provide this interface
  auto atlen = readlinkat(fd_, "", &result[0], result.size());
  if (atlen == int(result.size())) {
    // It's longer than we expected; TOCTOU detected!
    throw std::system_error(
        ENAMETOOLONG,
        std::generic_category(),
        "readlinkat: link contents grew while examining file");
  }
  if (atlen >= 0) {
    return w_string(result.data(), atlen);
  }
  // if we get ENOTDIR back then we're probably on an older linux and
  // should fall back to the technique used below.
  if (errno != ENOTDIR) {
    throw std::system_error(
        errno, std::generic_category(), "readlinkat for readSymbolicLink");
  }
#endif

  auto myName = getOpenedPath();
  auto len = readlink(myName.c_str(), &result[0], result.size());
  if (len == int(result.size())) {
    // It's longer than we expected; TOCTOU detected!
    throw std::system_error(
        ENAMETOOLONG,
        std::generic_category(),
        "readlink: link contents grew while examining file");
  }
  if (len >= 0) {
    return w_string(result.data(), len);
  }

  throw std::system_error(
      errno, std::generic_category(), "readlink for readSymbolicLink");
#else // _WIN32
  auto rep = facebook::eden::getReparseData((HANDLE)fd_).value();
  WCHAR* target;
  USHORT targetlen;
  switch (rep->ReparseTag) {
    case IO_REPARSE_TAG_SYMLINK:
      target = rep->SymbolicLinkReparseBuffer.PathBuffer +
          (rep->SymbolicLinkReparseBuffer.SubstituteNameOffset / sizeof(WCHAR));
      targetlen =
          rep->SymbolicLinkReparseBuffer.SubstituteNameLength / sizeof(WCHAR);
      break;

    case IO_REPARSE_TAG_MOUNT_POINT:
      target = rep->MountPointReparseBuffer.PathBuffer +
          (rep->MountPointReparseBuffer.SubstituteNameOffset / sizeof(WCHAR));
      targetlen =
          rep->MountPointReparseBuffer.SubstituteNameLength / sizeof(WCHAR);
      break;
    default:
      throw std::system_error(
          ENOSYS, std::generic_category(), "Unsupported ReparseTag");
  }

  return w_string(target, targetlen);
#endif
}

bool FileDescriptor::isatty() const {
#ifdef _WIN32
  return GetFileType(reinterpret_cast<HANDLE>(fd_)) == FILE_TYPE_CHAR;
#else
  return ::isatty(fd_);
#endif
}

Result<int, std::error_code> FileDescriptor::read(void* buf, int size) const {
#ifndef _WIN32
  auto result = ::read(fd_, buf, size);
  if (result == -1) {
    int errcode = errno;
    return Result<int, std::error_code>(
        std::error_code(errcode, std::generic_category()));
  }
  return Result<int, std::error_code>(result);
#else
  if (fdType_ == FDType::Socket) {
    auto result = ::recv(fd_, static_cast<char*>(buf), size, 0);
    if (result == -1) {
      int errcode = WSAGetLastError();
      return Result<int, std::error_code>(
          std::error_code(errcode, std::system_category()));
    }
    return Result<int, std::error_code>(result);
  }

  DWORD result = 0;
  if (!ReadFile((HANDLE)fd_, buf, size, &result, nullptr)) {
    auto err = GetLastError();
    if (err == ERROR_BROKEN_PIPE) {
      // Translate broken pipe on read to EOF
      result = 0;
    } else {
      return Result<int, std::error_code>(
          std::error_code(err, std::system_category()));
    }
  }
  return Result<int, std::error_code>(result);
#endif
}

Result<int, std::error_code> FileDescriptor::write(const void* buf, int size)
    const {
#ifndef _WIN32
  auto result = ::write(fd_, buf, size);
  if (result == -1) {
    int errcode = errno;
    return Result<int, std::error_code>(
        std::error_code(errcode, std::generic_category()));
  }
  return Result<int, std::error_code>(result);
#else
  if (fdType_ == FDType::Socket) {
    auto result = ::send(fd_, static_cast<const char*>(buf), size, 0);
    if (result == -1) {
      int errcode = WSAGetLastError();
      return Result<int, std::error_code>(
          std::error_code(errcode, std::system_category()));
    }
    return Result<int, std::error_code>(result);
  }
  DWORD result = 0;
  if (!WriteFile((HANDLE)fd_, buf, size, &result, nullptr)) {
    return Result<int, std::error_code>(
        std::error_code(GetLastError(), std::system_category()));
  }
  return Result<int, std::error_code>(result);
#endif
}

const FileDescriptor& FileDescriptor::stdIn() {
  static FileDescriptor f(
#ifdef _WIN32
      intptr_t(GetStdHandle(STD_INPUT_HANDLE))
#else
      STDIN_FILENO
#endif
          ,
      FileDescriptor::FDType::Unknown);
  return f;
}

const FileDescriptor& FileDescriptor::stdOut() {
  static FileDescriptor f(
#ifdef _WIN32
      intptr_t(GetStdHandle(STD_OUTPUT_HANDLE))
#else
      STDOUT_FILENO
#endif
          ,
      FileDescriptor::FDType::Unknown);
  return f;
}

const FileDescriptor& FileDescriptor::stdErr() {
  static FileDescriptor f(
#ifdef _WIN32
      intptr_t(GetStdHandle(STD_ERROR_HANDLE))
#else
      STDERR_FILENO
#endif
          ,
      FileDescriptor::FDType::Unknown);
  return f;
}

#ifdef _WIN32

ULONG FileDescriptor::getReparseTag() const {
  return facebook::eden::getReparseData((HANDLE)fd_).value()->ReparseTag;
}
#endif
} // namespace watchman
