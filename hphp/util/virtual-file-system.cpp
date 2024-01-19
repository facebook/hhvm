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

#include "hphp/util/virtual-file-system.h"

#include "hphp/util/alloc.h"
#include "hphp/util/blob-writer.h"
#include "hphp/util/service-data.h"
#include "hphp/util/trace.h"

#include <folly/FileUtil.h>
#include <cstddef>
#include <cstdint>

TRACE_SET_MOD(vfs);

namespace HPHP {

namespace {

////////////////////////////////////////////////////////////////////////////////
// File Format:

/*
 * Fixed Header:
 *
 * - Magic   (4 bytes): "HVFS"
 * - Version (2 bytes)
 *
 * Everything after the fixed header depends on the version number.
 *
 * Version 1 header:
 *
 * - Repo schema (up to 256 bytes)
 * (Everything after this depends on the repo schema)
 */

////////////////////////////////////////////////////////////////////////////////

constexpr Blob::Magic kMagic{ 'H', 'V', 'F', 'S' };

constexpr Blob::Version kCurrentVersion = 1;

// Arbitrary limits on the size of various sections. The sizes are all
// 64-bits, but we don't allow the full range so that a corrupted file
// won't cause us to try to pre-allocate huge amounts of memory. These
// limits were sized so that we should never exceed them, but if we
// ever do, we can just raise them.
constexpr size_t kFileSizeLimit      = 1ull << 36;
constexpr size_t kDirectorySizeLimit = 1ull << 31;
constexpr size_t kIndexSizeLimit     = 1ull << 27;

// If you hit this limit you also need to change Blob::HashMapIndex::Bucket.
constexpr size_t kIndexDataSizeLimit = std::numeric_limits<uint32_t>::max();

enum class Chunks {
  FILES,
  DIRECTORIES,

  SIZE // Leave last!
};

enum class Indexes {
  PATH_TO_ENTRY,

  SIZE // Leave last!
};

struct FileEntry {
  std::string path;
  size_t contentOffset;
  size_t size;
  char* buf;

  template <typename SerDe>
  void serde(SerDe& sd) {
    sd(path)(contentOffset)(size);
  }
};

struct PathCompare {
  bool equal(const std::string& s1, const std::string& s2) const {
    return s1 == s2;
  }
  size_t hash(const std::string& s) const {
    return hash_string_cs_software(s.c_str(), s.size());
  }
};

////////////////////////////////////////////////////////////////////////////////

}

////////////////////////////////////////////////////////////////////////////////
// Writer state
struct VirtualFileSystemWriter::Data : Blob::Writer<Chunks, Indexes> {
  hphp_fast_string_map<VirtualFileSystem::Entry> files;
};

////////////////////////////////////////////////////////////////////////////////

VirtualFileSystemWriter::VirtualFileSystemWriter(const std::string& path)
  : m_data{std::make_unique<Data>()} {
  m_data->header(path, kMagic, kCurrentVersion);
}

VirtualFileSystemWriter::~VirtualFileSystemWriter() {}

bool VirtualFileSystemWriter::addFile(const std::string& relPath,
                                      const std::string& realPath) {
  if (relPath.empty() || relPath[0] == '/') {
    return false;
  }

  std::vector<char> buf;
  auto res = folly::readFile(realPath.c_str(), buf);
  if (!res) {
    return false;
  }

  size_t contentOffset = 0;
  if (buf.size() > 0) {
    contentOffset = m_data->sizes.get(Chunks::FILES);
    m_data->write(Chunks::FILES, buf.data(), buf.size());
  }

  auto ret = m_data->files.emplace(relPath,
    VirtualFileSystem::Entry(VirtualFileSystem::EntryType::REGULAR_FILE,
                             Blob::Bounds { contentOffset, buf.size() } ));
  if (!ret.second) {
    return false;
  }
  return true;
}
bool VirtualFileSystemWriter::addFileWithoutContent(const std::string& relPath) {
  if (relPath.empty() || relPath[0] == '/') {
    return false;
  }

  auto ret = m_data->files.emplace(relPath,
    VirtualFileSystem::Entry(VirtualFileSystem::EntryType::EMPTY_FILE,
                             Blob::Bounds { 0, 0 } ));
  if (!ret.second) {
    return false;
  }
  return true;
}

void addDirectories(const std::filesystem::path& path, hphp_fast_string_map<std::set<std::string>>& directories) {
  auto filename = path.filename().string();
  auto parent_path = path.parent_path();

  directories[parent_path.string()].emplace(filename);

  if (!parent_path.empty()) {
    addDirectories(parent_path, directories);
  }
}

void VirtualFileSystemWriter::finish() {
  hphp_fast_string_map<std::set<std::string>> directories;
  for (auto const& it : m_data->files) {
    addDirectories(std::filesystem::path(it.first), directories);
  }

  auto entries = std::move(m_data->files);
  for (auto const& it : directories) {
    auto contentOffset = m_data->sizes.get(Chunks::DIRECTORIES);

    BlobEncoder encoder;
    encoder(it.second);
    m_data->write(Chunks::DIRECTORIES, encoder.data(), encoder.size());

    auto DEBUG_ONLY ret = entries.emplace(it.first,
      VirtualFileSystem::Entry(VirtualFileSystem::EntryType::DIRECTORY,
                               Blob::Bounds { contentOffset, encoder.size() } ));
    assertx(ret.second);
  }

  m_data->hashMapIndex<VirtualFileSystem::Entry, PathCompare>(
    Indexes::PATH_TO_ENTRY, entries, [](auto const& it) { return it.first; },
    [](auto const& it) { return &it.second; });

  auto data = std::move(m_data);
  data->finish();
}

////////////////////////////////////////////////////////////////////////////////

namespace {

ServiceData::ExportedCounter* s_contentSize =
  ServiceData::createCounter("misc.vfs-content-size");
ServiceData::ExportedCounter* s_fileCount =
  ServiceData::createCounter("misc.vfs-file-count");

};

////////////////////////////////////////////////////////////////////////////////
// Reader state
struct VirtualFileSystem::Data : Blob::Reader<Chunks, Indexes> {
  Blob::HashMapIndex<PathCompare> pathToEntryIndex;

