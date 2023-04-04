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

#include "hphp/runtime/vm/repo-file.h"

#include "hphp/runtime/base/autoload-handler.h"
#include "hphp/runtime/base/variable-serializer.h"
#include "hphp/runtime/base/variable-unserializer.h"

#include "hphp/runtime/vm/func-emitter.h"
#include "hphp/runtime/vm/repo-autoload-map-builder.h"
#include "hphp/runtime/vm/repo-global-data.h"
#include "hphp/runtime/vm/unit-emitter.h"

#include "hphp/util/build-info.h"
#include "hphp/util/htonll.h"
#include "hphp/util/lock-free-ptr-wrapper.h"

#include "hphp/zend/zend-string.h"

#include <folly/FileUtil.h>
#include <folly/String.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <tuple>

TRACE_SET_MOD(repo_file);

namespace HPHP {

////////////////////////////////////////////////////////////////////////////////

namespace {

enum class RepoFileIndexes {
  UNIT_SYMBOLS,
  UNIT_INFOS,
  AUTOLOAD_TYPES,
  AUTOLOAD_FUNCS,
  AUTOLOAD_CONSTANTS,
  AUTOLOAD_TYPEALIASES,
  AUTOLOAD_MODULES,
  PATH_TO_UNIT_INFO,

  SIZE // Leave last!
};

uint8_t  hostToFile(uint8_t v)  { return v; }
uint16_t hostToFile(uint16_t v) { return htons(v); }
uint32_t hostToFile(uint32_t v) { return htonl(v); }
uint64_t hostToFile(uint64_t v) { return htonll(v); }

uint8_t  fileToHost(uint8_t v)  { return v; }
uint16_t fileToHost(uint16_t v) { return ntohs(v); }
uint32_t fileToHost(uint32_t v) { return ntohl(v); }
uint64_t fileToHost(uint64_t v) { return ntohll(v); }

// Wrapper around file descriptor for automatic closing and wrapping
// operations in a nicer interface.
struct FD {
  FD() : m_fd{-1} {}
  FD(const std::string& path, int flags) : m_path{path}
  {
    auto fd = folly::openNoInt(m_path.c_str(), flags, 0644);
    if (fd < 0) {
      auto const error = folly::errnoStr(errno);
      always_assert_flog(
        false,
        "Unable to open {}: {}", m_path, error
      );
    }
    m_fd = fd;
  }
  ~FD() { if (isOpen()) ::close(m_fd); }

  bool isOpen() const { return m_fd >= 0; }

  void write(const void* data, size_t size) const {
    auto const written = folly::writeFull(m_fd, data, size);
    if (written == size) return;
    if (written < 0) {
      auto const error = folly::errnoStr(errno);
      always_assert_flog(
        false,
        "Failed writing {} bytes to {}: {}",
        size, m_path, error
      );
    }
    always_assert_flog(
      false,
      "Partial write to {} (expected {}, actual {})",
      m_path, size, written
    );
  }

  // Write an integer in network byte-order
  template <typename T> void writeInt(T v) const {
    v = hostToFile(v);
    this->write(&v, sizeof(v));
  }

  void read(void* data, size_t size) const {
    auto const read = folly::readFull(m_fd, data, size);
    if (read == size) return;
    if (read < 0) {
      auto const error = folly::errnoStr(errno);
      always_assert_flog(
        false,
        "Failed reading {} bytes from {}: {}",
        size, m_path, error
      );
    }
    always_assert_flog(
      false,
      "Partial read from {} (expected {}, actual {})",
      m_path, size, read
    );
  }

  // Read a network byte-order integer
  template <typename T> std::remove_cv_t<T> readInt() const {
    std::remove_cv_t<T> v;
    this->read(&v, sizeof(v));
    return fileToHost(v);
  }

  void pread(void* data, size_t size, size_t offset) const {
    auto const read = folly::preadFull(m_fd, data, size, offset);
    if (read == size) return;
    if (read < 0) {
      auto const error = folly::errnoStr(errno);
      always_assert_flog(
        false,
        "Failed reading {} bytes from {} at {}: {}",
        size, m_path, offset, error
      );
    }
    always_assert_flog(
      false,
      "Partial read from {} at {} (expected {}, actual {})",
      m_path, offset, size, read
    );
  }

  void seek(size_t offset) const {
    auto const actualOffset = ::lseek(m_fd, offset, SEEK_SET);
    if (actualOffset == offset) return;
    if (actualOffset < 0) {
      auto const error = folly::errnoStr(errno);
      always_assert_flog(
        false,
        "Failed to seek to {} in {}: {}",
        offset, m_path, error
      );
    }
    always_assert_flog(
      false,
      "Partial seek in {} (expected {}, actual {})",
      m_path, offset, actualOffset
    );
  }

  uint64_t fileSize() const {
    auto const size = ::lseek(m_fd, 0, SEEK_END);
    if (size >= 0) return size;
    auto const error = folly::errnoStr(errno);
    always_assert_flog(
      false,
      "Failed to seek to end of {}: {}",
      m_path, error
    );
  }

  struct Blob {
    std::unique_ptr<char[]> buffer;
    BlobDecoder decoder;
  };

