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

#include "hphp/runtime/ext/extension.h"

#include "hphp/runtime/base/file-util.h"
#include "hphp/runtime/base/file.h"
#include "hphp/runtime/vm/native-data.h"
#include "hphp/util/blob-writer.h"
#include "hphp/util/configs/repo.h"
#include "hphp/util/logger.h"
#include "hphp/util/service-data.h"

#include <cerrno>
#include <cstring>
#include <limits>
#include <memory>
#include <stdexcept>
#include <string>
#include <string_view>
#include <unistd.h>

#include <fcntl.h>
#include <folly/FileUtil.h>
#include <folly/container/F14Map.h>

namespace HPHP {

namespace {

///////////////////////////////////////////////////////////////////////////////

enum class BlobKVChunk {
  VALUES,
};

enum class BlobKVIndex {
  KEY_TO_ENTRY,
};

struct BlobKVEntry {
  Blob::Bounds value;

  template <typename SerDe>
  void serde(SerDe& sd) {
    sd(value);
  }
};

constexpr Blob::Magic kMagic{'H', 'B', 'K', 'V'};
constexpr Blob::Version kCurrentVersion = 1;

constexpr size_t kIndexSizeLimit = std::numeric_limits<uint32_t>::max();
constexpr size_t kIndexDataSizeLimit = std::numeric_limits<uint32_t>::max();

ServiceData::ExportedCounter* s_abandonedWriters =
  ServiceData::createCounter("vm.blobkv.abandoned_writers");

void throwIfRepoMode() {
  if (Cfg::Repo::Authoritative) {
    SystemLib::throwInvalidOperationExceptionObject(
      "HH\\BlobKV\\Writer is only supported in non-authoritative mode"
    );
  }
}

std::string translatePath(const OptString& path, const char* func) {
  FileUtil::checkPathAndError(path, func, 1);
  auto translated = File::TranslatePath(path);
  if (translated.empty()) {
    SystemLib::throwInvalidArgumentExceptionObject(
      fmt::format("{} expects a non-empty path", func)
    );
  }
  return translated.toCppString();
}

std::string stringArg(const OptString& arg) {
  return std::string{arg.data(), static_cast<size_t>(arg.size())};
}

std::string_view stringView(const OptString& arg) {
  return std::string_view{arg.data(), static_cast<size_t>(arg.size())};
}

[[noreturn]] void throwBlobError(const std::exception& error) {
  SystemLib::throwRuntimeExceptionObject(error.what());
}

void removeFile(const std::string& path, const char* description) {
  if (::unlink(path.c_str()) != 0 && errno != ENOENT) {
    Logger::FWarning(
      "Unable to remove {} {}: {}",
      description,
      path,
      folly::errnoStr(errno)
    );
  }
}

void validateValueBounds(
  const std::string& path,
  const std::string& key,
  Blob::Bounds value,
  size_t valuesSize
) {
  if (value.offset <= valuesSize && value.size <= valuesSize - value.offset) {
    return;
  }

  throw std::runtime_error(fmt::format(
    "Corrupt BlobKV entry for key {} in {}: value offset {} and size {} "
    "exceed VALUES chunk size {}",
    key,
    path,
    value.offset,
    value.size,
    valuesSize
  ));
}

struct BlobKVReaderData : Blob::Reader<BlobKVChunk, BlobKVIndex> {
  Blob::HashMapIndex<Blob::CaseSensitiveCompare> keyToEntryIndex;
};

struct BlobKVWriterData : Blob::Writer<BlobKVChunk, BlobKVIndex> {
  folly::F14FastMap<std::string, BlobKVEntry> entries;
};

///////////////////////////////////////////////////////////////////////////////

} // namespace

struct BlobKVReader : SystemLib::ClassLoader<"HH\\BlobKV\\Reader"> {
  BlobKVReader() = default;
  ~BlobKVReader() { sweep(); }

  void sweep() {
    m_data.reset();
  }

  void open(const std::string& path) {
    if (m_data) {
      SystemLib::throwInvalidOperationExceptionObject(
        "HH\\BlobKV\\Reader object was already initialized"
      );
    }

    auto data = std::make_unique<BlobKVReaderData>();
    data->init(
      path,
      kMagic,
      kCurrentVersion,
      Blob::ReadMode::PReadOnly,
      Blob::ErrorMode::Throw
    );
    data->check(BlobKVChunk::VALUES, 0);
    data->check(BlobKVIndex::KEY_TO_ENTRY, kIndexSizeLimit, kIndexDataSizeLimit);
    data->keyToEntryIndex =
      data->hashMapIndex<Blob::CaseSensitiveCompare>(BlobKVIndex::KEY_TO_ENTRY);
    m_data = std::move(data);
  }

  OptString read(const std::string& key) const {
    validate();

    auto const entry =
      m_data->getFromIndex<BlobKVEntry>(m_data->keyToEntryIndex, key);
    if (!entry) {
      return OptString{};
    }

    validateValueBounds(
      m_data->path,
      key,
      entry->value,
      m_data->sizes.get(BlobKVChunk::VALUES)
    );

    OptString value(entry->value.size, ReserveString);
    if (entry->value.size != 0) {
      m_data->fd.pread(
        value.mutableData(),
        entry->value.size,
        m_data->offsets.get(BlobKVChunk::VALUES) + entry->value.offset
      );
    }
    value.setSize(entry->value.size);
    return value;
  }

private:
  void validate() const {
    if (!m_data) {
      SystemLib::throwInvalidOperationExceptionObject(
        "HH\\BlobKV\\Reader object was not initialized"
      );
    }
  }

