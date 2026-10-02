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

#include "hphp/runtime/vm/jit/cprof-reader.h"

#include <algorithm>
#include <cstdint>
#include <filesystem>
#include <map>
#include <string>
#include <string_view>
#include <system_error>
#include <tuple>
#include <utility>
#include <vector>

#include "hphp/runtime/vm/func.h"
#include "hphp/runtime/vm/jit/cprof-checkpoint.h"
#include "hphp/runtime/vm/srckey.h"

namespace HPHP::jit::cprof {

namespace {

constexpr size_t kMaxDirectoryEntries = 1000;
constexpr size_t kMaxCheckpointFiles = 20;
constexpr size_t kMaxDecodedRecords = 200'000;

struct CheckpointFile {
  std::string filename;
  std::string path;
  std::filesystem::file_time_type mtime{};
};

struct SelectedRecord {
  ContProfProfileRecord record{};
  std::string filename;
};

bool isCheckpointFilename(std::string_view filename) {
  constexpr std::string_view kPrefix{"cont-prof-"};
  constexpr std::string_view kSuffix{".cprof"};

  return filename.starts_with(kPrefix) && filename.ends_with(kSuffix);
}

bool isBetterCandidate(
  const ContProfProfileRecord& candidate,
  const std::string& candidateFilename,
  const SelectedRecord& current
) {
  auto const candidateExecutions = candidate.functionExecutions();
  auto const currentExecutions = current.record.functionExecutions();

  // The filename makes otherwise identical records deterministic.
  return std::tie(
    candidate.header.capturedAtMs,
    candidateExecutions,
    candidateFilename
  ) > std::tie(
    current.record.header.capturedAtMs,
    currentExecutions,
    current.filename
  );
}

std::optional<std::vector<CheckpointFile>>
discoverCheckpointFiles(const std::string& directory) {
  if (directory.empty()) return std::vector<CheckpointFile>{};

  std::error_code iterationError;
  std::filesystem::directory_iterator iterator{directory, iterationError};
  if (iterationError) return std::nullopt;

  std::vector<CheckpointFile> files;
  size_t entriesSeen{};

  auto const end = std::filesystem::directory_iterator{};
  for (;
       !iterationError && iterator != end;
       iterator.increment(iterationError)) {
    if (++entriesSeen > kMaxDirectoryEntries) return std::nullopt;

    auto const& entry = *iterator;
    auto const filename = entry.path().filename().native();
    if (!isCheckpointFilename(filename)) continue;

    std::error_code entryError;
    auto const status = entry.symlink_status(entryError);
    if (entryError || !std::filesystem::is_regular_file(status)) continue;

    auto const mtime = entry.last_write_time(entryError);
    if (entryError) continue;

    files.push_back({
      filename,
      entry.path().native(),
      mtime,
    });
  }
  if (iterationError) return std::nullopt;

  // Limit startup work to the newest checkpoints.
  std::sort(
    files.begin(),
    files.end(),
    [](const CheckpointFile& lhs, const CheckpointFile& rhs) {
      return std::tie(lhs.mtime, lhs.filename) >
        std::tie(rhs.mtime, rhs.filename);
    }
  );
  if (files.size() > kMaxCheckpointFiles) files.resize(kMaxCheckpointFiles);

  return files;
}

}

std::optional<SrcKey> contProfTranslationSrcKey(
  const ContProfProfileTranslation& translation,
  const Func& func
) {
  switch (translation.startKind) {
    case ContProfStartKind::FuncEntry: {
      if (translation.numEntryArgs > func.numPositionalParams()) {
        return std::nullopt;
      }

      return SrcKey{
        &func,
        translation.numEntryArgs,
        false,
        SrcKey::FuncEntryTag{},
      };
    }

    case ContProfStartKind::NamedParamsFuncEntry: {
      if (!func.hasOptionalNamedParameters()) {
        return std::nullopt;
      }

      return SrcKey{
        &func,
        func.numPositionalParams(),
        true,
        SrcKey::FuncEntryTag{},
      };
    }
  }

  return std::nullopt;
}

std::optional<ContProfCheckpointReadResult>
readContProfCheckpointDirectory(const std::string& directory) {
  auto const files = discoverCheckpointFiles(directory);
  if (!files) return std::nullopt;

  size_t decodedRecords{};
  size_t filesRead{};
  std::map<ContProfFuncKey, SelectedRecord> selected;

  for (auto const& file : *files) {
    auto records = readContProfCheckpointFile(file.path);
    if (!records) continue;

    if (records->size() > kMaxDecodedRecords - decodedRecords) {
      break;
    }

    ++filesRead;
    decodedRecords += records->size();

    for (auto& record : *records) {
      auto const [current, inserted] =
        selected.try_emplace(record.header.funcKey);
      if (inserted ||
          isBetterCandidate(record, file.filename, current->second)) {
        current->second = SelectedRecord{std::move(record), file.filename};
      }
    }
  }

  ContProfCheckpointReadResult result{};
  result.filesRead = filesRead;
  result.recordsDecoded = decodedRecords;
  result.records.reserve(selected.size());

  for (auto& entry : selected) {
    result.records.push_back(std::move(entry.second.record));
  }

  return result;
}

bool isContProfProfileRecordCompatible(
  const ContProfProfileRecord& record,
  const Func& func
) {
  if (!isValidContProfProfileRecord(record)) return false;

  auto const currentKey = makeContProfFuncKey(func);
  if (!currentKey || *currentKey != record.header.funcKey) return false;

  for (auto const& translation : record.translations) {
    if (!contProfTranslationSrcKey(translation, func)) return false;
  }

  return true;
}

}
