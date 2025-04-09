/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "watchman/XattrUtils.h"
#include <folly/Exception.h>
#include <folly/String.h>
#include "watchman/GroupLookup.h"
#include "watchman/Logging.h"

#ifdef __linux__
#include <sys/xattr.h>
#endif

namespace watchman {

#ifdef __linux__
// The following are lifted from sys/acl.h and inlined to avoid that
// dependency
namespace {

#define ACL_EA_ACCESS "system.posix_acl_access"
#define ACL_EA_VERSION 0x0002

// ACL permission bits
#define ACL_READ (0x04)
#define ACL_WRITE (0x02)
#define ACL_EXECUTE (0x01)

// ACL tag types
#define ACL_UNDEFINED_TAG (0x00)
#define ACL_USER_OBJ (0x01)
#define ACL_USER (0x02)
#define ACL_GROUP_OBJ (0x04)
#define ACL_GROUP (0x08)
#define ACL_MASK (0x10)
#define ACL_OTHER (0x20)

// ACL qualifier constants
#define ACL_UNDEFINED_ID ((id_t) - 1)

// Endianness conversion functions
#if __BYTE_ORDER == __BIG_ENDIAN
#define cpu_to_le16(w16) le16_to_cpu(w16)
#define le16_to_cpu(w16) \
  ((u_int16_t)((u_int16_t)(w16) >> 8) | (u_int16_t)((u_int16_t)(w16) << 8))
#define cpu_to_le32(w32) le32_to_cpu(w32)
#define le32_to_cpu(w32)                             \
  ((u_int32_t)((u_int32_t)(w32) >> 24) |             \
   (u_int32_t)(((u_int32_t)(w32) >> 8) & 0xFF00) |   \
   (u_int32_t)(((u_int32_t)(w32) << 8) & 0xFF0000) | \
   (u_int32_t)((u_int32_t)(w32) << 24))
#elif __BYTE_ORDER == __LITTLE_ENDIAN
#define cpu_to_le16(w16) ((u_int16_t)(w16))
#define le16_to_cpu(w16) ((u_int16_t)(w16))
#define cpu_to_le32(w32) ((u_int32_t)(w32))
#define le32_to_cpu(w32) ((u_int32_t)(w32))
#else
#error unknown endianness?
#endif

// ACL data structures
struct acl_ea_entry {
  u_int16_t e_tag;
  u_int16_t e_perm;
  u_int32_t e_id;
};

struct acl_ea_header {
  u_int32_t a_version;
  acl_ea_entry a_entries[0];
};
} // namespace
#endif

bool setSecondaryGroupACL(
    const char* path,
    const char* secondary_group_name,
    bool read,
    bool write,
    bool execute) {
#ifdef __linux__
  // Get the secondary group's gid
  const struct group* sec_group = w_get_group(secondary_group_name);
  if (!sec_group) {
    logf(ERR, "failed to get group for {}: {}\n", path, folly::errnoStr(errno));
    return false;
  }

  // Get the path's current permissions
  struct stat st {};
  if (lstat(path, &st) != 0) {
    logf(ERR, "Failed to lstat {}: {}\n", path, folly::errnoStr(errno));
    return false;
  }

  // setxattr doesn't like when just GROUP is supplied, so we will provide
  // USER_OBJ, GROUP_OBJ, GROUP, MASK, and OTHER (based off of the path's
  // current permissions)
  const size_t acl_entry_count = 5;

  // Allocate the memory for the ACL data
  size_t acl_size =
      sizeof(acl_ea_header) + (acl_entry_count * sizeof(acl_ea_entry));
  char* ext_acl_p = (char*)malloc(acl_size);

  if (!ext_acl_p) {
    logf(
        ERR,
        "failed to allocate memory for extended attribue: {}\n",
        folly::errnoStr(errno));
    return false;
  }

  // Set up the header
  acl_ea_header* header = (acl_ea_header*)ext_acl_p;
  header->a_version = cpu_to_le32(ACL_EA_VERSION);

  // Set up the 5 entries
  acl_ea_entry* ext_entry = (acl_ea_entry*)(header + 1);

  // USER_OBJ entry
  uint16_t user_perm = ((st.st_mode & S_IRUSR) ? ACL_READ : 0) |
      ((st.st_mode & S_IWUSR) ? ACL_WRITE : 0) |
      ((st.st_mode & S_IXUSR) ? ACL_EXECUTE : 0);
  ext_entry->e_tag = cpu_to_le16(ACL_USER_OBJ);
  ext_entry->e_perm = cpu_to_le16(user_perm);
  ext_entry->e_id = ACL_UNDEFINED_ID;
  ext_entry++;

  // GROUP_OBJ entry
  uint16_t group_perm = ((st.st_mode & S_IRGRP) ? ACL_READ : 0) |
      ((st.st_mode & S_IWGRP) ? ACL_WRITE : 0) |
      ((st.st_mode & S_IXGRP) ? ACL_EXECUTE : 0);
  ext_entry->e_tag = cpu_to_le16(ACL_GROUP_OBJ);
  ext_entry->e_perm = cpu_to_le16(group_perm);
  ext_entry->e_id = ACL_UNDEFINED_ID;
  ext_entry++;

  // ACL_GROUP entry (secondary group)
  uint16_t sec_group_perm = (read ? ACL_READ : 0) | (write ? ACL_WRITE : 0) |
      (execute ? ACL_EXECUTE : 0);
  ext_entry->e_tag = cpu_to_le16(ACL_GROUP);
  ext_entry->e_perm = cpu_to_le16(sec_group_perm);
  ext_entry->e_id = cpu_to_le32(sec_group->gr_gid);
  ext_entry++;

  // MASK entry (calculated using the file's existing group permisisons and the
  // secondary group permissions)
  uint16_t mask_perm = group_perm | sec_group_perm;
  ext_entry->e_tag = cpu_to_le16(ACL_MASK);
  ext_entry->e_perm = cpu_to_le16(mask_perm);
  ext_entry->e_id = ACL_UNDEFINED_ID;
  ext_entry++;

  // OTHER entry
  uint16_t other_perm = ((st.st_mode & S_IROTH) ? ACL_READ : 0) |
      ((st.st_mode & S_IWOTH) ? ACL_WRITE : 0) |
      ((st.st_mode & S_IXOTH) ? ACL_EXECUTE : 0);
  ext_entry->e_tag = cpu_to_le16(ACL_OTHER);
  ext_entry->e_perm = cpu_to_le16(other_perm);
  ext_entry->e_id = ACL_UNDEFINED_ID;

  // Finally, set the ACL xattr on the path and free the allocated memory
  int ret = setxattr(path, ACL_EA_ACCESS, ext_acl_p, acl_size, 0);
  free(ext_acl_p);

  if (ret != 0) {
    logf(ERR, "failed to set ACL for {} : {}\n", path, folly::errnoStr(errno));
    return false;
  }

  return true;
#else
  (void)path;
  (void)secondary_group_name;
  (void)read;
  (void)write;
  (void)execute;
  log(ERR, "setSecondaryGroupACL() is only implemented on Linux\n");
  return false;
#endif
}

} // namespace watchman
