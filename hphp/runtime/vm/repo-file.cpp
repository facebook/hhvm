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

#include "hphp/util/blob-writer.h"
#include "hphp/util/build-info.h"
#include "hphp/util/lock-free-ptr-wrapper.h"

#include "hphp/zend/zend-string.h"

#include <folly/String.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <tuple>

TRACE_SET_MOD(repo_file);

namespace HPHP {

////////////////////////////////////////////////////////////////////////////////

namespace {

enum class RepoFileChunks {
  UNIT_EMITTERS,
  GLOBAL_DATA,
  PACKAGE_INFO,

  SIZE // Leave last!
};

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

constexpr Blob::Magic kMagic{ 'H', 'H', 'B', 'C' };

constexpr Blob::Version kCurrentVersion = 1;

// Arbitrary limits on the size of various sections. The sizes are all
// 64-bits, but we don't allow the full range so that a corrupted file
// won't cause us to try to pre-allocate huge amounts of memory. These
// limits were sized so that we should never exceed them, but if we
// ever do, we can just raise them.
constexpr size_t kUnitEmitterSizeLimit = 1ull << 33;
constexpr size_t kGlobalDataSizeLimit  = 1ull << 28;
constexpr size_t kPackageInfoSizeLimit = 1ull << 28;
constexpr size_t kIndexSizeLimit       = 1ull << 28;

// If you hit this limit you also need to change Blob::HashMapIndex::Bucket.
constexpr size_t kIndexDataSizeLimit   = std::numeric_limits<uint32_t>::max();

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
struct RepoFileBuilder::Data : Blob::Writer<RepoFileChunks, RepoFileIndexes> {
  struct UnitEmitterIndex {
    const StringData* path;
    int64_t sn;
    Blob::Bounds location;
  };
  std::vector<UnitEmitterIndex> unitEmittersIndex;
};

////////////////////////////////////////////////////////////////////////////////

RepoFileBuilder::RepoFileBuilder(const std::string& path)
  : m_data{std::make_unique<Data>()}
{
  m_data->header(path, kMagic, kCurrentVersion);
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
      { m_data->sizes.get(RepoFileChunks::UNIT_EMITTERS), size }
    }
  );

  m_data->write(RepoFileChunks::UNIT_EMITTERS, ue.blob.data(), size);
}

void RepoFileBuilder::finish(const RepoGlobalData& global,
                             const RepoAutoloadMapBuilder& autoloadMap,
                             const PackageInfo& packageInfo) {
  assertx(m_data);

  // Global data
  {
    BlobEncoder encoder;
    encoder(global);
    m_data->write(RepoFileChunks::GLOBAL_DATA, encoder.data(), encoder.size());
    always_assert(
      m_data->sizes.get(RepoFileChunks::GLOBAL_DATA) <= kGlobalDataSizeLimit);
  }

  // Package Info
  {
    BlobEncoder encoder;
    encoder(packageInfo);
    m_data->write(RepoFileChunks::PACKAGE_INFO, encoder.data(), encoder.size());
    always_assert(
      m_data->sizes.get(RepoFileChunks::PACKAGE_INFO) <= kPackageInfoSizeLimit);
  }

  // Unit Symbols
  std::vector<Blob::Bounds> unitSymbolsBounds;
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

    unitSymbolsBounds = m_data->listIndex(RepoFileIndexes::UNIT_SYMBOLS, list);
  }

  // Unit Infos
  std::vector<Blob::Bounds> unitInfosBounds;
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

    unitInfosBounds = m_data->listIndex(RepoFileIndexes::UNIT_INFOS, list);
  }

  // Repo Autoload Map
  // Symbol to Blob::Bounds for the UnitInfo
  {
    auto key_lambda = [](auto const& it) { return it.first->toCppString(); };
    auto value_lambda = [&](auto const& it) {
      return &unitInfosBounds[it.second];
    };

    m_data->hashMapIndex<Blob::Bounds, false>(
      RepoFileIndexes::AUTOLOAD_TYPES, autoloadMap.getTypes(),
      key_lambda, value_lambda);
    m_data->hashMapIndex<Blob::Bounds, false>(
      RepoFileIndexes::AUTOLOAD_FUNCS, autoloadMap.getFuncs(),
      key_lambda, value_lambda);
    m_data->hashMapIndex<Blob::Bounds, true>(
      RepoFileIndexes::AUTOLOAD_CONSTANTS, autoloadMap.getConstants(),
      key_lambda, value_lambda);
    m_data->hashMapIndex<Blob::Bounds, false>(
      RepoFileIndexes::AUTOLOAD_TYPEALIASES, autoloadMap.getTypeAliases(),
      key_lambda, value_lambda);
    m_data->hashMapIndex<Blob::Bounds, true>(
      RepoFileIndexes::AUTOLOAD_MODULES, autoloadMap.getModules(),
      key_lambda, value_lambda);
  }

  // Path to Blob::Bounds for the UnitInfo
  m_data->hashMapIndex<Blob::Bounds, true>(
    RepoFileIndexes::PATH_TO_UNIT_INFO, m_data->unitEmittersIndex,
    [](auto const& unit) { return unit.path->toCppString(); },
    [&](auto const& unit) { return &unitInfosBounds[unit.sn]; });

  auto data = std::move(m_data);
  data->finish();
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
struct RepoFileData : Blob::Reader<RepoFileChunks, RepoFileIndexes> {
  RepoGlobalData globalData;

