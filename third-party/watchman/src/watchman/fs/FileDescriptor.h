/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once
#include <system_error>
#include "eden/common/utils/FileUtils.h"
#include "watchman/Result.h"
#include "watchman/watchman_system.h"

class w_string;

namespace watchman {

struct FileInformation;
struct REPARSE_DATA_BUFFER;

enum class CaseSensitivity {
  // The caller knows that the filesystem path(s) in question are
  // case insensitive.
  CaseInSensitive,
  // The caller knows that the filesystem path(s) in question are
  // case sensitive.
  CaseSensitive,
  // The caller does not know if the path(s) are case sensitive
  Unknown,
};

/** Windows doesn't have equivalent bits for all of the various
 * open(2) flags, so we abstract it out here */
struct OpenFileHandleOptions {
  unsigned followSymlinks : 1; // O_NOFOLLOW
  unsigned closeOnExec : 1; // O_CLOEXEC
  unsigned metaDataOnly : 1; // avoid accessing file contents
  unsigned readContents : 1; // the read portion of O_RDONLY or O_RDWR
  unsigned writeContents : 1; // the write portion of O_WRONLY or O_RDWR
  unsigned create : 1; // O_CREAT
  unsigned exclusiveCreate : 1; // O_EXCL
  unsigned truncate : 1; // O_TRUNC
  unsigned strictNameChecks : 1;
  CaseSensitivity caseSensitive;

  OpenFileHandleOptions()
      : followSymlinks(0),
        closeOnExec(1),
        metaDataOnly(0),
        readContents(0),
        writeContents(0),
        create(0),
        exclusiveCreate(0),
        truncate(0),
        strictNameChecks(1),
        caseSensitive(CaseSensitivity::Unknown) {}

  static inline OpenFileHandleOptions queryFileInfo() {
    OpenFileHandleOptions opts;
    opts.metaDataOnly = 1;
    return opts;
  }

  static inline OpenFileHandleOptions openDir() {
    OpenFileHandleOptions opts;
    opts.readContents = 1;
    opts.strictNameChecks = false;
    opts.followSymlinks = 1;
    return opts;
  }

  static inline OpenFileHandleOptions strictOpenDir() {
    OpenFileHandleOptions opts;
    opts.readContents = 1;
    opts.strictNameChecks = true;
    opts.followSymlinks = 0;
    return opts;
  }
};

// Manages the lifetime of a system independent file descriptor.
// On POSIX systems this is a posix file descriptor.
// On Win32 systems this is a Win32 HANDLE object.
// It will close() the descriptor when it is destroyed.
class FileDescriptor {
 public:
  using system_handle_type =
#ifdef _WIN32
      // We track the HANDLE value as intptr_t to avoid needing
      // to pull in the windows header files all over the place;
      // this is consistent with the _get_osfhandle function in
      // the msvcrt library.
      intptr_t
#else
      int
#endif
      ;

  enum class FDType {
    Unknown,
    Generic,
    Pipe,
    Socket,
  };

  // A value representing the canonical invalid handle
  // value for the system.
  static constexpr system_handle_type kInvalid = -1;

  // Normalizes invalid handle values to our canonical invalid handle value.
  // Otherwise, just returns the handle as-is.
  static system_handle_type normalizeHandleValue(system_handle_type h);

  // If the FDType is Unknown, probe it to determine its type
  static FDType resolveFDType(system_handle_type h, FDType fdType);

  ~FileDescriptor();

  // Default construct to an empty instance
  FileDescriptor() = default;

  // Construct a file descriptor object from an fd.
  // Will happily accept an invalid handle value without
  // raising an error; the FileDescriptor will simply evaluate as
  // false in a boolean context.
  explicit FileDescriptor(system_handle_type fd, FDType fdType);

  // Construct a file descriptor object from an fd.
  // If fd is invalid will throw a generic error with a message
  // constructed from the provided operation name and the current
  // errno value.
  FileDescriptor(system_handle_type fd, const char* operation, FDType fdType);

  // No copying
  FileDescriptor(const FileDescriptor&) = delete;
  FileDescriptor& operator=(const FileDescriptor&) = delete;

  FileDescriptor(FileDescriptor&& other) noexcept;
  FileDescriptor& operator=(FileDescriptor&& other) noexcept;

  // Closes the associated descriptor
  void close();

  // Stops tracking the descriptor, returning it to the caller.
  // The caller is then responsible for closing it.
  system_handle_type release();

  // In a boolean context, returns true if this object owns
  // a valid descriptor.
  explicit operator bool() const {
    return fd_ != kInvalid;
  }

  // Returns the underlying descriptor value
  inline system_handle_type system_handle() const {
    return fd_;
  }

#ifndef _WIN32
  // Returns the descriptor value as a file descriptor.
  // This method is only present on posix systems to aid in
  // detecting non-portable use at compile time.
  inline int fd() const {
    return fd_;
  }
#else
  // Returns the descriptor value as a file handle.
  // This method is only present on win32 systems to aid in
  // detecting non-portable use at compile time.
  inline intptr_t handle() const {
    return fd_;
  }
#endif

  inline FDType fdType() const {
    return fdType_;
  }

  // Set the close-on-exec bit
  void setCloExec();

  // Enable non-blocking IO
  void setNonBlock();

  // Disable non-blocking IO
  void clearNonBlock();

  /** equivalent to fstat(2) */
  FileInformation getInfo() const;

  /** Returns the filename associated with the file handle */
  w_string getOpenedPath() const;

  /** Returns the symbolic link target */
  w_string readSymbolicLink() const;

  /**
   * Returns true if this FileDescriptor is connected to a tty device.
   * Calls isatty() on unix and GetFileType on Windows.
   */
  bool isatty() const;

  /** read(2), but yielding a Result for system independent error reporting */
  Result<int, std::error_code> read(void* buf, int size) const;

  /** write(2), but yielding a Result for system independent error reporting */
  Result<int, std::error_code> write(const void* buf, int size) const;

  // Return a global handle to one of the standard IO stream descriptors
  static const FileDescriptor& stdIn();
  static const FileDescriptor& stdOut();
  static const FileDescriptor& stdErr();

#ifdef _WIN32
  ULONG getReparseTag() const;
#endif

 private:
  system_handle_type fd_{kInvalid};
  FDType fdType_{FDType::Unknown};
};
} // namespace watchman