  std::unique_ptr<BlobKVReaderData> m_data;
};

struct BlobKVWriter : SystemLib::ClassLoader<"HH\\BlobKV\\Writer"> {
  BlobKVWriter() = default;
  ~BlobKVWriter() { sweep(); }

  void sweep() {
    auto const abandoned = m_data && !m_finalized;

    if (abandoned) {
      s_abandonedWriters->increment();
      removeFile(m_data->sourceFilename, "abandoned BlobKV file");
    }
    m_data.reset();

    m_finalized = true;
  }

  void open(const std::string& path) {
    throwIfRepoMode();
    if (m_data || !m_path.empty()) {
      SystemLib::throwInvalidOperationExceptionObject(
        "HH\\BlobKV\\Writer object was already initialized"
      );
    }

    m_path = path;

    auto data = std::make_unique<BlobKVWriterData>();
    try {
      data->exclusiveHeader(
        path,
        kMagic,
        kCurrentVersion,
        Blob::ErrorMode::Throw
      );
    } catch (...) {
      if (data->fd.isOpen()) {
        removeFile(data->sourceFilename, "failed BlobKV file");
      }
      resetOpenState();
      throw;
    }
    m_data = std::move(data);
    m_finalized = false;
  }

  void write(std::string key, std::string_view value) {
    throwIfRepoMode();
    validateMutable();

    try {
      auto const offset = m_data->sizes.get(BlobKVChunk::VALUES);
      if (!value.empty()) {
        m_data->write(BlobKVChunk::VALUES, value.data(), value.size());
      }
      m_data->entries[std::move(key)] =
        BlobKVEntry{Blob::Bounds{offset, value.size()}};
    } catch (...) {
      // A failed write may have appended value bytes without recording their
      // size in the chunk table, leaving the .part unable to produce a
      // consistent file. Poison the writer so a caller that swallows the
      // exception cannot retry finalize() and publish a corrupt artifact.
      poison();
      throw;
    }
  }

  void finalize() {
    throwIfRepoMode();
    validateMutable();

    auto data = std::move(m_data);
    m_finalized = true;

    try {
      data->hashMapIndex<BlobKVEntry, Blob::CaseSensitiveCompare>(
        BlobKVIndex::KEY_TO_ENTRY,
        data->entries,
        [](auto const& it) { return it.first; },
        [](auto const& it) { return &it.second; }
      );

      data->finish();
    } catch (...) {
      auto const part = data->sourceFilename;
      data.reset();
      removeFile(part, "failed BlobKV file");
      throw;
    }
  }

private:
  void validateMutable() const {
    if (!m_data || m_finalized) {
      SystemLib::throwInvalidOperationExceptionObject(
        "HH\\BlobKV\\Writer object is not open for writes"
      );
    }
  }

  void poison() {
    if (m_data) {
      removeFile(m_data->sourceFilename, "failed BlobKV file");
      m_data.reset();
    }
    m_finalized = true;
  }

  void resetOpenState() {
    m_path.clear();
  }

  std::unique_ptr<BlobKVWriterData> m_data;
  std::string m_path;
  bool m_finalized{true};
};

void HHVM_METHOD(BlobKVReader, __construct, const OptString& path) {
  auto const translated = translatePath(path, "HH\\BlobKV\\Reader::__construct");
  try {
    Native::data<BlobKVReader>(this_)->open(translated);
  } catch (const std::exception& error) {
    throwBlobError(error);
  }
}

OptString HHVM_METHOD(BlobKVReader, read, const OptString& key) {
  try {
    return Native::data<BlobKVReader>(this_)->read(stringArg(key));
  } catch (const std::exception& error) {
    throwBlobError(error);
  }
}

void HHVM_METHOD(BlobKVWriter, __construct, const OptString& path) {
  auto const translated = translatePath(path, "HH\\BlobKV\\Writer::__construct");
  try {
    Native::data<BlobKVWriter>(this_)->open(translated);
  } catch (const std::exception& error) {
    throwBlobError(error);
  }
}

void HHVM_METHOD(BlobKVWriter, write, const OptString& key, const OptString& value) {
  try {
    Native::data<BlobKVWriter>(this_)->write(stringArg(key), stringView(value));
  } catch (const std::exception& error) {
    throwBlobError(error);
  }
}

void HHVM_METHOD(BlobKVWriter, finalize) {
  try {
    Native::data<BlobKVWriter>(this_)->finalize();
  } catch (const std::exception& error) {
    throwBlobError(error);
  }
}

static struct BlobKVExtension final : Extension {
  BlobKVExtension() : Extension("blobkv", NO_EXTENSION_VERSION_YET, "hphp_hphpi") {}

  std::vector<std::string> hackFiles() const override {
    return {
      "ext_blobkv-reader.php",
      "ext_blobkv-writer.php",
    };
  }

  void moduleRegisterNative() override {
    Native::registerNativeDataInfo<BlobKVReader>();
    Native::registerNativeDataInfo<BlobKVWriter>();

    HHVM_NAMED_ME(HH\\BlobKV\\Reader, __construct,
                  HHVM_MN(BlobKVReader, __construct));
    HHVM_NAMED_ME(HH\\BlobKV\\Reader, read,
                  HHVM_MN(BlobKVReader, read));
    HHVM_NAMED_ME(HH\\BlobKV\\Writer, __construct,
                  HHVM_MN(BlobKVWriter, __construct));
    HHVM_NAMED_ME(HH\\BlobKV\\Writer, write,
                  HHVM_MN(BlobKVWriter, write));
    HHVM_NAMED_ME(HH\\BlobKV\\Writer, finalize,
                  HHVM_MN(BlobKVWriter, finalize));
  }
} s_blobkv_extension;

}
