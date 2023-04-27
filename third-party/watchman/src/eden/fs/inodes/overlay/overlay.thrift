/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This software may be used and distributed according to the terms of the
 * GNU General Public License version 2.
 */

namespace cpp2 facebook.eden.overlay
namespace py facebook.eden.overlay

typedef binary Hash
typedef string PathComponent
typedef string RelativePath

// An entry can be in one of three states:
//
// 1. Non-materialized, unknown inode number
// 2. Non-materialized, known inode number
// 3. Materialized (inode number must be known)
//
// Eventually, once legacy data has been migrated, only states #2 and #3 will
// occur. All tree entries will be given an inode number upon allocation,
// regardless of whether the entry exists in the overlay.

struct OverlayEntry {
  // Holds the mode_t data, which encodes the file type and permissions
  // Note: eventually this data will be obsoleted by the InodeMetadata table.
  1: i32 mode;
  // The child's inode number.  Until legacy data is migrated, this may be zero
  // or unset.  It should never be the case that hash is unset (indicating
  // materialized) and inodeNumber is zero or unset.
  2: i64 inodeNumber;
  // If not materialized, then this child is identical to an existing
  // source control Tree or Blob.  This contains the hash of that Tree or Blob.
  // If materialized, the hash is either unset or has zero length.
  3: optional Hash hash;
}

struct OverlayDir {
  // The contents of this dir.
  1: map<PathComponent, OverlayEntry> entries;
}