  Blob readBlob(size_t offset, size_t size) const {
    auto buffer = std::make_unique<char[]>(size);
    if (size > 0) this->pread(buffer.get(), size, offset);
    BlobDecoder decoder{buffer.get(), size};
    return { std::move(buffer), std::move(decoder) };
  }

  FD(const FD&) = delete;
  FD& operator=(const FD&) = delete;

  FD(FD&& o) noexcept
    : m_fd{o.m_fd}
    , m_path{std::move(o.m_path)}
  {
    o.m_fd = -1;
  }

  FD& operator=(FD&& o) {
    std::swap(m_fd, o.m_fd);
    std::swap(m_path, o.m_path);
    return *this;
  }
private:
  int m_fd;
  std::string m_path;
};

////////////////////////////////////////////////////////////////////////////////
// File Format:

/*
 * Fixed Header:
 *
 * - Magic   (4 bytes): "HHBC"
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

constexpr std::array<char, 4> kMagic{ 'H', 'H', 'B', 'C' };

constexpr uint16_t kCurrentVersion = 1;

// Arbitrary limits on the size of various sections. The sizes are all
// 64-bits, but we don't allow the full range so that a corrupted file
// won't cause us to try to pre-allocate huge amounts of memory. These
// limits were sized so that we should never exceed them, but if we
// ever do, we can just raise them.
constexpr size_t kUnitEmitterSizeLimit       = 1ull << 33;
constexpr size_t kGlobalDataSizeLimit        = 1ull << 28;
constexpr size_t kIndexSizeLimit             = 1ull << 31;

// Blob of data containing the sizes of the various sections. This is
// stored near the beginning of the file and can be used to compute
// offsets for the sections. Since we don't know the sizes until after
// we write everything, we have to write these as fixed size integers.
struct SizeHeader {
  uint64_t unitEmittersSize      = 0;
  uint64_t globalDataSize        = 0;
  uint64_t indexSizes[uint32_t(RepoFileIndexes::SIZE) * 2] = { 0 };

  constexpr static size_t kFileSize =
    sizeof(unitEmittersSize) +
    sizeof(globalDataSize) +
    sizeof(indexSizes);

  void write(const FD& fd) const {
    fd.writeInt(unitEmittersSize);
    fd.writeInt(globalDataSize);
    for (auto i = 0; i < uint32_t(RepoFileIndexes::SIZE) * 2; i++) {
      fd.writeInt(indexSizes[i]);
    }
  }

  void read(const FD& fd) {
    unitEmittersSize      = fd.readInt<decltype(unitEmittersSize)>();
    globalDataSize        = fd.readInt<decltype(globalDataSize)>();
    for (auto i = 0; i < uint32_t(RepoFileIndexes::SIZE) * 2; i++) {
      indexSizes[i]       =
        fd.readInt<std::remove_reference<decltype(indexSizes[0])>::type>();
    }
  }

  void setIndexSizes(RepoFileIndexes index, uint64_t indexSize,
                     uint64_t dataSize) {
    // We must write indexes from the lowest to highest
    assertx(uint32_t(index) == 0 || indexSizes[(uint32_t(index) - 1) * 2] != 0);
    indexSizes[uint32_t(index) * 2] = indexSize;
    indexSizes[uint32_t(index) * 2 + 1] = dataSize;
  }
};

////////////////////////////////////////////////////////////////////////////////

const StringData* relativePathToSourceRoot(const StringData* path) {
  assertx(!path->empty());
  if (path->data()[0] == '/' && !RO::SourceRoot.empty() &&
    !strncmp(RO::SourceRoot.c_str(), path->data(), RO::SourceRoot.size())) {
    return makeStaticString(path->data() + RO::SourceRoot.size(),
                            path->size() - RO::SourceRoot.size());
  }
  return path;
}

////////////////////////////////////////////////////////////////////////////////

/**
 * The Hash Map Index.
 * Contains two part the index and the data.
 * The index is just the buckets and contains the offset where to find the data
 * for that bucket. Each offset is stored using fixed with uint32_t. So we can
 * seek into it.
 * The data for each bucket is 0 or more items. Starts with the key and then the
 * item data.
 *
 * Layout:
 * Index (N + 1 offsets)
 * -----------------------------------------------------------------------------
 * | offset 0  | offset 1 | offset 2 | offset 3 | ...                          |
 * -----------------------------------------------------------------------------
 * Data (N buckets)
 * -----------------------------------------------------------------------------
 * | key0, item0 data, key1, item1 data | | key2, item2 data | ...             |
 * -----------------------------------------------------------------------------
 *
 * How to read:
 * h = hash(key)
 * bucket = h % N
 * // We read the current bucket and next bucket to know when our bucket ends
 * start_offset, end_offset = read(bucket, 2)
 * blob = read(start_offset, end_offset - start_offset)
 * // iterate through blob and check if the key is there. If it is return data
 * while (blob.remaining() > 0)
 *   candidate_key = blob.read()
 *   data = blob.read()
 *   if (key == candidate_key)
 *     return data
 * return null
 */
template <typename T, typename D, typename L, typename KF, typename VF>
void writeHashMapIndex(D data, RepoFileIndexes index, L& list, KF key_lambda,
                       VF value_lambda) {
  assertx(data);
  BlobEncoder indexBlob;
  BlobEncoder dataBlob;
  auto num_buckets = list.size();

  struct Item {
    const StringData* key;
    T* value;
  };
  using Bucket = std::vector<Item>;
  auto buckets = std::vector<Bucket>(num_buckets);

  for (auto const& it : list) {
    auto key = key_lambda(it);
    auto hash = key->hash();
    auto bucket = hash % num_buckets;
    buckets[bucket].push_back(Item { key, value_lambda(it) });
  }
  for (auto const& bucket : buckets) {
    auto offset = dataBlob.size();
    indexBlob.fixedWidth(uint32_t(offset));
    for (auto& item : bucket) {
      dataBlob(item.key->toCppString())(*item.value);
      FTRACE(2, "writeHashMapIndex item {} {} {} {}\n", uint32_t(index),
             item.key, offset, dataBlob.size() - offset);
    }
    FTRACE(1, "writeHashMapIndex {} {} {}\n", uint32_t(index), offset,
           dataBlob.size() - offset);
  }
  indexBlob.fixedWidth(uint32_t(dataBlob.size()));

  data->fd.write(indexBlob.data(), indexBlob.size());
  data->fd.write(dataBlob.data(), dataBlob.size());
  data->sizes.setIndexSizes(index, indexBlob.size(), dataBlob.size());
}

/**
 * The Hash List Index.
 * Contains two part the index and the data.
 * The index is just the offsets so we can find the data. Each offset is stored
 * using fixed with uint32_t. So we can seek into it.
 * The data for each bucket contains the data for that item
 *
 * Layout:
 * Index (N + 1 offsets)
 * -----------------------------------------------------------------------------
 * | offset 0  | offset 1 | offset 2 | offset 3 | ...                          |
 * -----------------------------------------------------------------------------
 * Data (N buckets)
 * -----------------------------------------------------------------------------
 * | item0 data | item1 data | item2 data | ...                                |
 * -----------------------------------------------------------------------------
 *
 * How to read:
 * // We read the current offset and next offset to know when our data ends
 * start_offset, end_offset = read(id, 2)
 * blob = read(start_offset, end_offset - start_offset)
 * return blob.read()
 */
template <typename D, typename T>
std::vector<RepoBounds> writeListIndex(D data, RepoFileIndexes index,
                                       std::vector<T> list) {
  assertx(data);
  BlobEncoder indexBlob;
  BlobEncoder dataBlob;

  std::vector<RepoBounds> bounds;
  bounds.reserve(list.size());
  size_t offset = 0;
  for (auto& item : list) {
    indexBlob.fixedWidth(uint32_t(dataBlob.size()));
    dataBlob(item);
    auto size = dataBlob.size() - offset;
    bounds.push_back(RepoBounds { offset, size });
    FTRACE(1, "writeListIndex {} {} {}\n", uint32_t(index), offset, size);
    offset += size;
  }
  indexBlob.fixedWidth(uint32_t(dataBlob.size()));

  data->fd.write(indexBlob.data(), indexBlob.size());
  data->fd.write(dataBlob.data(), dataBlob.size());
  data->sizes.setIndexSizes(index, indexBlob.size(), dataBlob.size());

  return bounds;
}

////////////////////////////////////////////////////////////////////////////////

}

////////////////////////////////////////////////////////////////////////////////

void RepoUnitInfo::serde(BlobEncoder& sd) const {
  std::string spath = path->toCppString();
  auto const pathData = spath.c_str();
  assertx(!spath.empty());
  if (pathData[0] == '/' && !RO::SourceRoot.empty() &&
      !strncmp(RO::SourceRoot.c_str(), pathData, RO::SourceRoot.size())) {
    spath = spath.substr(RO::SourceRoot.size());
  }

  sd(unitSn)(spath)(emitterLocation)(symbolsLocation);
}

void RepoUnitInfo::serde(BlobDecoder& sd) {
  std::string spath;

  sd(unitSn)(spath)(emitterLocation)(symbolsLocation);

  if (!RO::SourceRoot.empty() && !spath.empty() && spath.c_str()[0] != '/') {
    spath = RO::SourceRoot + spath;
  }
  path = makeStaticString(spath);
}

////////////////////////////////////////////////////////////////////////////////

// Builder state
struct RepoFileBuilder::Data {
  FD fd;
  std::string sourceFilename;
  std::string destFilename;
  uint64_t sizeHeaderOffset = 0;
  SizeHeader sizes;

