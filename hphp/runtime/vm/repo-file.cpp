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

#include "hphp/runtime/vm/func-emitter.h"
#include "hphp/runtime/vm/repo-autoload-map-builder.h"
#include "hphp/runtime/vm/repo-global-data.h"
#include "hphp/runtime/vm/unit-emitter.h"

#include "hphp/util/blob-writer.h"

#include <folly/String.h>
#include <magic_enum/magic_enum.hpp>

TRACE_SET_MOD(repo_file)

namespace HPHP {

////////////////////////////////////////////////////////////////////////////////

namespace {

enum class RepoFileChunks {
  UNIT_EMITTERS,
  GLOBAL_DATA,
  PACKAGE_INFO,
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
  if (path->data()[0] == '/' && !Cfg::Server::SourceRoot.empty() &&
    !strncmp(Cfg::Server::SourceRoot.c_str(), path->data(), Cfg::Server::SourceRoot.size())) {
    return makeStaticString(path->data() + Cfg::Server::SourceRoot.size(),
                            path->size() - Cfg::Server::SourceRoot.size());
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
  if (pathData[0] == '/' && !Cfg::Server::SourceRoot.empty() &&
      !strncmp(Cfg::Server::SourceRoot.c_str(), pathData, Cfg::Server::SourceRoot.size())) {
    spath = spath.substr(Cfg::Server::SourceRoot.size());
  }

  sd(unitSn)(spath)(emitterLocation)(symbolsLocation);
}

void RepoUnitInfo::serde(BlobDecoder& sd) {
  std::string spath;

  sd(unitSn)(spath)(emitterLocation)(symbolsLocation);

  if (!Cfg::Server::SourceRoot.empty() && !spath.empty() && spath.c_str()[0] != '/') {
    spath = Cfg::Server::SourceRoot + spath;
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

  void finish(const RepoGlobalData&,
              std::vector<RepoUnitSymbols>,
              const PackageInfo&,
              const RepoAutoloadMapBuilder::TypeNameMap& types,
              const RepoAutoloadMapBuilder::FuncNameMap& funcs,
              const RepoAutoloadMapBuilder::CaseSensitiveMap& constants,
              const RepoAutoloadMapBuilder::TypeNameMap& typeAliases,
              const RepoAutoloadMapBuilder::CaseSensitiveMap& modules);
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

  std::vector<RepoUnitSymbols> unitSymbols(
    m_data->unitEmittersIndex.size());
  auto add_symbols = [&](auto const& symbols, auto type) {
    for (auto const& info : symbols) {
      unitSymbols[info.second].push_back(std::make_pair(info.first, type));
    }
  };

  add_symbols(autoloadMap.getTypes(), RepoSymbolType::TYPE);
  add_symbols(autoloadMap.getFuncs(), RepoSymbolType::FUNC);
  add_symbols(autoloadMap.getConstants(), RepoSymbolType::CONSTANT);
  add_symbols(autoloadMap.getTypeAliases(), RepoSymbolType::TYPE_ALIAS);
  add_symbols(autoloadMap.getModules(), RepoSymbolType::MODULE);

  auto data = std::move(m_data);
  data->finish(
    global,
    std::move(unitSymbols),
    packageInfo,
    autoloadMap.getTypes(),
    autoloadMap.getFuncs(),
    autoloadMap.getConstants(),
    autoloadMap.getTypeAliases(),
    autoloadMap.getModules()
  );
}

void RepoFileBuilder::Data::finish(
    const RepoGlobalData& global,
    std::vector<RepoUnitSymbols> unitSymbols,
    const PackageInfo& packageInfo,
    const RepoAutoloadMapBuilder::TypeNameMap& types,
    const RepoAutoloadMapBuilder::FuncNameMap& funcs,
    const RepoAutoloadMapBuilder::CaseSensitiveMap& constants,
    const RepoAutoloadMapBuilder::TypeNameMap& typeAliases,
    const RepoAutoloadMapBuilder::CaseSensitiveMap& modules) {
  // Global data
  {
    BlobEncoder encoder;
    encoder(global);
    write(RepoFileChunks::GLOBAL_DATA, encoder.data(), encoder.size());
    always_assert(
      sizes.get(RepoFileChunks::GLOBAL_DATA) <= kGlobalDataSizeLimit);
  }

  // Package Info
  {
    BlobEncoder encoder;
    encoder(packageInfo);
    write(RepoFileChunks::PACKAGE_INFO, encoder.data(), encoder.size());
    always_assert(
      sizes.get(RepoFileChunks::PACKAGE_INFO) <= kPackageInfoSizeLimit);
  }

  // Unit Symbols
  auto const unitSymbolsBounds =
    listIndex(RepoFileIndexes::UNIT_SYMBOLS, unitSymbols);

  // Unit Infos
  std::vector<Blob::Bounds> unitInfosBounds;
  {
    std::vector<RepoUnitInfo> list(unitEmittersIndex.size());
    for (auto const& unit : unitEmittersIndex) {
      list[unit.sn] = RepoUnitInfo {
        unit.sn,
        unit.path,
        unit.location,
        unitSymbolsBounds[unit.sn]
      };
    }

    unitInfosBounds = listIndex(RepoFileIndexes::UNIT_INFOS, list);
  }

  // Repo Autoload Map
  // Symbol to Blob::Bounds for the UnitInfo
  {
    auto key_lambda = [](auto const& it) { return it.first->toCppString(); };
    auto value_lambda = [&](auto const& it) {
      return &unitInfosBounds[it.second];
    };

    hashMapIndex<Blob::Bounds, TypeNameCompare>(
      RepoFileIndexes::AUTOLOAD_TYPES, types,
      key_lambda, value_lambda);
    hashMapIndex<Blob::Bounds, FuncNameCompare>(
      RepoFileIndexes::AUTOLOAD_FUNCS, funcs,
      key_lambda, value_lambda);
    hashMapIndex<Blob::Bounds, Blob::CaseSensitiveCompare>(
      RepoFileIndexes::AUTOLOAD_CONSTANTS, constants,
      key_lambda, value_lambda);
    hashMapIndex<Blob::Bounds, TypeNameCompare>(
      RepoFileIndexes::AUTOLOAD_TYPEALIASES, typeAliases,
      key_lambda, value_lambda);
    hashMapIndex<Blob::Bounds, Blob::CaseSensitiveCompare>(
      RepoFileIndexes::AUTOLOAD_MODULES, modules,
      key_lambda, value_lambda);
  }

  // Path to Blob::Bounds for the UnitInfo
  hashMapIndex<Blob::Bounds, Blob::CaseSensitiveCompare>(
    RepoFileIndexes::PATH_TO_UNIT_INFO, unitEmittersIndex,
    [](auto const& unit) { return unit.path->toCppString(); },
    [&](auto const& unit) { return &unitInfosBounds[unit.sn]; });

  Blob::Writer<RepoFileChunks, RepoFileIndexes>::finish();
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
  explicit RepoFileData(
      const std::string& inputPath,
      Blob::ReadMode mode = Blob::ReadMode::PReadOnly) {
    init(inputPath, kMagic, kCurrentVersion, mode);

    check(RepoFileChunks::UNIT_EMITTERS, 0);
    check(RepoFileChunks::GLOBAL_DATA, kGlobalDataSizeLimit);
    check(RepoFileChunks::PACKAGE_INFO, kPackageInfoSizeLimit);

    for (auto index : magic_enum::enum_values<RepoFileIndexes>()) {
      check(index, kIndexSizeLimit, kIndexDataSizeLimit);
    }

    globalData = readChunk<RepoGlobalData>(RepoFileChunks::GLOBAL_DATA);
    packageInfo = readChunk<PackageInfo>(RepoFileChunks::PACKAGE_INFO);
  }

  RepoGlobalData globalData;

  PackageInfo packageInfo;

  CaseSensitiveHashMapIndex pathToUnitInfoBoundsIndex;
  Blob::ListIndex unitInfosIndex;
  Blob::ListIndex unitSymbolsIndex;

  std::atomic<bool> loadedGlobalTables{false};

  using UnitSnToUnitInfoMap = folly_concurrent_hash_map_simd<int64_t,
                                                            RepoUnitInfo>;
  mutable UnitSnToUnitInfoMap snToUnitInfo{};

  using PathToSymbolsMap = folly_concurrent_hash_map_simd<
    const StringData*, RepoUnitSymbols, string_data_hash, string_data_same>;
  mutable PathToSymbolsMap pathToSymbols{};

private:
  template <typename T>
  T readChunk(RepoFileChunks chunk) {
    auto blob = fd.readBlob(offsets.get(chunk), sizes.get(chunk));
    T value;
    blob.decoder(value);
    blob.decoder.assertDone();
    return value;
  }
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

template <typename KeyCompare>
const RepoUnitInfo* findUnitInfoFromKey(
    const RepoFileData& data, const Blob::HashMapIndex<KeyCompare>& map,
    const StringData* key) {
  auto bounds = data.getFromIndex<Blob::Bounds>(map, key->slice());
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
  s_repoFileData = std::make_unique<RepoFileData>(path);
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

  data.pathToUnitInfoBoundsIndex = data.hashMapIndex<Blob::CaseSensitiveCompare>(
    RepoFileIndexes::PATH_TO_UNIT_INFO);

  // Repo autoload map
  if (loadAutoloadMap) {
    AutoloadHandler::setRepoAutoloadMap(
      std::make_unique<RepoAutoloadMap>(
        data.hashMapIndex<TypeNameCompare>(RepoFileIndexes::AUTOLOAD_TYPES),
        data.hashMapIndex<FuncNameCompare>(RepoFileIndexes::AUTOLOAD_FUNCS),
        data.hashMapIndex<Blob::CaseSensitiveCompare>(RepoFileIndexes::AUTOLOAD_CONSTANTS),
        data.hashMapIndex<TypeNameCompare>(RepoFileIndexes::AUTOLOAD_TYPEALIASES),
        data.hashMapIndex<Blob::CaseSensitiveCompare>(RepoFileIndexes::AUTOLOAD_MODULES)
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

template <typename KeyCompare>
const RepoUnitInfo* RepoFile::findUnitInfo(
    const Blob::HashMapIndex<KeyCompare>& map, const StringData* key) {
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
  const CaseSensitiveHashMapIndex& map, const StringData* key);

template
const RepoUnitInfo* RepoFile::findUnitInfo(
  const HashMapTypeIndex& map, const StringData* key);


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