  PackageInfo packageInfo;

  Blob::CaseSensitiveHashMapIndex pathToUnitInfoBoundsIndex;
  Blob::ListIndex unitInfosIndex;
  Blob::ListIndex unitSymbolsIndex;

  std::atomic<bool> loadedGlobalTables{false};

  using UnitSnToUnitInfoMap = folly_concurrent_hash_map_simd<int64_t,
                                                            RepoUnitInfo>;
  mutable UnitSnToUnitInfoMap snToUnitInfo{};

  using PathToSymbolsMap = folly_concurrent_hash_map_simd<
    const StringData*, RepoUnitSymbols, string_data_hash, string_data_same>;
  mutable PathToSymbolsMap pathToSymbols{};
};

std::unique_ptr<RepoFileData> s_repoFileData{};

const RepoUnitInfo& getUnitInfoFromUnitSn(const RepoFileData& data,
                                          int64_t unitSn) {
  auto index = data.unitInfosIndex;
  assertx(unitSn >= 0 && unitSn < index.size);

  auto acc = data.snToUnitInfo.find(unitSn);
  if (acc != data.snToUnitInfo.cend()) {
    return acc->second;
  }

  auto res = data.getFromIndex<RepoUnitInfo>(index, unitSn);
  assertx(res);
  auto insertRes = data.snToUnitInfo.insert(unitSn, *res);
  return insertRes.first->second;
}