  struct UnitEmitterIndex {
    const StringData* path;
    int64_t sn;
    RepoBounds location;
  };
  std::vector<UnitEmitterIndex> unitEmittersIndex;
};

////////////////////////////////////////////////////////////////////////////////

RepoFileBuilder::RepoFileBuilder(const std::string& path)
  : m_data{std::make_unique<Data>()}
{
  m_data->sourceFilename = folly::sformat("{}.part", path);
  m_data->destFilename = path;
  m_data->fd = FD{
    m_data->sourceFilename,
    O_CLOEXEC | O_CREAT | O_TRUNC | O_WRONLY
  };

  m_data->fd.write(kMagic.data(), sizeof(kMagic));
  m_data->fd.writeInt(kCurrentVersion);

  auto const repoSchema = repoSchemaId();
  always_assert(repoSchema.size() < 256);
  m_data->fd.writeInt((uint8_t)repoSchema.size());
  m_data->fd.write(repoSchema.data(), repoSchema.size());

  m_data->sizeHeaderOffset =
    sizeof(kMagic) + sizeof(kCurrentVersion) +
    sizeof(uint8_t) + repoSchema.size();
  // Sizes go here: We don't know the sizes yet, so we'll just write
  // zeros. Afterwards we'll go back and overwrite it with the actual
  // sizes.
  m_data->sizes.write(m_data->fd);
}

RepoFileBuilder::~RepoFileBuilder() {
}

void RepoFileBuilder::add(const EncodedUE& ue) {
  assertx(m_data);
  assertx(ue.path->isStatic());
  assertx(ue.sn >= 0);

  FTRACE(2, "RepoFileBuilder::add {}\n", ue.path);

  const StringData* path = ue.path;
  assertx(!path->empty());
  path = relativePathToSourceRoot(path);

  auto const size = ue.blob.size();
  always_assert(size <= kUnitEmitterSizeLimit);

  m_data->unitEmittersIndex.emplace_back(
    RepoFileBuilder::Data::UnitEmitterIndex{
      path,
      ue.sn,
      { m_data->sizes.unitEmittersSize, size }
    }
  );

  m_data->fd.write(ue.blob.data(), size);
  m_data->sizes.unitEmittersSize += size;
}

void RepoFileBuilder::finish(const RepoGlobalData& global,
                             const RepoAutoloadMapBuilder& autoloadMap) {
  assertx(m_data);

  // Global data
  {
    BlobEncoder encoder;
    encoder(global);
    m_data->fd.write(encoder.data(), encoder.size());
    m_data->sizes.globalDataSize = encoder.size();
    always_assert(m_data->sizes.globalDataSize <= kGlobalDataSizeLimit);
  }

  // Unit Symbols
  std::vector<RepoBounds> unitSymbolsBounds;
  {
    std::vector<RepoUnitSymbols> list(m_data->unitEmittersIndex.size());

    auto add_symbols = [&](auto const& symbols, auto type) {
      for (auto const& info : symbols) {
        list[info.second].push_back(std::make_pair(info.first, type));
      }
    };

    add_symbols(autoloadMap.getTypes(), RepoSymbolType::TYPE);
    add_symbols(autoloadMap.getFuncs(), RepoSymbolType::FUNC);
    add_symbols(autoloadMap.getConstants(), RepoSymbolType::CONSTANT);
    add_symbols(autoloadMap.getTypeAliases(), RepoSymbolType::TYPE_ALIAS);
    add_symbols(autoloadMap.getModules(), RepoSymbolType::MODULE);

    unitSymbolsBounds = writeListIndex(m_data.get(),
                                       RepoFileIndexes::UNIT_SYMBOLS, list);
  }

  // Unit Infos
  std::vector<RepoBounds> unitInfosBounds;
  {
    std::vector<RepoUnitInfo> list(m_data->unitEmittersIndex.size());
    for (auto const& unit : m_data->unitEmittersIndex) {
      list[unit.sn] = RepoUnitInfo {
        unit.sn,
        unit.path,
        unit.location,
        unitSymbolsBounds[unit.sn]
      };
    }

    unitInfosBounds = writeListIndex(m_data.get(), RepoFileIndexes::UNIT_INFOS,
                                     list);
  }

  // Repo Autoload Map
  // Symbol to RepoBounds for the UnitInfo
  {
    auto key_lambda = [](auto const& it) { return it.first; };
    auto value_lambda = [&](auto const& it) {
      return &unitInfosBounds[it.second];
    };

    writeHashMapIndex<RepoBounds>(m_data.get(),
                                  RepoFileIndexes::AUTOLOAD_TYPES,
                                  autoloadMap.getTypes(),
                                  key_lambda, value_lambda);
    writeHashMapIndex<RepoBounds>(m_data.get(),
                                  RepoFileIndexes::AUTOLOAD_FUNCS,
                                  autoloadMap.getFuncs(),
                                  key_lambda, value_lambda);
    writeHashMapIndex<RepoBounds>(m_data.get(),
                                  RepoFileIndexes::AUTOLOAD_CONSTANTS,
                                  autoloadMap.getConstants(),
                                  key_lambda, value_lambda);
    writeHashMapIndex<RepoBounds>(m_data.get(),
                                  RepoFileIndexes::AUTOLOAD_TYPEALIASES,
                                  autoloadMap.getTypeAliases(),
                                  key_lambda, value_lambda);
    writeHashMapIndex<RepoBounds>(m_data.get(),
                                  RepoFileIndexes::AUTOLOAD_MODULES,
                                  autoloadMap.getModules(),
                                  key_lambda, value_lambda);
  }

  // Path to RepoBounds for the UnitInfo
  writeHashMapIndex<RepoBounds>(m_data.get(),
                                RepoFileIndexes::PATH_TO_UNIT_INFO,
                                m_data->unitEmittersIndex,
                                [](auto const& unit) { return unit.path; },
                                [&](auto const& unit) {
                                  return &unitInfosBounds[unit.sn];
                                });

  // All the sizes are updated, so now go patch the size table.
  m_data->fd.seek(m_data->sizeHeaderOffset);
  m_data->sizes.write(m_data->fd);

  auto const source = m_data->sourceFilename;
  auto const dest = m_data->destFilename;
  m_data.reset();

  // Signify completion of the file by renaming it to its final name.
  if (::rename(source.c_str(), dest.c_str()) < 0) {
    auto const error = folly::errnoStr(errno);
    always_assert_flog(
      "Unable to rename {} to {}: {}",
      source, dest, error
    );
  }
}

////////////////////////////////////////////////////////////////////////////////

RepoFileBuilder::EncodedUE::EncodedUE(const UnitEmitter& ue)
  : path{ue.m_filepath}
  , sn{ue.m_sn}
{
  BlobEncoder encoder;
  const_cast<UnitEmitter&>(ue).serde(encoder, false);
  blob = encoder.take();
}

////////////////////////////////////////////////////////////////////////////////

namespace {

// Reader state
struct RepoFileData {
  FD fd;

