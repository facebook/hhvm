/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "watchman/test/lib/FakeFileSystem.h"
#include <folly/MapUtil.h>

namespace watchman {

namespace {

constexpr folly::StringPiece kRootPrefix = folly::kIsWindows ? "Z:\\" : "/";

/**
 * Ensures the specified path is absolute, and returns it minus the leading
 * slash. The following sequence of /-delimited names can be used to traverse
 * the filesystem tree.
 */
folly::StringPiece parseAbsolute(folly::StringPiece path) {
  if (path.removePrefix(kRootPrefix)) {
    return path;
  } else {
    throw std::logic_error{fmt::format("Path {} must be absolute", path)};
  }
}

folly::StringPiece parseAbsolute(const char* path) {
  return parseAbsolute(folly::StringPiece{path});
}

/**
 * Ensures the specified path is absolute, and returns two ranges, one to the
 * dirname and one to the basename.
 */
std::pair<folly::StringPiece, folly::StringPiece> parseAbsoluteBasename(
    const char* path) {
  folly::StringPiece parsed{path};
  if (!parsed.removePrefix(kRootPrefix)) {
    throw std::logic_error{fmt::format("Path {} must be absolute", path)};
  }
  path = parsed.data(); // It's still null-terminated.

  const char* slash = strrchr(path, '/');
  if (!slash) {
    return {folly::StringPiece{}, path};
  } else {
    return {folly::StringPiece{path, slash}, folly::StringPiece{slash + 1}};
  }
}

/**
 * Traverse the directory structure and call `func` on the leaf inode.
 *
 * Throws ENOENT if it doesn't exist.
 */
template <typename Func>
std::invoke_result_t<Func, const FakeInode&> withPath(
    const FakeInode& root,
    folly::StringPiece path,
    const char* op,
    Func&& func) {
  const FakeInode* inode = &root;
  while (!path.empty()) {
    size_t idx = path.find('/');
    folly::StringPiece this_level;
    if (idx == folly::StringPiece::npos) {
      this_level = path;
      path.clear();
    } else {
      this_level = path.subpiece(0, idx);
      path.advance(idx + 1);
    }

    inode = folly::get_ptr(inode->children, this_level.str());
    if (!inode) {
      throw std::system_error(
          ENOENT,
          std::generic_category(),
          fmt::format("{}: no file at {}", op, path));
    }
  }

  return func(*inode);
}

/**
 * Traverse the directory structure and call `func` on the leaf inode.
 *
 * Throws ENOENT if it doesn't exist.
 */
template <typename Func>
std::invoke_result_t<Func, FakeInode&> withPath(
    FakeInode& root,
    folly::StringPiece path,
    const char* op,
    Func&& func) {
  FakeInode* inode = &root;
  while (!path.empty()) {
    size_t idx = path.find('/');
    folly::StringPiece this_level;
    if (idx == folly::StringPiece::npos) {
      this_level = path;
      path.clear();
    } else {
      this_level = path.subpiece(0, idx);
      path.advance(idx + 1);
    }

    inode = folly::get_ptr(inode->children, this_level.str());
    if (!inode) {
      throw std::system_error(
          ENOENT,
          std::generic_category(),
          fmt::format("{}: no file at {}", op, path));
    }
  }

  return func(*inode);
}

class FakeDirHandle : public DirHandle {
 public:
  struct FakeDirEntry {
    std::string name;
    std::optional<FileInformation> stat;
  };

  explicit FakeDirHandle(std::vector<FakeDirEntry> entries)
      : entries_{std::move(entries)} {}

  const DirEntry* readDir() override {
    if (idx_ >= entries_.size()) {
      return nullptr;
    }

    auto& e = entries_[idx_++];
    current_.has_stat = e.stat.has_value();
    current_.d_name = e.name.c_str();
    current_.stat = e.stat ? e.stat.value() : FileInformation{};
    return &current_;
  }

#ifndef _WIN32
  int getFd() const override {
    return 0;
  }
#endif

