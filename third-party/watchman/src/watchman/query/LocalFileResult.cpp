/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "watchman/query/LocalFileResult.h"
#include "watchman/ContentHash.h"

namespace watchman {

LocalFileResult::LocalFileResult(
    w_string fullPath,
    ClockStamp clock,
    CaseSensitivity caseSensitivity)
    : fullPath_(std::move(fullPath)),
      clock_(clock),
      caseSensitivity_(caseSensitivity) {}

void LocalFileResult::getInfo() {
  if (info_.has_value()) {
    return;
  }
  try {
    info_ = getFileInformation(fullPath_.c_str(), caseSensitivity_);
    exists_ = true;
  } catch (const std::exception&) {
    // Treat any error as effectively deleted
    exists_ = false;
    info_ = FileInformation::makeDeletedFileInformation();
  }
}

std::optional<FileInformation> LocalFileResult::stat() {
  if (!info_.has_value()) {
    accessorNeedsProperties(FileResult::Property::FullFileInformation);
    return std::nullopt;
  }
  return info_;
}

std::optional<size_t> LocalFileResult::size() {
  if (!info_.has_value()) {
    accessorNeedsProperties(FileResult::Property::Size);
    return std::nullopt;
  }
  return info_->size;
}

std::optional<struct timespec> LocalFileResult::accessedTime() {
  if (!info_.has_value()) {
    accessorNeedsProperties(FileResult::Property::StatTimeStamps);
    return std::nullopt;
  }
  return info_->atime;
}

std::optional<struct timespec> LocalFileResult::modifiedTime() {
  if (!info_.has_value()) {
    accessorNeedsProperties(FileResult::Property::StatTimeStamps);
    return std::nullopt;
  }
  return info_->mtime;
}

std::optional<struct timespec> LocalFileResult::changedTime() {
  if (!info_.has_value()) {
    accessorNeedsProperties(FileResult::Property::StatTimeStamps);
    return std::nullopt;
  }
  return info_->ctime;
}

w_string_piece LocalFileResult::baseName() {
  return w_string_piece(fullPath_).baseName();
}

w_string_piece LocalFileResult::dirName() {
  return w_string_piece(fullPath_).dirName();
}

std::optional<bool> LocalFileResult::exists() {
  if (!info_.has_value()) {
    accessorNeedsProperties(FileResult::Property::Exists);
    return std::nullopt;
  }
  return exists_;
}

std::optional<ResolvedSymlink> LocalFileResult::readLink() {
  if (symlinkTarget_.has_value()) {
    return symlinkTarget_;
  }
  accessorNeedsProperties(FileResult::Property::SymlinkTarget);
  return std::nullopt;
}

std::optional<ClockStamp> LocalFileResult::ctime() {
  return clock_;
}

std::optional<ClockStamp> LocalFileResult::otime() {
  return clock_;
}

std::optional<FileResult::ContentHash> LocalFileResult::getContentSha1() {
  if (contentSha1_.empty()) {
    accessorNeedsProperties(FileResult::Property::ContentSha1);
    return std::nullopt;
  }
  return contentSha1_.value();
}

void LocalFileResult::batchFetchProperties(
    const std::vector<std::unique_ptr<FileResult>>& files) {
  for (auto& f : files) {
    auto localFile = dynamic_cast<LocalFileResult*>(f.get());
    localFile->getInfo();

    if (localFile->neededProperties() & FileResult::Property::SymlinkTarget) {
      ResolvedSymlink target = NotSymlink{};
      // If this file is not a symlink then we immediately yield a "not a
      // symlink" rather than propagating an error. This behavior is relied
      // upon by the field rendering code and checked in test_symlink.py.
      if (localFile->info_->isSymlink()) {
        target = readSymbolicLink(localFile->fullPath_.c_str());
      }
      localFile->symlinkTarget_ = target;
    }

    if (localFile->neededProperties() & FileResult::Property::ContentSha1) {
      // TODO: find a way to reference a ContentHashCache instance
      // that will work with !InMemoryView based views.
      localFile->contentSha1_ = makeResultWith([&] {
        return ContentHashCache::computeHashImmediate(
            localFile->fullPath_.c_str());
      });
    }

    localFile->clearNeededProperties();
  }
}

} // namespace watchman
