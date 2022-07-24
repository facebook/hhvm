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

#include "hphp/util/blob-encoder.h"
#include "hphp/util/build-info.h"
#include "hphp/util/htonll.h"
#include "hphp/util/lock-free-ptr-wrapper.h"

#include "hphp/zend/zend-string.h"

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

Blob loadBlob(const FD& fd, size_t offset, size_t size) {
  auto buffer = std::make_unique<char[]>(size);
  if (size > 0) fd.pread(buffer.get(), size, offset);
  BlobDecoder decoder{buffer.get(), size};
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
constexpr size_t kUnitEmitterSizeLimit       = 1ull << 33;

// Blob of data containing the sizes of the various sections. This is
// stored near the beginning of the file and can be used to compute
// offsets for the sections. Since we don't know the sizes until after
// we write everything, we have to write these as fixed size integers.
struct SizeHeader {
  uint64_t unitEmittersSize      = 0;
  uint64_t unitEmittersIndexSize = 0;
  uint64_t globalDataSize        = 0;
  uint64_t autoloadMapSize       = 0;

  constexpr static size_t kFileSize =
    sizeof(unitEmittersSize) +
    sizeof(unitEmittersIndexSize) +
    sizeof(globalDataSize) +
    sizeof(autoloadMapSize);

  void write(const FD& fd) const {
    writeInt(fd, unitEmittersSize);
    writeInt(fd, unitEmittersIndexSize);
    writeInt(fd, globalDataSize);
    writeInt(fd, autoloadMapSize);
  }

  void read(const FD& fd) {
    unitEmittersSize      = readInt<decltype(unitEmittersSize)>(fd);
    unitEmittersIndexSize = readInt<decltype(unitEmittersIndexSize)>(fd);
    globalDataSize        = readInt<decltype(globalDataSize)>(fd);
    autoloadMapSize       = readInt<decltype(autoloadMapSize)>(fd);
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

void RepoFileBuilder::add(const EncodedUE& ue) {
  assertx(m_data);
  assertx(ue.path->isStatic());
  assertx(ue.sn >= 0);

  m_data->unitEmittersIndex.emplace_back(
    RepoFileBuilder::Data::UnitEmitterIndex{
      ue.path,
      ue.sn,
      m_data->sizes.unitEmittersSize
    }
  );

  auto const size = ue.blob.size();
  always_assert(size <= kUnitEmitterSizeLimit);
  m_data->fd.write(ue.blob.data(), size);
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
    BlobEncoder encoder;

    always_assert(
      m_data->unitEmittersIndex.size() <= std::numeric_limits<uint32_t>::max()
    );
    encoder((uint32_t)m_data->unitEmittersIndex.size());
    for (auto const& index : m_data->unitEmittersIndex) {
      assertx(index.sn < m_data->unitEmittersIndex.size());
      assertx(seenSN.emplace(index.sn).second);
      encoder(index.path->toCppString())(index.sn)(index.offset);
    }
    m_data->fd.write(encoder.data(), encoder.size());
    m_data->sizes.unitEmittersIndexSize = encoder.size();
    always_assert(
      m_data->sizes.unitEmittersIndexSize <= kUnitEmittersIndexSizeLimit
    );
  }

  // Global data
  {
    BlobEncoder encoder;
    encoder(global);
    m_data->fd.write(encoder.data(), encoder.size());
    m_data->sizes.globalDataSize = encoder.size();
    always_assert(m_data->sizes.globalDataSize <= kGlobalDataSizeLimit);
  }

  // Repo autoload map
  {
    BlobEncoder encoder;
    autoloadMap.serde(encoder);
    m_data->fd.write(encoder.data(), encoder.size());
    m_data->sizes.autoloadMapSize = encoder.size();
    always_assert(m_data->sizes.autoloadMapSize <= kAutoloadMapSizeLimit);
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

RepoFileBuilder::EncodedUE::EncodedUE(const UnitEmitter& ue)
  : path{ue.m_filepath}
  , sn{ue.m_sn}
{
  BlobEncoder encoder;
  const_cast<UnitEmitter&>(ue).serde(encoder, false);
  blob = encoder.take();
}

///////////////////////////////////////////////////////////////////////////////

namespace {

struct RepoFileData;

struct Bounds {
  size_t offset = 0;
  size_t size = 0;
};

struct UnitInfo {
  UnsafeLockFreePtrWrapper<StringOrToken> path;
  Bounds location;

  bool valid() const { return path->isToken() || path->ptr(); }
  const StringData* getPath(const RepoFileData&);
};

struct SHA1Hasher {
  size_t operator()(const SHA1& s) const { return s.hash(); }
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

  RepoGlobalData globalData;

  hphp_fast_map<SHA1, int64_t, SHA1Hasher> pathToSN;
  std::vector<UnitInfo> unitInfo;

  std::atomic<bool> loadedUnitEmitterIndices{false};
};

std::unique_ptr<RepoFileData> s_repoFileData{};

// Lazily load the unit's path
const StringData* UnitInfo::getPath(const RepoFileData& data) {
  assertx(valid());

  auto wrapper = path.copy();
  if (wrapper.isPtr()) return wrapper.ptr();

  auto lock = path.lock_for_update();

  wrapper = path.copy();
  if (wrapper.isPtr()) {
    assertx(wrapper.ptr());
    return wrapper.ptr();
  }

  auto const loadedPath = [&] {
    auto const token = wrapper.token();
    assertx(token < data.sizes.unitEmittersIndexSize);

    // We don't know how large the path actually is. Take an initial
    // guess (enough to read the size). If the guess is insufficient,
    // do another read with the proper size.
    size_t actualSize;

    {
      auto const firstSize =
        std::min<size_t>(data.sizes.unitEmittersIndexSize - token, 128);
      auto blob = loadBlob(
        data.fd,
        data.unitEmittersIndexOffset + token,
        firstSize
      );

      actualSize = blob.decoder.peekStdStringSize();
      if (actualSize <= firstSize) {
        std::string pathStr;
        blob.decoder(pathStr);
        assertx(blob.decoder.advanced() == actualSize);
        assertx(!pathStr.empty());
        return makeStaticString(pathStr);
      }
    }

    assertx(actualSize <= data.sizes.unitEmittersIndexSize - token);
    auto blob = loadBlob(
      data.fd,
      data.unitEmittersIndexOffset + token,
      actualSize
    );
    std::string pathStr;
    blob.decoder(pathStr);
    blob.decoder.assertDone();
    assertx(!pathStr.empty());
    return makeStaticString(pathStr);
  }();

  lock.update(StringOrToken::FromPtr(loadedPath));
  return loadedPath;
}

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
  assertx(!s_repoFileData->loadedUnitEmitterIndices.load());

  auto& data = *s_repoFileData;

  // Unit emitter indices
  {
    auto blob = loadBlob(
      data.fd,
      data.unitEmittersIndexOffset,
      data.sizes.unitEmittersIndexSize
    );

    uint32_t count;
    blob.decoder(count);

    assertx(data.pathToSN.empty());
    assertx(data.unitInfo.empty());
    data.pathToSN.reserve(count);
    data.unitInfo.resize(count);

    int64_t lastSN = -1;
    std::string lastPath;
    auto const setSize = [&] (const std::string& path, size_t offset) {
      if (lastSN < 0) return;
      auto const lastOffset = data.unitInfo[lastSN].location.offset;
      always_assert_flog(
        path.empty() || offset >= lastOffset,
        "Invalid unit-emitter offset for {} in {}: "
        "{} is not monotonically increasing from previous offset {}",
        path, data.path, offset, lastOffset
      );
      auto const size = offset - lastOffset;
      always_assert_flog(
        size <= kUnitEmitterSizeLimit,
        "Invalid unit-emitter for {} in {}: "
        "size {} exceeds maximum allowed of {}",
        lastPath, data.path, size, kUnitEmitterSizeLimit
      );
      data.unitInfo[lastSN].location.size = size;
    };

    for (size_t i = 0; i < count; ++i) {
      auto const pathOffset = blob.decoder.advanced();
      std::string path;
      int64_t sn;
      size_t offset;
      blob.decoder(path)(sn)(offset);
      assertx(!path.empty());
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
      lastPath = path;

      auto& info = data.unitInfo[sn];
      assertx(!info.valid());
      info.path = StringOrToken::FromToken(pathOffset);
      info.location = Bounds{offset, 0};

      auto const DEBUG_ONLY insert =
        data.pathToSN.emplace(SHA1{string_sha1(path)}, sn);
      assertx(insert.second);
    }
    blob.decoder.assertDone();
    setSize(std::string{}, data.sizes.unitEmittersSize);
    data.loadedUnitEmitterIndices.store(true);
  }

  // Repo autoload map
  if (loadAutoloadMap) {
    auto blob = loadBlob(
      data.fd,
      data.autoloadMapOffset,
      data.sizes.autoloadMapSize
    );
    AutoloadHandler::setRepoAutoloadMap(
      RepoAutoloadMapBuilder::serde(blob.decoder)
    );
    blob.decoder.assertDone();
  }
}

std::unique_ptr<UnitEmitter>
RepoFile::loadUnitEmitter(const StringData* searchPath,
                          const StringData* path,
                          const Native::FuncTable& nativeFuncs,
                          bool lazy) {
  assertx(s_repoFileData);
  assertx(s_repoFileData->loadedUnitEmitterIndices.load());

  auto& data = *s_repoFileData;

  assertx(path->isStatic());
  assertx(searchPath->isStatic());

  auto const it = data.pathToSN.find(SHA1{string_sha1(searchPath->slice())});
  if (it == data.pathToSN.end()) return nullptr;
  auto const sn = it->second;

  assertx(sn >= 0 && sn < data.unitInfo.size());
  auto& info = data.unitInfo[sn];
  assertx(info.valid());
  if (info.getPath(data) != searchPath) return nullptr;

  auto blob = loadBlob(
    data.fd,
    data.unitEmittersOffset + info.location.offset,
    info.location.size
  );

  auto ue =
    std::make_unique<UnitEmitter>(SHA1{ (uint64_t)sn }, SHA1{}, nativeFuncs);
  ue->m_filepath = path;
  ue->m_sn = sn;
  ue->serde(blob.decoder, lazy);
  ue->finish();
  blob.decoder.assertDone();
  return ue;
}

size_t RepoFile::remainingSizeOfUnit(int64_t unitSn, Token token) {
  assertx(s_repoFileData);
  assertx(s_repoFileData->loadedUnitEmitterIndices.load());
  assertx(unitSn >= 0);
  auto const& data = *s_repoFileData;
  assertx(unitSn < data.unitInfo.size());
  auto const& info = data.unitInfo[unitSn];
  assertx(info.valid());
  assertx(token <= info.location.size);
  return info.location.size - token;
}

void RepoFile::readRawFromUnit(int64_t unitSn, Token token,
                               unsigned char* ptr, size_t len) {
  assertx(s_repoFileData);
  assertx(s_repoFileData->loadedUnitEmitterIndices.load());
  assertx(unitSn >= 0);
  auto const& data = *s_repoFileData;
  assertx(unitSn < data.unitInfo.size());
  auto const& info = data.unitInfo[unitSn];
  assertx(info.valid());
  assertx(token <= info.location.size);
  always_assert(token + len <= info.location.size);
  data.fd.pread(
    ptr, len, data.unitEmittersOffset + info.location.offset + token
  );
}

int64_t RepoFile::findUnitSN(const StringData* path) {
  assertx(s_repoFileData);
  assertx(s_repoFileData->loadedUnitEmitterIndices.load());
  assertx(path->isStatic());
  auto& data = *s_repoFileData;
  auto const it = data.pathToSN.find(SHA1{string_sha1(path->slice())});
  if (it == data.pathToSN.end()) return -1;
  assertx(it->second >= 0 && it->second < data.unitInfo.size());
  auto& info = data.unitInfo[it->second];
  assertx(info.valid());
  return info.getPath(data) == path ? it->second : -1;
}

const StringData* RepoFile::findUnitPath(int64_t unitSn) {
  assertx(s_repoFileData);
  assertx(s_repoFileData->loadedUnitEmitterIndices.load());
  assertx(unitSn >= 0);
  auto& data = *s_repoFileData;
  if (unitSn >= data.unitInfo.size()) return nullptr;
  auto& info = data.unitInfo[unitSn];
  if (!info.valid()) return nullptr;
  return info.getPath(data);
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
  auto& data = *s_repoFileData;
  std::vector<const StringData*> ret;
  ret.reserve(data.pathToSN.size());
  for (auto const& kv : data.pathToSN) {
    assertx(kv.second >= 0 && kv.second < data.unitInfo.size());
    auto& info = data.unitInfo[kv.second];
    assertx(info.valid());
    ret.emplace_back(info.getPath(data));
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

///////////////////////////////////////////////////////////////////////////////
}
