/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-present Facebook, Inc. (http://www.facebook.com)  |
   +----------------------------------------------------------------------+
   | This source file is subject to version 3.01 of the PHP license,      |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | http://www.php.net/license/3_01.txt                                  |
   | If you did not receive a copy of the PHP license and are unable to   |
   | obtain it through the world-wide-web, please send a note to          |
   | license@php.net so we can mail you a copy immediately.               |
   +----------------------------------------------------------------------+
*/

#pragma once

#include "hphp/util/blob.h"
#include "hphp/util/optional.h"

#include <memory>
#include <string>
#include <vector>

namespace HPHP {

struct VirtualFileSystemWriter {

  explicit VirtualFileSystemWriter(const std::string&);
  ~VirtualFileSystemWriter();

  bool addFile(const std::string& relPath, const std::string& realPath);
  bool addFileWithoutContent(const std::string& relPath);

  void finish();

  VirtualFileSystemWriter(const VirtualFileSystemWriter&) = delete;
  VirtualFileSystemWriter(VirtualFileSystemWriter&&) = delete;
  VirtualFileSystemWriter& operator=(const VirtualFileSystemWriter&) = delete;
  VirtualFileSystemWriter& operator=(VirtualFileSystemWriter&&) = delete;

private:
  struct Data;
  std::unique_ptr<Data> m_data;
};

struct VirtualFileSystem {

  enum class FileType {
    NOT_FOUND,
    REGULAR_FILE,
    DIRECTORY
  };

  enum class EntryType {
    REGULAR_FILE,
    EMPTY_FILE,
    DIRECTORY
  };

  struct Entry {

    Entry(EntryType type, Blob::Bounds location)
    : type(type), location(location) {}
    Entry(): Entry(EntryType::EMPTY_FILE, Blob::Bounds { 0, 0 }) {}

    Entry(const Entry& entry) = delete;
    Entry& operator=(const Entry&) = delete;
    Entry(Entry&& entry) noexcept;
    Entry& operator=(Entry&& entry);

    EntryType type;

    size_t fileSize() const {
      return location.size;
    }

    bool isRegularFile() const {
      return type == EntryType::REGULAR_FILE;
    }

    bool isEmptyFile() const {
      return type == EntryType::EMPTY_FILE;
    }

    bool isDirectory() const {
      return type == EntryType::DIRECTORY;
    }

    ~Entry();

    Blob::Bounds location;
    union {
      mutable char* m_content = nullptr;
      mutable std::vector<std::string>* m_list;
    };

    template <typename SerDe>
    void serde(SerDe& sd) {
      sd(type)(location);
    }
  };

  struct Content {
    size_t size;
    const char* buffer;
  };

  VirtualFileSystem(const std::string& path, const std::string& root);
  ~VirtualFileSystem();

  void destroy();

  bool exists(const std::string& path) const {
    return get(path) != nullptr;
  }

  bool fileExists(const std::string& path) const {
    auto res = get(path);
    return res && (res->isRegularFile() || res->isEmptyFile());
  }

  bool dirExists(const std::string& path) const {
    auto res = get(path);
    return res && res->isDirectory();
  }

  FileType fileType(const std::string& path) const {
    auto res = get(path);
    if (!res) {
      return FileType::NOT_FOUND;
    }
    return res->isDirectory() ? FileType::DIRECTORY : FileType::REGULAR_FILE;
  }

  Optional<size_t> fileSize(const std::string& path) const;

  Optional<Content> content(const std::string& path) const;

  std::vector<std::string> listDirectory(const std::string& path) const;

  void setLogger(std::function<void(const std::string&)>&& logger) {
    m_logger = logger;
  }

  void dump() const;

 private:
  const Entry* get(const std::string& path) const;

  struct Data;
  std::unique_ptr<Data> m_data;
  std::string m_root;
  std::function<void(const std::string&)> m_logger;
};

}
