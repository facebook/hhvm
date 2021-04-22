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

#include "hphp/runtime/vm/blob-helper.h"
#include "hphp/runtime/vm/repo-autoload-map-builder.h"

#include "hphp/util/build-info.h"
#include "hphp/util/htonll.h"

#include <folly/FileUtil.h>
#include <folly/String.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

namespace HPHP {

///////////////////////////////////////////////////////////////////////////////

namespace {

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

  void pread(void* data, size_t size, size_t offset) const {
    auto const read = folly::preadFull(m_fd, data, size, offset);
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

uint8_t  hostToFile(uint8_t v)  { return v; }
uint16_t hostToFile(uint16_t v) { return htons(v); }
uint64_t hostToFile(uint64_t v) { return htonll(v); }

uint8_t  fileToHost(uint8_t v)  { return v; }
uint16_t fileToHost(uint16_t v) { return ntohs(v); }
uint64_t fileToHost(uint64_t v) { return ntohll(v); }

// Write an integer in network byte-order
template <typename T> void writeInt(const FD& fd, T v) {
  v = hostToFile(v);
  fd.write(&v, sizeof(v));
}

// Read a network byte-order integer
template <typename T> std::remove_cv_t<T> readInt(const FD& fd) {
  std::remove_cv_t<T> v;
  fd.read(&v, sizeof(v));
  return fileToHost(v);
}

struct Blob {
  std::unique_ptr<char[]> buffer;
  BlobDecoder decoder;
};

Blob loadBlob(const FD& fd, size_t offset, size_t size, bool useGlobalIds) {
  auto buffer = std::make_unique<char[]>(size);
  if (size > 0) fd.pread(buffer.get(), size, offset);
  BlobDecoder decoder{buffer.get(), size, useGlobalIds};
  return { std::move(buffer), std::move(decoder) };
}

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
constexpr size_t kGlobalDataSizeLimit        = 1ull << 28;
constexpr size_t kUnitEmittersIndexSizeLimit = 1ull << 32;
constexpr size_t kAutoloadMapSizeLimit       = 1ull << 32;
constexpr size_t kArrayTypesSizeLimit        = 1ull << 28;
constexpr size_t kLiteralTableSizeLimit      = 1ull << 33;
constexpr size_t kUnitEmitterSizeLimit       = 1ull << 33;
constexpr size_t kLineTableSizeLimit         = 1ull << 32;

// Blob of data containing the sizes of the various sections. This is
// stored near the beginning of the file and can be used to compute
// offsets for the sections. Since we don't know the sizes until after
// we write everything, we have to write these as fixed size integers.
struct SizeHeader {
  uint64_t unitEmittersSize      = 0;
  uint64_t unitEmittersIndexSize = 0;
  uint64_t globalDataSize        = 0;
  uint64_t autoloadMapSize       = 0;
  uint64_t arrayTypesSize        = 0;
  uint64_t literalTableSize      = 0;

  constexpr static size_t kFileSize =
    sizeof(unitEmittersSize) +
    sizeof(unitEmittersIndexSize) +
    sizeof(globalDataSize) +
    sizeof(autoloadMapSize) +
    sizeof(arrayTypesSize) +
    sizeof(literalTableSize);

  void write(const FD& fd) const {
    writeInt(fd, unitEmittersSize);
    writeInt(fd, unitEmittersIndexSize);
    writeInt(fd, globalDataSize);
    writeInt(fd, autoloadMapSize);
    writeInt(fd, arrayTypesSize);
    writeInt(fd, literalTableSize);
  }

  void read(const FD& fd) {
    unitEmittersSize      = readInt<decltype(unitEmittersSize)>(fd);
    unitEmittersIndexSize = readInt<decltype(unitEmittersIndexSize)>(fd);
    globalDataSize        = readInt<decltype(globalDataSize)>(fd);
    autoloadMapSize       = readInt<decltype(autoloadMapSize)>(fd);
    arrayTypesSize        = readInt<decltype(arrayTypesSize)>(fd);
    literalTableSize      = readInt<decltype(literalTableSize)>(fd);
  }
};

}

///////////////////////////////////////////////////////////////////////////////

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
    size_t offset;
  };
  std::vector<UnitEmitterIndex> unitEmittersIndex;
};