  using PathToEntryMap = folly_concurrent_hash_map_simd<std::string, Entry>;
  mutable PathToEntryMap pathToEntryCache{};
};

////////////////////////////////////////////////////////////////////////////////

VirtualFileSystem::Entry::Entry(Entry&& o) noexcept
  : type(o.type), location(o.location) {
  switch (type) {
    case EntryType::DIRECTORY:
      std::swap(m_list, o.m_list);
      break;
    case EntryType::REGULAR_FILE:
      std::swap(m_content, o.m_content);
      break;
    case EntryType::EMPTY_FILE:
      break;
  }
}

VirtualFileSystem::Entry& VirtualFileSystem::Entry::operator=(
    VirtualFileSystem::Entry&& o) {
  type = o.type;
  location = o.location;

  switch (type) {
    case EntryType::DIRECTORY:
      std::swap(m_list, o.m_list);
      break;
    case EntryType::REGULAR_FILE:
      std::swap(m_content, o.m_content);
      break;
    case EntryType::EMPTY_FILE:
      break;
  }
  return *this;
}

VirtualFileSystem::Entry::~Entry() {
  switch (type) {
    case EntryType::DIRECTORY:
      if (m_list) {
        delete m_list;
      }
      break;
    case EntryType::REGULAR_FILE:
      if (m_content) {
        // If we used the swappable readonly arena we can not free the memory
        auto arena = get_swappable_readonly_arena();
        if (!arena) {
          free(m_content);
          s_contentSize->addValue(-fileSize());
        }
      }
      break;
    case EntryType::EMPTY_FILE:
      break;
  }
}

////////////////////////////////////////////////////////////////////////////////

VirtualFileSystem::VirtualFileSystem(const std::string& path,
                                     const std::string& root)
  : m_data{std::make_unique<Data>()} {
  m_root = root;
  assertx(!m_root.empty() && m_root[0] == '/');
  if (m_root[m_root.size() - 1] != '/') {
    m_root += '/';
  }

  auto& data = *m_data;
  data.init(path, kMagic, kCurrentVersion);

  data.check(Chunks::FILES, kFileSizeLimit);
  data.check(Chunks::DIRECTORIES, kDirectorySizeLimit);

  for (auto i = 0; i < uint32_t(Indexes::SIZE); i++) {
    data.check(Indexes(i), kIndexSizeLimit, kIndexDataSizeLimit);
  }

  data.pathToEntryIndex =
    data.hashMapIndex<PathCompare>(Indexes::PATH_TO_ENTRY);
}

VirtualFileSystem::~VirtualFileSystem() {}

Optional<VirtualFileSystem::Content> VirtualFileSystem::content(
    const std::string& path) const {
  auto entry = get(path);
  if (!entry || !entry->isRegularFile()) {
    return {};
  }
  auto size = entry->fileSize();
  auto content = entry->m_content;
  if (content == nullptr) {
    FTRACE(1, "Fetch content {} {} {}\n", path.c_str(), entry->location.offset,
           size);
    auto arena = get_swappable_readonly_arena();
    content = static_cast<char*>(arena ? arena->allocate(size) : malloc(size));
    s_contentSize->addValue(size);
    m_data->fd.pread(content, size,
      m_data->offsets.get(Chunks::FILES) + entry->location.offset);
    entry->m_content = content;
  }
  return { Content { size, content } };
}

std::vector<std::string> VirtualFileSystem::listDirectory(
    const std::string& path) const {
  auto entry = get(path);
  if (!entry || !entry->isDirectory()) {
    return std::vector<std::string>{};
  }

  auto list = entry->m_list;
  if (list == nullptr) {
    list = new std::vector<std::string> {};
    auto blob = m_data->fd.readBlob(m_data->offsets.get(Chunks::DIRECTORIES) + entry->location.offset, entry->location.size);
    blob.decoder(*list);
    entry->m_list = list;
  }
  return *list;
}

Optional<size_t> VirtualFileSystem::fileSize(const std::string& path) const {
  auto entry = get(path);
  if (!entry || !entry->isRegularFile()) {
    return {};
  }
  return { entry->fileSize() };
}

namespace {

Optional<std::string> relativePath(const std::string& path, const std::string& root) {
  auto removeTrailingSlash = [](const std::string& relPath) {
    if (relPath.size() > 0 && relPath[relPath.size() - 1] != '/') {
      return relPath;
    }

    std::string s = relPath;
    s.erase(std::find_if(s.rbegin(), s.rend(),
      [](unsigned char ch) { return ch != '/'; }).base(),
      s.end());
    return s;
  };

  if (path.empty() || path[0] != '/') {
    return { removeTrailingSlash(path) };
  }

  // Check if the absolute path start with m_root and if so remove it
  if (path.rfind(root, 0) == 0) {
    auto relPath = path.substr(root.size(), path.size() - root.size());
    return { removeTrailingSlash(relPath) };
  }

  return { };
}

}

const VirtualFileSystem::Entry* VirtualFileSystem::get(const std::string& path) const {
  assertx(m_data);
  auto& data = *m_data;

  auto relPathRes = relativePath(path, m_root);
  if (!relPathRes) {
    return nullptr;
  }

  auto relPath = *relPathRes;
  auto acc = data.pathToEntryCache.find(relPath);
  if (acc != data.pathToEntryCache.cend()) {
    return &acc->second;
  }

  auto entry = data.getFromIndex<Entry>(data.pathToEntryIndex, relPath);
  if (!entry) {
    return nullptr;
  }

  if (m_logger) {
    m_logger(relPath);
  }

  auto insertRes = data.pathToEntryCache.insert(relPath, std::move(*entry));
  if (insertRes.second) {
    s_fileCount->increment();
  }
  return &insertRes.first->second;
}

void VirtualFileSystem::dump() const {
  assertx(m_data);
  auto& data = *m_data;

  auto dataBounds = data.pathToEntryIndex.dataBounds;
  auto dataBlob = data.fd.readBlob(dataBounds.offset, dataBounds.size);
  while (dataBlob.decoder.remaining() > 0) {
    std::string path;
    Entry entry;
    dataBlob.decoder(path)(entry);

    std::string type;
    switch (entry.type) {
      case EntryType::DIRECTORY:
        type = "directory";
        break;
      case EntryType::REGULAR_FILE:
        type = "regular";
        break;
      case EntryType::EMPTY_FILE:
        type = "empty";
        break;
    }

    printf("- %s\n  Type: %s\n  Size: %zu\n", path.c_str(), type.c_str(), entry.fileSize());
  }
}

////////////////////////////////////////////////////////////////////////////////

}