const RepoUnitInfo& getUnitInfoFromBounds(const RepoFileData& data,
                                          const Blob::Bounds& bounds) {
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
                                         const Blob::Bounds& bounds) {
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

template <bool CaseSensitive>
const RepoUnitInfo* findUnitInfoFromKey(
    const RepoFileData& data, const Blob::HashMapIndex<CaseSensitive>& map,
    const StringData* key) {
  auto bounds = data.getFromIndex<Blob::Bounds>(map, key->toCppString());
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

}

////////////////////////////////////////////////////////////////////////////////

void RepoFile::init(const std::string& path) {
  assertx(!s_repoFileData);
  s_repoFileData = std::make_unique<RepoFileData>();
  auto& data = *s_repoFileData;
  data.init(path, kMagic, kCurrentVersion);

  data.check(RepoFileChunks::UNIT_EMITTERS, 0);
  data.check(RepoFileChunks::GLOBAL_DATA, kGlobalDataSizeLimit);
  data.check(RepoFileChunks::PACKAGE_INFO, kPackageInfoSizeLimit);

  for (auto i = 0; i < uint32_t(RepoFileIndexes::SIZE); i++) {
    data.check(RepoFileIndexes(i), kIndexSizeLimit, kIndexDataSizeLimit);
  }

  // We load global data eagerly
  {
    auto blob = data.fd.readBlob(
      data.offsets.get(RepoFileChunks::GLOBAL_DATA),
      data.sizes.get(RepoFileChunks::GLOBAL_DATA)
    );
    blob.decoder(data.globalData);
    blob.decoder.assertDone();
  }

  // We load package info eagerly
  {
    auto blob = data.fd.readBlob(
      data.offsets.get(RepoFileChunks::PACKAGE_INFO),
      data.sizes.get(RepoFileChunks::PACKAGE_INFO)
    );
    blob.decoder(data.packageInfo);
    blob.decoder.assertDone();
  }
}

void RepoFile::destroy() {
  if (!s_repoFileData) return;
  s_repoFileData.reset();
}

const RepoGlobalData& RepoFile::globalData() {
  assertx(s_repoFileData);
  return s_repoFileData->globalData;
}

void RepoFile::loadGlobalTables(bool loadAutoloadMap) {
  assertx(s_repoFileData);
  assertx(!s_repoFileData->loadedGlobalTables.load());
  auto& data = *s_repoFileData;

  data.unitSymbolsIndex = data.listIndex(RepoFileIndexes::UNIT_SYMBOLS);
  data.unitInfosIndex = data.listIndex(RepoFileIndexes::UNIT_INFOS);

  data.pathToUnitInfoBoundsIndex = data.hashMapIndex<true>(
    RepoFileIndexes::PATH_TO_UNIT_INFO);

  // Repo autoload map
  if (loadAutoloadMap) {
    AutoloadHandler::setRepoAutoloadMap(
      std::make_unique<RepoAutoloadMap>(
        data.hashMapIndex<false>(RepoFileIndexes::AUTOLOAD_TYPES),
        data.hashMapIndex<false>(RepoFileIndexes::AUTOLOAD_FUNCS),
        data.hashMapIndex<true>(RepoFileIndexes::AUTOLOAD_CONSTANTS),
        data.hashMapIndex<false>(RepoFileIndexes::AUTOLOAD_TYPEALIASES),
        data.hashMapIndex<true>(RepoFileIndexes::AUTOLOAD_MODULES)
      )
    );
  }

  data.loadedGlobalTables.store(true);
}

const PackageInfo& RepoFile::packageInfo() {
  assertx(s_repoFileData);
  return s_repoFileData->packageInfo;
}

std::unique_ptr<UnitEmitter>
RepoFile::loadUnitEmitter(const StringData* path,
                          const RepoUnitInfo* info,
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
    data.offsets.get(RepoFileChunks::UNIT_EMITTERS) +
      info->emitterLocation.offset,
    info->emitterLocation.size
  );

  auto ue = std::make_unique<UnitEmitter>(SHA1{ (uint64_t)info->unitSn }, SHA1{},
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
  data.fd.pread(ptr, len,
    data.offsets.get(RepoFileChunks::UNIT_EMITTERS) +
      info.emitterLocation.offset + token
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

template <bool CaseSensitive>
const RepoUnitInfo* RepoFile::findUnitInfo(
    const Blob::HashMapIndex<CaseSensitive>& map, const StringData* key) {
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
  const Blob::CaseInsensitiveHashMapIndex& map, const StringData* key);

template
const RepoUnitInfo* RepoFile::findUnitInfo(
  const Blob::CaseSensitiveHashMapIndex& map, const StringData* key);


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

std::size_t RepoFile::numUnits() {
  assertx(s_repoFileData);
  assertx(s_repoFileData->loadedGlobalTables.load());
  return s_repoFileData->unitInfosIndex.size;
}

////////////////////////////////////////////////////////////////////////////////
}
