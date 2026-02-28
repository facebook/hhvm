/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <optional>
#include <vector>
#include "watchman/Clock.h"
#include "watchman/fs/FileInformation.h"
#include "watchman/watchman_string.h"

namespace watchman {

struct NotSymlink {};
using ResolvedSymlink = std::variant<NotSymlink, w_string>;

// A View-independent way of accessing file properties in the
// query engine.  A FileResult is not intended to be accessed
// concurrently from multiple threads and may be unsafe to
// be used in that manner (there is no implied locking).
class FileResult {
 public:
  virtual ~FileResult();

  // Maybe returns the file information.
  // Returns folly::none if the file information is not yet known.
  virtual std::optional<FileInformation> stat() = 0;

  // Returns the stat.st_atime field
  virtual std::optional<struct timespec> accessedTime() = 0;

  // Returns the stat.st_mtime field
  virtual std::optional<struct timespec> modifiedTime() = 0;

  // Returns the stat.st_ctime field
  virtual std::optional<struct timespec> changedTime() = 0;

  // Returns the size of the file in bytes, as reported in
  // the stat.st_size field.
  virtual std::optional<size_t> size() = 0;

  // Returns the name of the file in its containing dir
  virtual w_string_piece baseName() = 0;
  // Returns the name of the containing dir relative to the
  // VFS root
  virtual w_string_piece dirName() = 0;

  // Maybe return the file existence status.
  // Returns folly::none if the information is not currently known.
  virtual std::optional<bool> exists() = 0;

  // Returns the symlink target
  virtual std::optional<ResolvedSymlink> readLink() = 0;

  // Maybe return the change time.
  // Returns folly::none if ctime is not currently known
  virtual std::optional<ClockStamp> ctime() = 0;

  // Maybe return the observed time.
  // Returns folly::none if otime is not currently known
  virtual std::optional<ClockStamp> otime() = 0;

  // Returns the SHA-1 hash of the file contents
  using ContentHash = std::array<uint8_t, 20>;
  virtual std::optional<ContentHash> getContentSha1() = 0;

  // Maybe return the dtype.
  // Returns folly::none if the dtype is not currently known.
  // Returns DType::Unknown if we have dtype data but it doesn't
  // tell us the dtype (this is common on some older filesystems
  // on linux).
  virtual std::optional<DType> dtype();

  // A bitset of Property values
  using Properties = uint_least16_t;

  // Represents one of the FileResult fields.
  // Values are such that these can be bitwise OR'd to
  // produce a value of type `Properties` representing
  // multiple properties
  enum Property : Properties {
    // No specific fields required
    None = 0,
    // The dirName() and/or baseName() methods will be called
    Name = 1 << 0,
    // Need the mtime/ctime data returned by stat(2).
    StatTimeStamps = 1 << 1,
    // Need only enough information to distinguish between
    // file types, not the full mode information.
    FileDType = 1 << 2,
    // The ctime() method will be called
    CTime = 1 << 3,
    // The otime() method will be called
    OTime = 1 << 4,
    // The getContentSha1() method will be called
    ContentSha1 = 1 << 5,
    // The exists() method will be called
    Exists = 1 << 6,
    // Will need size information.
    Size = 1 << 7,
    // the readLink() method will be called
    SymlinkTarget = 1 << 8,
    // Need full stat metadata
    FullFileInformation = 1 << 9,
  };

  // Perform a batch fetch to fill in some missing data.
  // `files` is the set of FileResult instances that need more
  // data; their individual neededProperties_ values describes
  // the set of data that is needed.
  // `files` are assumed to all be of the same FileResult descendant,
  // and this is guaranteed by the current implementation.
  // When batchFetchProperties is called, it is invoked on one of
  // the elements of `files`.
  // The expectation is that the implementation of `batchFetchProperties`
  // will perform whatever actions are necessary to ensure that
  // a subsequent attempt to evaluate `neededProperties_` against each
  // member of `files` will not result in adding any of
  // those `FileResult` instances in being added to a deferred
  // batch.
  // The implementation of batchFetchProperties must clear
  // neededProperties_ to None.
  virtual void batchFetchProperties(
      const std::vector<std::unique_ptr<FileResult>>& files) = 0;

 protected:
  // To be called by one of the FileResult accessors when it needs
  // to record which properties are required to satisfy the request.
  void accessorNeedsProperties(Properties properties) {
    neededProperties_ |= properties;
  }

  // Clear any recorded needed properties
  void clearNeededProperties() {
    neededProperties_ = Property::None;
  }

  // Return the set of needed properties
  Properties neededProperties() const {
    return neededProperties_;
  }

 private:
  // The implementation of FileResult will set appropriate
  // bits in neededProperties_ when its accessors are called
  // and the associated data is not available.
  Properties neededProperties_{Property::None};
};

} // namespace watchman
