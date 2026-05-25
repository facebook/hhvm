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

#include "hphp/runtime/ext/sbcc/format/sbcc-reader.h"

#include "hphp/runtime/ext/sbcc/format/sbcc-file.h"

#include <folly/FileUtil.h>
#include <fmt/core.h>
#include <folly/ScopeGuard.h>
#include <folly/String.h>

#include "hphp/runtime/base/runtime-option.h"
#include "hphp/runtime/vm/unit-emitter.h"

#include "hphp/util/blob-writer.h"
#include "hphp/util/build-info.h"
#include "hphp/util/trace.h"

namespace HPHP {

TRACE_SET_MOD(sbcc)

///////////////////////////////////////////////////////////////////////////////
// Reader state — follows the VirtualFileSystem::Data pattern.

struct SBCCReader::Data : Blob::Reader<SBCCChunk, SBCCIndex> {
  Blob::HashMapIndex<SHA1Compare> sha1ToEntryIndex;
};

namespace {

// Arbitrary limits on section sizes to prevent huge allocations from a
// corrupted file. Sized so we should never hit them in practice.
constexpr size_t kUEBlobsSizeLimit  = 1ull << 36; // 64 GB
constexpr size_t kIndexSizeLimit    = 1ull << 27; // 128 MB
constexpr size_t kIndexDataSizeLimit = std::numeric_limits<uint32_t>::max();

} // namespace

///////////////////////////////////////////////////////////////////////////////

SBCCReader::SBCCReader() = default;
SBCCReader::~SBCCReader() = default;

bool SBCCReader::initialized() const {
  return m_data != nullptr;
}

size_t SBCCReader::entryCount() const {
  return m_data ? m_data->sha1ToEntryIndex.size : 0;
}

void SBCCReader::init(const std::string& path) {
  // Pre-validate the Blob header before calling data->init(), which aborts
  // on mismatch. This gives the caller a chance to handle errors gracefully.
  {
    auto fd = folly::openNoInt(path.c_str(), O_CLOEXEC | O_RDONLY);
    if (fd < 0) {
      throw std::runtime_error(
        fmt::format("Cannot open SBCC file: {}", path));
    }
    SCOPE_EXIT { ::close(fd); };

    // Read magic (4 bytes) + version (2 bytes, NBO) + schema_len (1 byte)
    unsigned char hdr[7];
    if (folly::readFull(fd, hdr, sizeof(hdr)) != sizeof(hdr)) {
      throw std::runtime_error(
        fmt::format("SBCC file too small: {}", path));
    }

    // Check magic
    if (memcmp(hdr, "SBCC", 4) != 0) {
      throw std::runtime_error(
        fmt::format("Bad magic in SBCC file: {}", path));
    }

    // Check version (network byte order)
    uint16_t version = (uint16_t(hdr[4]) << 8) | hdr[5];
    if (version != kCurrentVersion) {
      throw std::runtime_error(fmt::format(
        "SBCC version mismatch in {} (expected {}, got {})",
        path, kCurrentVersion, version));
    }

    // Check repo schema
    uint8_t schemaLen = hdr[6];
    std::string fileSchema(schemaLen, '\0');
    if (folly::readFull(fd, fileSchema.data(), schemaLen) != schemaLen) {
      throw std::runtime_error(
        fmt::format("Truncated schema in SBCC file: {}", path));
    }
    auto rtSchema = repoSchemaId();
    if (fileSchema != rtSchema) {
      throw std::runtime_error(fmt::format(
        "SBCC repo schema mismatch in {} (expected {}, got {})",
        path, rtSchema, fileSchema));
    }
  }

  auto data = std::make_unique<Data>();

  // Blob::Reader::init() will re-read the header but won't abort since
  // we pre-validated magic, version, and repo schema above.
  data->init(path, {'S', 'B', 'C', 'C'}, kCurrentVersion,
             Blob::ReadMode::MMap);
  data->check(SBCCChunk::UE_BLOBS, kUEBlobsSizeLimit);
  data->check(SBCCIndex::SHA1_TO_ENTRY, kIndexSizeLimit, kIndexDataSizeLimit);

  data->sha1ToEntryIndex =
    data->hashMapIndex<SHA1Compare>(SBCCIndex::SHA1_TO_ENTRY);

  m_data = std::move(data);
}

void SBCCReader::destroy() {
  m_data.reset();
}

std::unique_ptr<UnitEmitter> SBCCReader::lookup(
    const SHA1& sha1,
    const char* filename,
    SBCCLookupResult* result) const {
  if (!m_data) {
    if (result) *result = SBCCLookupResult::Miss;
    return nullptr;
  }

  // Look up via hash map index.
  auto bounds = m_data->getFromIndex<Blob::Bounds, SHA1>(
    m_data->sha1ToEntryIndex,
    sha1);
  if (!bounds) {
    if (result) *result = SBCCLookupResult::Miss;
    return nullptr;
  }

  // Hit — read and deserialize the UE blob.
  static constexpr size_t kMaxUESize = 100u * 1024 * 1024; // 100 MB
  if (bounds->size > kMaxUESize) {
    if (result) *result = SBCCLookupResult::Corrupt;
    return nullptr;
  }

  try {
    auto blob = m_data->fd.readBlob(
      m_data->offsets.get(SBCCChunk::UE_BLOBS) + bounds->offset,
      bounds->size);

    static const auto& pkgInfo = RepoOptions::defaults().packageInfo();
    auto ue = std::make_unique<UnitEmitter>(sha1, SHA1{}, pkgInfo);
    ue->m_filepath = makeStaticString(filename);
    ue->serde(blob.decoder, false);
    ue->finish();
    blob.decoder.assertDone();
    if (result) *result = SBCCLookupResult::Hit;
    return ue;
  } catch ([[maybe_unused]] const std::exception& e) {
    FTRACE(1, "SBCC: corrupt entry for {}: {}\n", filename, e.what());
    if (result) *result = SBCCLookupResult::Corrupt;
    return nullptr;
  } catch (...) {
    FTRACE(1, "SBCC: corrupt entry for {} (unknown exception)\n", filename);
    if (result) *result = SBCCLookupResult::Corrupt;
    return nullptr;
  }
}

///////////////////////////////////////////////////////////////////////////////

} // namespace HPHP