  std::string path;

  SizeHeader sizes;
  uint64_t fileSize = 0;

  uint64_t unitEmittersOffset = 0;
  uint64_t globalDataOffset = 0;
  uint64_t indexOffsets[uint32_t(RepoFileIndexes::SIZE) * 2] = { 0 };

  RepoGlobalData globalData;

  RepoFile::CaseSensitiveHashMapIndex pathToUnitInfoBoundsIndex;
  RepoFile::ListIndex unitInfosIndex;
  RepoFile::ListIndex unitSymbolsIndex;

  std::atomic<bool> loadedGlobalTables{false};

  using UnitSnToUnitInfoMap = folly_concurrent_hash_map_simd<int64_t,
                                                            RepoUnitInfo>;
  mutable UnitSnToUnitInfoMap snToUnitInfo{};

  using PathToSymbolsMap = folly_concurrent_hash_map_simd<
    const StringData*, RepoUnitSymbols, string_data_hash, string_data_same>;
  mutable PathToSymbolsMap pathToSymbols{};
};

std::unique_ptr<RepoFileData> s_repoFileData{};

////////////////////////////////////////////////////////////////////////////////

template <typename T, typename Compare>
Optional<T> getFromIndex(const RepoFileData& data,
    const RepoFile::HashMapIndex<Compare>& map, const StringData* key) {
  if (map.size == 0) {
    return {};
  }

  auto hash = key->hash();
  auto bucket = hash % map.size;

  FTRACE(1, "getFromIndex Hash {} {} {} {}\n", key, hash, bucket, map.size);

  uint32_t currentOffset;
  uint32_t nextOffset;
  assertx(sizeof(currentOffset) * bucket < map.indexBounds.size);
  assertx(sizeof(currentOffset) * bucket + sizeof(currentOffset) * 2
          <= map.indexBounds.size);
  auto indexBlob = data.fd.readBlob(
    map.indexBounds.offset + sizeof(currentOffset) * bucket,
    sizeof(currentOffset) * 2);
  indexBlob.decoder.fixedWidth(currentOffset);
  indexBlob.decoder.fixedWidth(nextOffset);

  FTRACE(2, "getFromIndex Hash {} {} {}\n", currentOffset, nextOffset,
         map.dataBounds.size);
  assertx(currentOffset <= map.dataBounds.size);
  assertx(nextOffset <= map.dataBounds.size);

  if (currentOffset == nextOffset) {
    return {};
  }

  auto dataBlob = data.fd.readBlob(map.dataBounds.offset + currentOffset,
                                   nextOffset - currentOffset);

  auto compare = Compare();
  while (dataBlob.decoder.remaining() > 0) {
    FTRACE(3, "getFromIndex Hash {}\n", dataBlob.decoder.remaining());
    std::string candidate_key;
    dataBlob.decoder(candidate_key);
    const String candidate_key_str = candidate_key;

    T res;
    dataBlob.decoder(res);
    if (compare(candidate_key_str.get(), key)) {
      return { res };
    }
  }
  return {};
}

template <typename T>
Optional<T> getFromIndex(const RepoFileData& data,
                         const RepoFile::ListIndex& list, int64_t index) {
  if (index < 0 || index >= list.size) {
    return {};
  }

  uint32_t currentOffset;
  uint32_t nextOffset;
  assertx(sizeof(currentOffset) * index < list.indexBounds.size);
  assertx(sizeof(currentOffset) * index + sizeof(currentOffset) * 2
          <= list.indexBounds.size);
  auto indexBlob = data.fd.readBlob(
    list.indexBounds.offset + sizeof(currentOffset) * index,
    sizeof(currentOffset) * 2);
  indexBlob.decoder.fixedWidth(currentOffset);
  indexBlob.decoder.fixedWidth(nextOffset);

  assertx(currentOffset < list.dataBounds.size);
  assertx(nextOffset <= list.dataBounds.size);
  auto dataBlob = data.fd.readBlob(list.dataBounds.offset + currentOffset,
                                   nextOffset - currentOffset);
  T res;
  dataBlob.decoder(res);
  return { res };
}

const RepoUnitInfo& getUnitInfoFromUnitSn(const RepoFileData& data,
                                          int64_t unitSn) {
  auto index = data.unitInfosIndex;
  assertx(unitSn >= 0 && unitSn < index.size);

  auto acc = data.snToUnitInfo.find(unitSn);
  if (acc != data.snToUnitInfo.cend()) {
    return acc->second;
  }

  auto res = getFromIndex<RepoUnitInfo>(data, index, unitSn);
  assertx(res);
  auto insertRes = data.snToUnitInfo.insert(unitSn, *res);
  return insertRes.first->second;
}

const RepoUnitInfo& getUnitInfoFromBounds(const RepoFileData& data,
                                          const RepoBounds& bounds) {
  FTRACE(1, "getUnitInfoFromBounds {} {} {} {}\n", bounds.offset, bounds.size,
         data.unitInfosIndex.dataBounds.offset,
         data.unitInfosIndex.dataBounds.size);
  assertx(bounds.size > 0);
  assertx(bounds.offset < data.unitInfosIndex.dataBounds.size);
  assertx(bounds.offset + bounds.size
          <= data.unitInfosIndex.dataBounds.size);

  auto blob = data.fd.readBlob(
    data.unitInfosIndex.dataBounds.offset + bounds.offset, bounds.size);
  RepoUnitInfo info;
  blob.decoder(info);
  blob.decoder.assertDone();

  auto insertRes = data.snToUnitInfo.insert(info.unitSn, info);
  return insertRes.first->second;
}

RepoUnitSymbols getUnitSymbolsFromBounds(const RepoFileData& data,
                                         const RepoBounds& bounds) {
  FTRACE(1, "getUnitSymbolsFromBounds {} {} {} {}\n", bounds.offset,
         bounds.size, data.unitSymbolsIndex.dataBounds.offset,
         data.unitSymbolsIndex.dataBounds.size);
  assertx(bounds.size > 0);
  assertx(bounds.offset < data.unitSymbolsIndex.dataBounds.size);
  assertx(bounds.offset + bounds.size
          <= data.unitSymbolsIndex.dataBounds.size);

  auto blob = data.fd.readBlob(
    data.unitSymbolsIndex.dataBounds.offset + bounds.offset, bounds.size);
  RepoUnitSymbols symbols;
  blob.decoder(symbols);
  blob.decoder.assertDone();

  return symbols;
}

template <typename Compare>
const RepoUnitInfo* findUnitInfoFromKey(
    const RepoFileData& data, const RepoFile::HashMapIndex<Compare>& map,
    const StringData* key) {
  auto bounds = getFromIndex<RepoBounds>(data, map, key);
  if (!bounds) {
    return nullptr;
  }
  return &getUnitInfoFromBounds(data, *bounds);
}

const RepoUnitInfo* findUnitInfoFromPath(const RepoFileData& data,
                                         const StringData* path) {
  auto searchPath = path;
  if (!path->empty()) {
    searchPath = relativePathToSourceRoot(path);
  }

  FTRACE(1, "findUnitInfoFromPath {} {}\n", searchPath->data(), path->data());

  return findUnitInfoFromKey(data, data.pathToUnitInfoBoundsIndex, searchPath);
}

template <typename Compare>
RepoFile::HashMapIndex<Compare> readHashMapIndexHeader(const RepoFileData& data,
    RepoFileIndexes index) {
  auto i = uint32_t(index);
  auto indexOffset = data.indexOffsets[i * 2];
  auto indexSize = data.sizes.indexSizes[i * 2];
  auto dataOffset = data.indexOffsets[i * 2 + 1];
  auto dataSize = data.sizes.indexSizes[i * 2 + 1];
  size_t size = (indexSize / sizeof(uint32_t)) - 1;
  return RepoFile::HashMapIndex<Compare>(size,
    RepoBounds { indexOffset, indexSize }, RepoBounds { dataOffset, dataSize });
}

RepoFile::ListIndex readListIndexHeader(const RepoFileData& data,
                                        RepoFileIndexes index) {
  auto i = uint32_t(index);
  auto indexOffset = data.indexOffsets[i * 2];
  auto indexSize = data.sizes.indexSizes[i * 2];
  auto dataOffset = data.indexOffsets[i * 2 + 1];
  auto dataSize = data.sizes.indexSizes[i * 2 + 1];
  FTRACE(1, "readListIndexHeader {} {} {} {} {}\n", uint32_t(index), indexOffset,
         indexSize, dataOffset, dataSize);
  size_t size = (indexSize / sizeof(uint32_t)) - 1;
  return RepoFile::ListIndex(size, RepoBounds { indexOffset, indexSize },
                             RepoBounds { dataOffset, dataSize });
}

}

////////////////////////////////////////////////////////////////////////////////

void RepoFile::init(const std::string& path) {
  assertx(!s_repoFileData);
  s_repoFileData = std::make_unique<RepoFileData>();
  s_repoFileData->path = path;

  auto& data = *s_repoFileData;
  data.fd = FD{path, O_CLOEXEC | O_RDONLY};

  {
    std::remove_cv_t<decltype(kMagic)> magic;
    data.fd.read(magic.data(), sizeof(magic));
    always_assert_flog(
      magic == kMagic,
      "Incorrect magic bytes in {}",
      path
    );
  }

  {
    auto const version = data.fd.readInt<decltype(kCurrentVersion)>();
    always_assert_flog(
      version == kCurrentVersion,
      "Unsupported version in {} (expected {}, got {})",
      path, kCurrentVersion, version
    );
  }

  auto const repoSchemaSize = data.fd.readInt<uint8_t>();

  {
    std::string repoSchema;
    repoSchema.resize(repoSchemaSize);
    data.fd.read(repoSchema.data(), repoSchema.size());
    always_assert_flog(
      repoSchema == repoSchemaId(),
      "Mismatched repo-schema in {} (expected {}, got {})",
      path, repoSchemaId(), repoSchema
    );
  }

  {
    data.sizes.read(data.fd);
    data.fileSize = data.fd.fileSize();

    auto offset =
      sizeof(kMagic) + sizeof(kCurrentVersion) +
      sizeof(uint8_t) + repoSchemaSize +
      SizeHeader::kFileSize;

    auto const check = [&] (const char* what, size_t size, size_t limit) {
      always_assert_flog(
        size <= limit,
        "Invalid section size for {}: {} is {} (larger than limit of {})",
        path, what, size, limit
      );
      always_assert_flog(
        offset <= data.fileSize,
        "Corrupted size table for {}: "
        "calculated offset {} for {} is greater than file size {}",
        path, offset, what, data.fileSize
      );
    };

    check("unit-emitters", 0, 0);
    data.unitEmittersOffset = offset;
    offset += data.sizes.unitEmittersSize;

    check("global data", data.sizes.globalDataSize, kGlobalDataSizeLimit);
    data.globalDataOffset = offset;
    offset += data.sizes.globalDataSize;

    for (auto i = 0; i < uint32_t(RepoFileIndexes::SIZE) * 2; i++) {
      check("index", data.sizes.indexSizes[i], kIndexSizeLimit);
      data.indexOffsets[i] = offset;
      offset += data.sizes.indexSizes[i];
    }

    always_assert_flog(
      offset == data.fileSize,
      "Corrupted size table for {}: "
      "calculated end of data offset {} does not match file size {}",
      path, offset, data.fileSize
    );
  }

  // We load global data eagerly
  {
    auto blob = data.fd.readBlob(
      data.globalDataOffset,
      data.sizes.globalDataSize
    );
    blob.decoder(data.globalData);
    blob.decoder.assertDone();
  }
}

void RepoFile::destroy() {
  if (!s_repoFileData) return;
  s_repoFileData.reset();
}

void RepoFile::postfork() {
  if (!s_repoFileData) return;
  // Save the file path, then destroy and re-init state.
  auto const path = s_repoFileData->path;
  RepoFile::destroy();
  RepoFile::init(path);
}

const RepoGlobalData& RepoFile::globalData() {
  assertx(s_repoFileData);
  return s_repoFileData->globalData;
}

void RepoFile::loadGlobalTables(bool loadAutoloadMap) {
  assertx(s_repoFileData);
  assertx(!s_repoFileData->loadedGlobalTables.load());
  auto& data = *s_repoFileData;

  data.unitSymbolsIndex = readListIndexHeader(data,
                                              RepoFileIndexes::UNIT_SYMBOLS);
  data.unitInfosIndex = readListIndexHeader(data, RepoFileIndexes::UNIT_INFOS);

  data.pathToUnitInfoBoundsIndex = readHashMapIndexHeader<string_data_same>(
    data, RepoFileIndexes::PATH_TO_UNIT_INFO);

  // Repo autoload map
  if (loadAutoloadMap) {
    AutoloadHandler::setRepoAutoloadMap(
      std::make_unique<RepoAutoloadMap>(
        readHashMapIndexHeader<string_data_isame>(data,
          RepoFileIndexes::AUTOLOAD_TYPES),
        readHashMapIndexHeader<string_data_isame>(data,
          RepoFileIndexes::AUTOLOAD_FUNCS),
        readHashMapIndexHeader<string_data_same>(data,
          RepoFileIndexes::AUTOLOAD_CONSTANTS),
        readHashMapIndexHeader<string_data_isame>(data,
          RepoFileIndexes::AUTOLOAD_TYPEALIASES),
        readHashMapIndexHeader<string_data_same>(data,
          RepoFileIndexes::AUTOLOAD_MODULES)
      )
    );
  }

  data.loadedGlobalTables.store(true);
}

std::unique_ptr<UnitEmitter>
RepoFile::loadUnitEmitter(const StringData* path,
                          const RepoUnitInfo* info,
                          const Native::FuncTable& nativeFuncs,
                          bool lazy) {
  assertx(s_repoFileData);
  assertx(s_repoFileData->loadedGlobalTables.load());
  auto& data = *s_repoFileData;

  if (info == nullptr) {
    assertx(path->isStatic());
    info = findUnitInfoFromPath(data, path);
    if (!info) return nullptr;
    assertx(info->path == path);
  }

  auto blob = data.fd.readBlob(
    data.unitEmittersOffset + info->emitterLocation.offset,
    info->emitterLocation.size
  );

  auto ue = std::make_unique<UnitEmitter>(SHA1{ (uint64_t)info->unitSn },
                                          SHA1{}, nativeFuncs,
                                          RepoOptions::defaults().packageInfo());
  ue->m_filepath = info->path;
  ue->m_sn = info->unitSn;
  ue->serde(blob.decoder, lazy);
  ue->finish();
  blob.decoder.assertDone();
  return ue;
}

size_t RepoFile::remainingSizeOfUnit(int64_t unitSn, Token token) {
  assertx(s_repoFileData);
  assertx(s_repoFileData->loadedGlobalTables.load());
  auto const& data = *s_repoFileData;
  auto info = getUnitInfoFromUnitSn(data, unitSn);
  assertx(token <= info.emitterLocation.size);
  return info.emitterLocation.size - token;
}

void RepoFile::readRawFromUnit(int64_t unitSn, Token token,
                               unsigned char* ptr, size_t len) {
  assertx(s_repoFileData);
  assertx(s_repoFileData->loadedGlobalTables.load());
  assertx(unitSn >= 0);
  auto const& data = *s_repoFileData;
  auto info = getUnitInfoFromUnitSn(data, unitSn);
  assertx(token <= info.emitterLocation.size);
  always_assert(token + len <= info.emitterLocation.size);
  data.fd.pread(
    ptr, len, data.unitEmittersOffset + info.emitterLocation.offset + token
  );
}

const StringData* RepoFile::findUnitPath(int64_t unitSn) {
  assertx(s_repoFileData);
  assertx(s_repoFileData->loadedGlobalTables.load());
  assertx(unitSn >= 0);
  auto& data = *s_repoFileData;
  if (unitSn >= data.unitInfosIndex.size) return nullptr;
  auto info = getUnitInfoFromUnitSn(data, unitSn);
  return info.path;
}

const StringData* RepoFile::findUnitPath(const SHA1& sha1) {
  // SHA1s aren't a thing for RepoFile. When we create a UnitEmitter,
  // we just set the SHA1 to be the SN. So, if you ask for an
  // UnitEmitter by SHA1, we'll only have it if all the words are zero
  // except the last one (which we do a normal SN search for). We only
  // have this function at all for compatibility for tc-print.
  always_assert(sha1.q.size() == 5);
  if (sha1.q[0] != 0 ||
      sha1.q[1] != 0 ||
      sha1.q[2] != 0 ||
      sha1.q[3] != 0) {
    return nullptr;
  }
  return findUnitPath(sha1.q[4]);
}

template <typename Compare>
const RepoUnitInfo* RepoFile::findUnitInfo(const HashMapIndex<Compare>& map,
                                           const StringData* key) {
  // We need to check here because sometime people call this before RepoFileData
  // has been inited
  if (map.size == 0) {
    return nullptr;
  }

  assertx(s_repoFileData);
  assertx(s_repoFileData->loadedGlobalTables.load());
  auto& data = *s_repoFileData;
  return findUnitInfoFromKey(data, map, key);
}

template
const RepoUnitInfo* RepoFile::findUnitInfo(
  const CaseInsensitiveHashMapIndex& map, const StringData* key);

template
const RepoUnitInfo* RepoFile::findUnitInfo(
  const CaseSensitiveHashMapIndex& map, const StringData* key);


const RepoUnitSymbols* RepoFile::findUnitSymbols(const StringData* path) {
  assertx(s_repoFileData);
  assertx(s_repoFileData->loadedGlobalTables.load());
  auto& data = *s_repoFileData;

  // Convert to relative path before looking in the cache because that is what
  // is used when setting the cache
  if (!path->empty()) {
    path = relativePathToSourceRoot(path);
  }

  auto acc = data.pathToSymbols.find(path);
  if (acc != data.pathToSymbols.cend()) {
    return &acc->second;
  }

  auto info = findUnitInfoFromPath(data, path);
  if (!info) {
    return nullptr;
  }

  auto res = getUnitSymbolsFromBounds(data, info->symbolsLocation);
  auto insertRes = data.pathToSymbols.insert(info->path, res);
  return &insertRes.first->second;
}

std::vector<const StringData*> RepoFile::enumerateUnits() {
  assertx(s_repoFileData);
  assertx(s_repoFileData->loadedGlobalTables.load());
  auto& data = *s_repoFileData;

  std::vector<const StringData*> ret;
  ret.reserve(data.unitInfosIndex.size);

  auto dataBlob = data.fd.readBlob(data.unitInfosIndex.dataBounds.offset,
                                   data.unitInfosIndex.dataBounds.size);
  while (dataBlob.decoder.remaining() > 0) {
    RepoUnitInfo info;
    dataBlob.decoder(info);
    ret.push_back(info.path);
  }

  // Maintain deterministic order
  std::sort(
    ret.begin(), ret.end(),
    [] (const StringData* a, const StringData* b) {
      return strcmp(a->data(), b->data()) < 0;
    }
  );
  return ret;
}

////////////////////////////////////////////////////////////////////////////////
}
