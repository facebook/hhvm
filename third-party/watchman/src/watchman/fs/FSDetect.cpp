/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "watchman/fs/FSDetect.h"
#include <folly/FileUtil.h>
#include <folly/String.h>
#include "eden/common/utils/FSDetect.h"
#include "watchman/fs/FileDescriptor.h"
#include "watchman/watchman_system.h"

#ifdef HAVE_SYS_VFS_H
#include <sys/vfs.h>
#endif
#ifdef HAVE_SYS_STATVFS_H
#include <sys/statvfs.h>
#endif
#ifdef HAVE_SYS_PARAM_H
#include <sys/param.h>
#endif
#ifdef HAVE_SYS_MOUNT_H
#include <sys/mount.h>
#endif
#ifdef __linux__
#include <linux/magic.h>
#endif

using namespace watchman;

namespace watchman {

CaseSensitivity getCaseSensitivityForPath([[maybe_unused]] const char* path) {
#ifdef __APPLE__
  return pathconf(path, _PC_CASE_SENSITIVE) ? CaseSensitivity::CaseSensitive
                                            : CaseSensitivity::CaseInSensitive;
#elif defined(_WIN32)
  return CaseSensitivity::CaseInSensitive;
#else
  return CaseSensitivity::CaseSensitive;
#endif
}

} // namespace watchman

// This function is used to return the fstype for a given path
// based on the linux style /proc/mounts data provided.
// It will return nullptr for paths that don't have a match.
std::optional<w_string> find_fstype_in_linux_proc_mounts(
    std::string_view path,
    std::string_view procMountsData) {
  std::vector<std::string_view> lines;
  std::string_view bestMountPoint, bestVfsType;

  folly::split("\n", procMountsData, lines);
  for (auto& line : lines) {
    std::string_view device, mountPoint, vfstype, opts, freq, passno;
    if (folly::split(
            " ", line, device, mountPoint, vfstype, opts, freq, passno)) {
      // Look for the mountPoint that matches the longest prefix of path
      // we intentially bias towards the last matching path
      if (mountPoint.size() >= bestMountPoint.size() &&
          folly::StringPiece{path}.startsWith(mountPoint)) {
        if (
            // mount point matches path exactly
            path.size() == mountPoint.size() ||
            // prefix ends with a slash
            folly::StringPiece{mountPoint}.endsWith('/') ||
            // the byte after the mount point prefix is a slash and thus is
            // not an ambiguous prefix match
            (path.size() > mountPoint.size() &&
             path[mountPoint.size()] == '/')) {
          // This is a better match than any prior mount point
          bestMountPoint = mountPoint;

          if (vfstype == "fuse" || vfstype == "fuse.edenfs") {
            // For example: edenfs registers with fstype "fuse" or "fuse.edenfs"
            // and device "edenfs", so we take the device node
            // as the filesystem type
            bestVfsType = device;
          } else if (vfstype == "nfs") {
            // similar to fuse edenfs will register itself under the
            // device name. In general, we don't want watchman to be used
            // over nfs, so in all cases except eden we still use "nfs"
            // as the type.
            if (facebook::eden::is_edenfs_fs_type(device)) {
              bestVfsType = device;
            } else {
              bestVfsType = vfstype;
            }
          } else {
            // Most other fuse filesystems use libfuse which registers
            // with fstype names like "fuse.something", or we're working
            // with non-fuse filesystems and have a more meaningful
            // fstype reported anyway
            bestVfsType = vfstype;
          }
        }
      }
    }
  }

  if (bestVfsType.empty()) {
    return std::nullopt;
  }
  return w_string(bestVfsType.data(), bestVfsType.size());
}

w_string w_fstype_detect_macos_nfs(w_string fstype, w_string edenfs_indicator) {
  if (fstype == "nfs" &&
      facebook::eden::is_edenfs_fs_type(edenfs_indicator.string())) {
    return edenfs_indicator;
  }
  return fstype;
}

// The primary purpose of checking the filesystem type is to prevent
// watching filesystems that are known to be problematic, such as
// network or remote mounted filesystems.  As such, we don't strictly
// need to have a fully comprehensive mapping of the underlying filesystem
// type codes to names, just the known problematic types

w_string w_fstype([[maybe_unused]] const char* path) {
#ifdef __linux__
  // If possible, we prefer to read the filesystem type names from
  // `/proc/self/mounts`
  std::string mounts;
  if (folly::readFile("/proc/self/mounts", mounts)) {
    auto fstype = find_fstype_in_linux_proc_mounts(path, mounts);
    if (fstype) {
      return *fstype;
    }
  }

  // Reading the mount table can fail for the simple reason that `/proc` isn't
  // mounted, so fall back to some slightly manual code that looks at known
  // filesystem type ids.

  struct statfs sfs;
  const char* name = "unknown";

  // Unfortunately the FUSE magic number is not defined in linux/magic.h,
  // and is only available in the Linux source code in fs/fuse/inode.c
  constexpr __fsword_t FUSE_MAGIC_NUMBER = 0x65735546;

  if (statfs(path, &sfs) == 0) {
    switch (sfs.f_type) {
#ifdef CIFS_MAGIC_NUMBER
      case CIFS_MAGIC_NUMBER:
        name = "cifs";
        break;
#endif
#ifdef NFS_SUPER_MAGIC
      case NFS_SUPER_MAGIC:
        name = "nfs";
        break;
#endif
#ifdef SMB_SUPER_MAGIC
      case SMB_SUPER_MAGIC:
        name = "smb";
        break;
#endif
      case FUSE_MAGIC_NUMBER:
        name = "fuse";
        break;
      default:
        name = "unknown";
    }
  }

  return w_string(name, W_STRING_UNICODE);
#elif STATVFS_HAS_FSTYPE_AS_STRING
  // if this is going to be used on macos this needs
  // to detect edenfs with w_fstype_detect_macos_nfs
  struct statvfs sfs;

  if (statvfs(path, &sfs) == 0) {
#ifdef HAVE_STRUCT_STATVFS_F_FSTYPENAME
    return w_string(sfs.f_fstypename, W_STRING_UNICODE);
#endif
#ifdef HAVE_STRUCT_STATVFS_F_BASETYPE
    return w_string(sfs.f_basetype, W_STRING_UNICODE);
#endif
  }
#elif HAVE_STATFS
  struct statfs sfs;

  if (statfs(path, &sfs) == 0) {
    auto fstype = w_string(sfs.f_fstypename, W_STRING_UNICODE);
    auto edenfs_indicator = w_string(sfs.f_mntfromname, W_STRING_UNICODE);
    return w_fstype_detect_macos_nfs(fstype, edenfs_indicator);
  }
#endif
#ifdef _WIN32
  auto wpath = w_string_piece(path).asWideUNC();
  WCHAR fstype[MAX_PATH + 1];
  FileDescriptor h(
      intptr_t(CreateFileW(
          wpath.c_str(),
          GENERIC_READ,
          FILE_SHARE_DELETE | FILE_SHARE_READ | FILE_SHARE_WRITE,
          nullptr,
          OPEN_EXISTING,
          FILE_FLAG_BACKUP_SEMANTICS,
          nullptr)),
      FileDescriptor::FDType::Generic);
  if (h &&
      GetVolumeInformationByHandleW(
          (HANDLE)h.handle(), nullptr, 0, 0, 0, 0, fstype, MAX_PATH + 1)) {
    return w_string(fstype, wcslen(fstype));
  }
  return w_string("unknown", W_STRING_UNICODE);
#else
  return w_string("unknown", W_STRING_UNICODE);
#endif
}

/* vim:ts=2:sw=2:et:
 */