 private:
  size_t idx_ = 0;
  DirEntry current_;
  std::vector<FakeDirEntry> entries_;
};

} // namespace

FakeFileSystem::Flags::Flags() = default;

FakeFileSystem::FakeFileSystem(Flags flags)
    : flags_{flags}, root_{std::in_place, FakeInode{fakeDir()}} {}

std::unique_ptr<DirHandle> FakeFileSystem::openDir(
    const char* path,
    bool strict) {
  auto root = root_.rlock();
  return withPath(
      *root, parseAbsolute(path), "openDir", [&](const FakeInode& inode) {
        // TODO: assert it's a directory

        // TODO: implement strict case checking
        (void)strict;
        std::vector<FakeDirHandle::FakeDirEntry> entries;
        for (auto& [name, child] : inode.children) {
          FakeDirHandle::FakeDirEntry entry;
          entry.name = name;
          if (flags_.includeReadDirStat) {
            entry.stat = child.metadata;
          }
          entries.push_back(std::move(entry));
        }

        return std::make_unique<FakeDirHandle>(std::move(entries));
      });
}

FileInformation FakeFileSystem::getFileInformation(
    const char* path,
    CaseSensitivity caseSensitive) {
  auto root = root_.rlock();
  return withPath(
      *root,
      parseAbsolute(path),
      "getFileInformation",
      [&](const FakeInode& inode) {
        // TODO: validate case
        (void)caseSensitive;

        return inode.metadata;
      });
}
void FakeFileSystem::touch(const char* path) {
  auto pair = parseAbsoluteBasename(path);
  auto& dirname = pair.first;
  auto& basename = pair.second;
  auto root = root_.wlock();
  withPath(*root, dirname, "touch", [&](FakeInode& inode) {
    // TODO: Should we assert if child exists or is a directory?
    auto [iter, inserted] = inode.children.emplace(basename.str(), fakeFile());
    // TODO: What does this mean on Windows? Should we ifdef?
    iter->second.metadata.mode |= 0700;
  });
}

void FakeFileSystem::defineContents(std::initializer_list<const char*> paths) {
  for (folly::StringPiece path : paths) {
    if (path.removeSuffix(kRootPrefix)) {
      fmt::print("addNode dir: {}\n", path);
      addNode(path.str().c_str(), fakeDir());
    } else {
      fmt::print("addNode file: {}\n", path);
      addNode(path.str().c_str(), fakeFile());
    }
  }
}

void FakeFileSystem::addNode(const char* path, const FileInformation& fi) {
  auto root = root_.wlock();
  FakeInode* inode = &*root;

  auto piece = parseAbsolute(path);
  while (!piece.empty()) {
    fmt::print("piece = {}\n", piece);
    size_t idx = piece.find('/');
    folly::StringPiece this_level;
    if (idx == folly::StringPiece::npos) {
      this_level = piece;
      piece.clear();
    } else {
      this_level = piece.subpiece(0, idx);
      piece.advance(idx + 1);
    }

    FakeInode* child = folly::get_ptr(inode->children, this_level.str());
    if (!child) {
      // TODO: add option for whether autocreate is desired or not
      FakeInode fakeInode{fakeDir()};
      auto [iter, yes] = inode->children.emplace(this_level.str(), fakeInode);
      child = &iter->second;
    } else {
      // TODO: ensure child is a directory
    }
    inode = child;
  }

  inode->metadata = fi;
}

void FakeFileSystem::updateMetadata(
    const char* path,
    std::function<void(FileInformation&)> func) {
  auto root = root_.wlock();
  return withPath(
      *root, parseAbsolute(path), "updateMetadata", [&](FakeInode& inode) {
        func(inode.metadata);
      });
}

void FakeFileSystem::removeRecursively(const char* path) {
  auto pair = parseAbsoluteBasename(path);
  auto& dirname = pair.first;
  auto& basename = pair.second;
  auto root = root_.wlock();
  return withPath(*root, dirname, "recursivelyRemove", [&](FakeInode& parent) {
    if (!parent.metadata.isDir()) {
      throw std::system_error(
          ENOTDIR,
          std::generic_category(),
          fmt::format("{} is not a directory", dirname));
    }
    size_t count = parent.children.erase(basename.str());
    if (count == 0) {
      throw std::system_error(
          ENOENT,
          std::generic_category(),
          fmt::format("{} does not exist", path));
    }
    (void)count;
  });
}

FileInformation FakeFileSystem::fakeDir() {
  FileInformation fi{};
  fi.mode = S_IFDIR;
  fi.size = 0;
  fi.uid = kDefaultUid;
  fi.gid = kDefaultGid;
  fi.ino = inodeNumber_.fetch_add(1, std::memory_order_acq_rel);
  fi.dev = kDefaultDev;
  fi.nlink = 2; // TODO: to populate
#ifdef _WIN32
  fi.fileAttributes = FILE_ATTRIBUTE_DIRECTORY;
#endif
  // TODO: populate timestamps
  return fi;
}

FileInformation FakeFileSystem::fakeFile() {
  FileInformation fi{};
  fi.mode = S_IFREG;
  fi.size = 0;
  fi.uid = kDefaultUid;
  fi.gid = kDefaultGid;
  fi.ino = inodeNumber_.fetch_add(1, std::memory_order_acq_rel);
  fi.dev = kDefaultDev;
  fi.nlink = 1;
#ifdef _WIN32
  fi.fileAttributes = 0;
#endif
  // TODO: populate timestamps
  return fi;
}

} // namespace watchman