///////////////////////////////////////////////////////////////////////////////

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
  writeInt(m_data->fd, kCurrentVersion);

  auto const repoSchema = repoSchemaId();
  always_assert(repoSchema.size() < 256);
  writeInt(m_data->fd, (uint8_t)repoSchema.size());
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

void RepoFileBuilder::add(const UnitEmitter& ue) {
  assertx(m_data);
  assertx(ue.m_filepath->isStatic());
  assertx(ue.m_sn >= 0);

  m_data->unitEmittersIndex.emplace_back(
    RepoFileBuilder::Data::UnitEmitterIndex{
      ue.m_filepath,
      ue.m_sn,
      m_data->sizes.unitEmittersSize
    }
  );

  BlobEncoder blob{ue.useGlobalIds()};
  blob(ue.useGlobalIds());
  const_cast<UnitEmitter&>(ue).serde(blob);
  auto const size = blob.size();
  always_assert(size <= kUnitEmitterSizeLimit);
  m_data->fd.write(blob.data(), size);
  m_data->sizes.unitEmittersSize += size;
}

void RepoFileBuilder::finish(const RepoGlobalData& global,
                             const RepoAutoloadMapBuilder& autoloadMap) {
  assertx(m_data);

  // Unit emitter indices (which lets us find a particular
  // UnitEmitter).
  {
    // Verify in debug builds that all SNs are unique.
    DEBUG_ONLY hphp_fast_set<int64_t> seenSN;
    BlobEncoder encoder{true};

    always_assert(
      m_data->unitEmittersIndex.size() <= std::numeric_limits<uint32_t>::max()
    );
    encoder((uint32_t)m_data->unitEmittersIndex.size());
    for (auto const& index : m_data->unitEmittersIndex) {
      assertx(index.sn < m_data->unitEmittersIndex.size());
      assertx(seenSN.emplace(index.sn).second);
      encoder(index.path)(index.sn)(index.offset);
    }
    m_data->fd.write(encoder.data(), encoder.size());
    m_data->sizes.unitEmittersIndexSize = encoder.size();
    always_assert(
      m_data->sizes.unitEmittersIndexSize <= kUnitEmittersIndexSizeLimit
    );
  }

  // Global data
  {
    BlobEncoder encoder{false};
    encoder(global);
    m_data->fd.write(encoder.data(), encoder.size());
    m_data->sizes.globalDataSize = encoder.size();
    always_assert(m_data->sizes.globalDataSize <= kGlobalDataSizeLimit);
  }

  // Repo autoload map
  {
    BlobEncoder encoder{true};
    autoloadMap.serde(encoder);
    m_data->fd.write(encoder.data(), encoder.size());
    m_data->sizes.autoloadMapSize = encoder.size();
    always_assert(m_data->sizes.autoloadMapSize <= kAutoloadMapSizeLimit);
   }

  // Global array type table
  {
    BlobEncoder encoder{true};
    globalArrayTypeTable().serde(encoder);
    m_data->fd.write(encoder.data(), encoder.size());
    m_data->sizes.arrayTypesSize = encoder.size();
    always_assert(m_data->sizes.arrayTypesSize <= kArrayTypesSizeLimit);
  }

  // Literal string table
  {
    auto& litstrTable = LitstrTable::get();
    litstrTable.setReading();

    BlobEncoder encoder{false};

    auto const numLitstrs = litstrTable.numLitstrs();
    always_assert(
      numLitstrs <= std::numeric_limits<uint32_t>::max()
    );
    encoder((uint32_t)numLitstrs);
    DEBUG_ONLY int next = 1;
    LitstrTable::get().forEachLitstr(
      [&] (int i, const StringData* name) {
        assertx(i == next);
        encoder(name);
        ++next;
      }
    );
    assertx(next == numLitstrs);
    m_data->fd.write(encoder.data(), encoder.size());
    m_data->sizes.literalTableSize = encoder.size();
    always_assert(m_data->sizes.literalTableSize <= kLiteralTableSizeLimit);
  }

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

///////////////////////////////////////////////////////////////////////////////

namespace {

struct Bounds {
  size_t offset = 0;
  size_t size = 0;
};

struct UnitInfo {
  const StringData* path = nullptr;
  Bounds location;
};

// Reader state
struct RepoFileData {
  FD fd;

  std::string path;

  SizeHeader sizes;
  uint64_t fileSize = 0;

  uint64_t unitEmittersOffset = 0;
  uint64_t unitEmittersIndexOffset = 0;
  uint64_t globalDataOffset = 0;
  uint64_t autoloadMapOffset = 0;
  uint64_t arrayTypesOffset = 0;
  uint64_t literalTableOffset = 0;

  RepoGlobalData globalData;

  hphp_fast_map<const StringData*, int64_t> pathToSN;
  std::vector<UnitInfo> unitInfo;

  std::vector<Bounds> literals;

  std::atomic<bool> loadedLitstrTable{false};
  std::atomic<bool> loadedUnitEmitterIndices{false};
};

std::unique_ptr<RepoFileData> s_repoFileData{};

}

///////////////////////////////////////////////////////////////////////////////

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
    auto const version = readInt<decltype(kCurrentVersion)>(data.fd);
    always_assert_flog(
      version == kCurrentVersion,
      "Unsupported version in {} (expected {}, got {})",
      path, kCurrentVersion, version
    );
  }

  auto const repoSchemaSize = readInt<uint8_t>(data.fd);

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

    check(
      "unit-emitters index",
      data.sizes.unitEmittersIndexSize,
      kUnitEmittersIndexSizeLimit
    );
    data.unitEmittersIndexOffset = offset;
    offset += data.sizes.unitEmittersIndexSize;

    check("global data", data.sizes.globalDataSize, kGlobalDataSizeLimit);
    data.globalDataOffset = offset;
    offset += data.sizes.globalDataSize;

    check("autoload-map", data.sizes.autoloadMapSize, kAutoloadMapSizeLimit);
    data.autoloadMapOffset = offset;
    offset += data.sizes.autoloadMapSize;

    check("array-types", data.sizes.arrayTypesSize, kArrayTypesSizeLimit);
    data.arrayTypesOffset = offset;
    offset += data.sizes.arrayTypesSize;

    check("literals", data.sizes.literalTableSize, kLiteralTableSizeLimit);
    data.literalTableOffset = offset;
    offset += data.sizes.literalTableSize;

    always_assert_flog(
      offset == data.fileSize,
      "Corrupted size table for {}: "
      "calculated end of data offset {} does not match file size {}",
      path, offset, data.fileSize
    );
  }

  // We load global data eagerly
  {
    auto blob = loadBlob(
      data.fd,
      data.globalDataOffset,
      data.sizes.globalDataSize,
      false
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

void RepoFile::loadGlobalTables(bool lazyLitstr) {
  assertx(s_repoFileData);
  assertx(!s_repoFileData->loadedLitstrTable.load());
  assertx(!s_repoFileData->loadedUnitEmitterIndices.load());

  auto& data = *s_repoFileData;

  // Literal strings. We need to do this first because other sections
  // might want to use literals.
  {
    auto blob = loadBlob(
      data.fd,
      data.literalTableOffset,
      data.sizes.literalTableSize,
      false
    );

    auto& table = LitstrTable::get();
    assertx(table.numLitstrs() == 0);

    uint32_t numLitstrs;
    blob.decoder(numLitstrs);

    assertx(data.literals.empty());
    data.literals.reserve(numLitstrs);

    NamedEntityPairTable namedInfo;
    namedInfo.resize(numLitstrs, LowStringPtr{nullptr});
    table.setNamedEntityPairTable(std::move(namedInfo));

    for (size_t id = 1; id < numLitstrs; ++id) {
      auto const offset = blob.decoder.advanced();
      if (!lazyLitstr) {
        const StringData* litstr;
        blob.decoder(litstr);
        table.setLitstr(id, litstr);
      } else {
        blob.decoder.skipString();
      }
      auto const post = blob.decoder.advanced();
      assertx(post >= offset);
      data.literals.emplace_back(Bounds{offset, post - offset});
    }
    blob.decoder.assertDone();
    data.loadedLitstrTable.store(true);
  }

  // Unit emitter indices
  {
    auto blob = loadBlob(
      data.fd,
      data.unitEmittersIndexOffset,
      data.sizes.unitEmittersIndexSize,
      true
    );

    uint32_t count;
    blob.decoder(count);

    assertx(data.pathToSN.empty());
    assertx(data.unitInfo.empty());
    data.pathToSN.reserve(count);
    data.unitInfo.resize(count);

    int64_t lastSN = -1;
    auto const setSize = [&] (const StringData* path, size_t offset) {
      if (lastSN < 0) return;
      auto const lastOffset = data.unitInfo[lastSN].location.offset;
      always_assert_flog(
        !path || offset >= lastOffset,
        "Invalid unit-emitter offset for {} in {}: "
        "{} is not monotonically increasing from previous offset {}",
        path, data.path, offset, lastOffset
      );
      auto const size = offset - lastOffset;
      always_assert_flog(
        size <= kUnitEmitterSizeLimit,
        "Invalid unit-emitter for {} in {}: "
        "size {} exceeds maximum allowed of {}",
        data.unitInfo[lastSN].path, data.path, size, kUnitEmitterSizeLimit
      );
      data.unitInfo[lastSN].location.size = size;
    };

    for (size_t i = 0; i < count; ++i) {
      const StringData* path;
      int64_t sn;
      size_t offset;
      blob.decoder(path)(sn)(offset);
      assertx(path);
      assertx(path->isStatic());
      assertx(sn >= 0);
      assertx(sn < count);

      always_assert_flog(
        offset <= data.sizes.unitEmittersSize,
        "Invalid unit-emitter offset for {} in {}: "
        "{} exceeds unit-emitter section size of {}",
        path, data.path, offset, data.sizes.unitEmittersSize
      );

      setSize(path, offset);
      lastSN = sn;

      auto& info = data.unitInfo[sn];
      assertx(!info.path);
      info.path = path;
      info.location = Bounds{offset, 0};

      auto const DEBUG_ONLY insert = data.pathToSN.emplace(path, sn);
      assertx(insert.second);
    }
    blob.decoder.assertDone();
    setSize(nullptr, data.sizes.unitEmittersSize);
    data.loadedUnitEmitterIndices.store(true);
  }

  // Global array type table
  {
    assertx(globalArrayTypeTable().empty());
    auto blob = loadBlob(
      data.fd,
      data.arrayTypesOffset,
      data.sizes.arrayTypesSize,
      true
    );
    globalArrayTypeTable().serde(blob.decoder);
    blob.decoder.assertDone();
  }

  // Repo autoload map
  {
    auto blob = loadBlob(
      data.fd,
      data.autoloadMapOffset,
      data.sizes.autoloadMapSize,
      true
    );
    AutoloadHandler::setRepoAutoloadMap(
      RepoAutoloadMapBuilder::serde(blob.decoder)
    );
    blob.decoder.assertDone();
  }
}

StringData* RepoFile::loadLitstr(size_t id) {
  assertx(s_repoFileData);
  assertx(s_repoFileData->loadedLitstrTable.load());

  auto const& data = *s_repoFileData;

  if (id == 0 || id > data.literals.size()) return nullptr;

  auto const& index = data.literals[id - 1];
  auto blob = loadBlob(
    data.fd,
    data.literalTableOffset + index.offset,
    index.size,
    false
  );
  const StringData* litstr;
  blob.decoder(litstr);
  blob.decoder.assertDone();
  LitstrTable::get().setLitstr(id, litstr);
  return const_cast<StringData*>(litstr);
}

std::unique_ptr<UnitEmitter>
RepoFile::loadUnitEmitter(const StringData* searchPath,
                          const StringData* path,
                          const Native::FuncTable& nativeFuncs,
                          bool lazy) {
  assertx(s_repoFileData);
  assertx(s_repoFileData->loadedUnitEmitterIndices.load());

  auto const& data = *s_repoFileData;

  assertx(searchPath->isStatic());
  assertx(path->isStatic());

  auto const it = data.pathToSN.find(searchPath);
  if (it == data.pathToSN.end()) return nullptr;
  auto const sn = it->second;
  assertx(sn >= 0 && sn < data.unitInfo.size());
  auto const& info = data.unitInfo[sn];
  assertx(info.path);

  auto blob = loadBlob(
    data.fd,
    data.unitEmittersOffset + info.location.offset,
    info.location.size,
    false
  );

  bool useGlobalIds;
  blob.decoder(useGlobalIds);
  blob.decoder.setUseGlobalIds(useGlobalIds);
  auto ue = std::make_unique<UnitEmitter>(
    SHA1{ (uint64_t)sn }, SHA1{}, nativeFuncs, useGlobalIds
  );
  ue->m_filepath = path;
  ue->m_sn = sn;
  ue->serde(blob.decoder);
  blob.decoder.assertDone();
  return ue;
}

void RepoFile::loadBytecode(int64_t unitSn,
                            Token token,
                            unsigned char* bc,
                            size_t bclen) {
  assertx(s_repoFileData);
  assertx(s_repoFileData->loadedUnitEmitterIndices.load());
  assertx(unitSn >= 0);
  auto const& data = *s_repoFileData;
  assertx(unitSn < data.unitInfo.size());
  auto const& info = data.unitInfo[unitSn];
  assertx(info.path);
  assertx(token + bclen <= info.location.size);
  data.fd.pread(
    bc, bclen, data.unitEmittersOffset + info.location.offset + token
  );
}

LineTable RepoFile::loadLineTable(int64_t unitSn,
                                  Token token) {
  assertx(s_repoFileData);
  assertx(s_repoFileData->loadedUnitEmitterIndices.load());
  assertx(unitSn >= 0);
  auto const& data = *s_repoFileData;
  assertx(unitSn < data.unitInfo.size());
  auto const& info = data.unitInfo[unitSn];
  assertx(info.path);

  assertx(token <= info.location.size);
  auto const size = std::min<size_t>(info.location.size - token, 128);
  size_t actualSize;

  {
    auto blob = loadBlob(
      data.fd,
      data.unitEmittersOffset + info.location.offset + token,
      size,
      false
    );

    LineTable lineTable;
    actualSize = FuncEmitter::optDeserializeLineTable(blob.decoder, lineTable);
    if (actualSize <= size) return lineTable;
  }
  always_assert_flog(
    actualSize <= kLineTableSizeLimit,
    "Invalid line table size for {} in {}: "
    "{} exceeds maximum line table size of {}",
    info.path, data.path, actualSize, kLineTableSizeLimit
  );
  always_assert_flog(
    actualSize <= info.location.size - token,
    "Invalid line table size for {} in {}: "
    "{} exceeds remaining unit-emitter size of {}",
    info.path, data.path, actualSize, info.location.size - token
  );

  auto blob = loadBlob(
    data.fd,
    data.unitEmittersOffset + info.location.offset + token,
    actualSize,
    false
  );
  LineTable lineTable;
  FuncEmitter::deserializeLineTable(blob.decoder, lineTable);
  blob.decoder.assertDone();
  return lineTable;
}

int64_t RepoFile::findUnitSN(const StringData* path) {
  assertx(s_repoFileData);
  assertx(s_repoFileData->loadedUnitEmitterIndices.load());
  assertx(path->isStatic());
  auto const& data = *s_repoFileData;
  auto const it = data.pathToSN.find(path);
  if (it == data.pathToSN.end()) return -1;
  assertx(it->second >= 0 && it->second < data.unitInfo.size());
  return it->second;
}

const StringData* RepoFile::findUnitPath(int64_t unitSn) {
  assertx(s_repoFileData);
  assertx(s_repoFileData->loadedUnitEmitterIndices.load());
  assertx(unitSn >= 0);
  auto const& data = *s_repoFileData;
  if (unitSn >= data.unitInfo.size()) return nullptr;
  return data.unitInfo[unitSn].path;
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

std::vector<const StringData*> RepoFile::enumerateUnits() {
  assertx(s_repoFileData);
  assertx(s_repoFileData->loadedUnitEmitterIndices.load());
  auto const& data = *s_repoFileData;
  std::vector<const StringData*> ret;
  ret.reserve(data.pathToSN.size());
  for (auto const& kv : data.pathToSN) ret.emplace_back(kv.first);
  // Maintain deterministic order
  std::sort(
    ret.begin(), ret.end(),
    [] (const StringData* a, const StringData* b) {
      return strcmp(a->data(), b->data()) < 0;
    }
  );
  return ret;
}

///////////////////////////////////////////////////////////////////////////////
}
