/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "watchman/fs/DirHandle.h"

#include <folly/ScopeGuard.h>
#include "watchman/fs/FileDescriptor.h"
#include "watchman/fs/FileSystem.h"
#include "watchman/fs/WindowsTime.h"
#include "watchman/watchman_string.h"
#include "watchman/watchman_system.h"

namespace watchman {

#ifdef _WIN32

namespace {
class WinDirHandle : public DirHandle {
  std::wstring dirWPath_;
  FileDescriptor h_;
  bool win7_{false};
  FILE_FULL_DIR_INFO* info_{nullptr};
  char __declspec(align(8)) buf_[64 * 1024];
  HANDLE hDirFind_{nullptr};
  char nameBuf_[WATCHMAN_NAME_MAX];
  DirEntry ent_;

 public:
  ~WinDirHandle() {
    if (hDirFind_) {
      FindClose(hDirFind_);
    }
  }

  explicit WinDirHandle(const char* path, bool strict) {
    dirWPath_ = w_string_piece(path).asWideUNC();

    h_ = openFileHandle(
        path,
        strict ? OpenFileHandleOptions::strictOpenDir()
               : OpenFileHandleOptions::openDir());

    // Use Win7 compatibility mode for readDir()
    if (getenv("WATCHMAN_WIN7_COMPAT") &&
        getenv("WATCHMAN_WIN7_COMPAT")[0] == '1') {
      win7_ = true;
    }

    ent_ = DirEntry();
    ent_.d_name = nameBuf_;
    ent_.has_stat = true;
    if (path[1] == ':') {
      ent_.stat.dev = tolower(path[0]) - 'a';
    }
  }

  const DirEntry* readDir() override {
    if (win7_) {
      return readDirWin7();
    }
    try {
      return readDirWin8();
    } catch (const std::system_error& err) {
      if (err.code().value() != ERROR_INVALID_PARAMETER) {
        throw;
      }
      // Fallback on Win7 implementation. FileFullDirectoryInfo
      // parameter is not supported before Win8
      win7_ = true;
      return readDirWin7();
    }
  }

 private:
  const DirEntry* readDirWin8() {
    if (!info_) {
      if (!GetFileInformationByHandleEx(
              (HANDLE)h_.handle(), FileFullDirectoryInfo, buf_, sizeof(buf_))) {
        if (GetLastError() == ERROR_NO_MORE_FILES) {
          return nullptr;
        }
        throw std::system_error(
            GetLastError(),
            std::system_category(),
            "GetFileInformationByHandleEx");
      }
      info_ = (FILE_FULL_DIR_INFO*)buf_;
    }

    // Decode the item currently pointed at
    DWORD len = WideCharToMultiByte(
        CP_UTF8,
        0,
        info_->FileName,
        info_->FileNameLength / sizeof(WCHAR),
        nameBuf_,
        sizeof(nameBuf_) - 1,
        nullptr,
        nullptr);

    if (len <= 0) {
      throw std::system_error(
          GetLastError(), std::system_category(), "WideCharToMultiByte");
    }

    nameBuf_[len] = 0;

    // Populate stat info to speed up the crawler() routine
    ent_.stat = FileInformation(info_->FileAttributes);
    FILETIME_LARGE_INTEGER_to_timespec(info_->CreationTime, &ent_.stat.ctime);
    FILETIME_LARGE_INTEGER_to_timespec(info_->LastAccessTime, &ent_.stat.atime);
    FILETIME_LARGE_INTEGER_to_timespec(info_->LastWriteTime, &ent_.stat.mtime);
    ent_.stat.size = info_->EndOfFile.QuadPart;

    // Advance the pointer to the next entry ready for the next read
    info_ = info_->NextEntryOffset == 0
        ? nullptr
        : (FILE_FULL_DIR_INFO*)(((char*)info_) + info_->NextEntryOffset);

    return &ent_;
  }

  const DirEntry* readDirWin7() {
    // FileFullDirectoryInfo is not supported prior to Windows 8
    WIN32_FIND_DATAW findFileData;
    bool success;

    if (!hDirFind_) {
      std::wstring strWPath(dirWPath_);
      strWPath += L"\\*";

      hDirFind_ = FindFirstFileW(strWPath.c_str(), &findFileData);
      success = hDirFind_ != INVALID_HANDLE_VALUE;
    } else {
      success = FindNextFileW(hDirFind_, &findFileData);
    }
    if (!success) {
      if (GetLastError() == ERROR_NO_MORE_FILES) {
        return nullptr;
      }

      throw std::system_error(
          GetLastError(),
          std::system_category(),
          hDirFind_ ? "FindNextFileW" : "FindFirstFileW");
    }

    DWORD len = WideCharToMultiByte(
        CP_UTF8,
        0,
        findFileData.cFileName,
        -1,
        nameBuf_,
        sizeof(nameBuf_) - 1,
        nullptr,
        nullptr);

    if (len <= 0) {
      throw std::system_error(
          GetLastError(), std::system_category(), "WideCharToMultiByte");
    }

    nameBuf_[len] = 0;

    // Populate stat info to speed up the crawler() routine
    ent_.stat = FileInformation(findFileData.dwFileAttributes);
    FILETIME_to_timespec(&findFileData.ftCreationTime, &ent_.stat.ctime);
    FILETIME_to_timespec(&findFileData.ftLastAccessTime, &ent_.stat.atime);
    FILETIME_to_timespec(&findFileData.ftLastWriteTime, &ent_.stat.mtime);

    LARGE_INTEGER fileSize;
    fileSize.HighPart = findFileData.nFileSizeHigh;
    fileSize.LowPart = findFileData.nFileSizeLow;
    ent_.stat.size = fileSize.QuadPart;

    return &ent_;
  }
};
} // namespace

std::unique_ptr<DirHandle> openDir(const char* path, bool strict) {
  return std::make_unique<WinDirHandle>(path, strict);
}

#endif

} // namespace watchman
