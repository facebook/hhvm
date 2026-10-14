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

#include "hphp/runtime/vm/jit/cprof-checkpoint.h"

#include <algorithm>
#include <atomic>
#include <chrono>
#include <condition_variable>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <exception>
#include <filesystem>
#include <mutex>
#include <string>
#include <sys/stat.h>
#include <system_error>
#include <thread>
#include <tuple>
#include <type_traits>
#include <unistd.h>
#include <utility>
#include <vector>

#include <fmt/format.h>
#include <folly/FileUtil.h>
#include <folly/Range.h>
#include <folly/ScopeGuard.h>
#include <folly/system/ThreadName.h>

#include "hphp/runtime/base/init-fini-node.h"
#include "hphp/runtime/base/program-functions.h"
#include "hphp/runtime/vm/jit/cprof-controller.h"
#include "hphp/runtime/vm/jit/cprof-serde.h"
#include "hphp/runtime/vm/jit/prof-data.h"
#include "hphp/runtime/vm/treadmill.h"
#include "hphp/util/configs/jit.h"
#include "hphp/util/configs/server.h"
#include "hphp/util/logger.h"
#include "hphp/util/service-data.h"

namespace HPHP::jit::cprof {

bool isContProfCheckpointFilename(std::string_view filename) {
  constexpr std::string_view prefix{"cont-prof-"};
  constexpr std::string_view suffix{".cprof"};
  if (!filename.starts_with(prefix) || !filename.ends_with(suffix)) return false;

  filename.remove_prefix(prefix.size());
  filename.remove_suffix(suffix.size());
  auto const separator = filename.find('-');
  if (separator == std::string_view::npos) return false;

  auto const isDecimal = [](std::string_view value) {
    return !value.empty() && std::all_of(value.begin(), value.end(), [](char c) {
      return c >= '0' && c <= '9';
    });
  };
  return isDecimal(filename.substr(0, separator)) &&
    isDecimal(filename.substr(separator + 1));
}

namespace {

using EncodedRecordCount = uint32_t;
using EncodedRecordSize = uint32_t;

constexpr size_t kMaxCheckpointBytes = 256U << 20;
constexpr EncodedRecordCount kMaxCheckpointRecords = 100'000;

auto const s_checkpointRecords =
  ServiceData::createCounter("jit.cprof.checkpoint.records");
auto const s_checkpointBytes =
  ServiceData::createCounter("jit.cprof.checkpoint.bytes");
auto const s_checkpointWriteFailures =
  ServiceData::createCounter("jit.cprof.checkpoint.write_failures");

// Start attempts include eligibility exclusions, but exclude calls with an
// already-joinable writer. Failures count invalid directories, thread creation
// failures, and worker failures before the Starting -> Running transition.
auto const s_writerStartAttempts =
  ServiceData::createCounter("jit.cprof.writer.start_attempts");
auto const s_writerStartFailures =
  ServiceData::createCounter("jit.cprof.writer.start_failures");

// Count every scheduled/final flush, including skips. Empty skips mean no
// captured records, not an unchanged nonempty checkpoint.
auto const s_writerCheckpointAttempts =
  ServiceData::createCounter("jit.cprof.writer.checkpoint_attempts");
auto const s_writerEmptyCheckpointSkips =
  ServiceData::createCounter("jit.cprof.writer.empty_checkpoint_skips");

// Eligibility reasons are first-match exclusions counted only by start(),
// never by the per-function contProfActive() checks.
auto const s_writerNotServerMode =
  ServiceData::createCounter("jit.cprof.writer.not_server_mode");
auto const s_writerCaptureDisabled =
  ServiceData::createCounter("jit.cprof.writer.capture_disabled");
auto const s_writerZeroInterval =
  ServiceData::createCounter("jit.cprof.writer.zero_interval");
auto const s_writerDeserializedProfData =
  ServiceData::createCounter("jit.cprof.writer.deserialized_prof_data");

// These identify actual startup failures or an exception escaping the worker's
// main loop, not recoverable errors within a checkpoint attempt.
auto const s_writerInvalidDirectory =
  ServiceData::createCounter("jit.cprof.writer.invalid_directory");
auto const s_writerThreadStartFailure =
  ServiceData::createCounter("jit.cprof.writer.thread_start_failure");
auto const s_writerUnexpectedStop =
  ServiceData::createCounter("jit.cprof.writer.unexpected_stop");

struct Writer {
  explicit Writer(size_t maxBytes) : m_maxBytes{maxBytes} {}

  template<class T>
  bool writeValue(T value) {
    static_assert(std::is_unsigned_v<T>);
    return writeBytes(reinterpret_cast<const uint8_t*>(&value), sizeof(value));
  }

  bool writeBytes(const uint8_t* data, size_t size) {
    if (size > m_maxBytes - m_bytes.size()) return false;
    if (size != 0) {
      m_bytes.insert(m_bytes.end(), data, data + size);
    }
    return true;
  }

  template<class T>
  bool replaceValue(size_t offset, T value) {
    static_assert(std::is_unsigned_v<T>);
    if (offset > m_bytes.size() ||
        sizeof(value) > m_bytes.size() - offset) {
      return false;
    }

    std::memcpy(m_bytes.data() + offset, &value, sizeof(value));
    return true;
  }

  size_t remaining() const { return m_maxBytes - m_bytes.size(); }

  std::vector<uint8_t> takeBytes() && { return std::move(m_bytes); }

private:
  size_t m_maxBytes;
  std::vector<uint8_t> m_bytes;
};

struct Reader {
  explicit Reader(folly::ByteRange bytes)
    : m_data{bytes.data()}
    , m_remaining{bytes.size()} {}

  template<class T>
  bool readValue(T& value) {
    static_assert(std::is_unsigned_v<T>);
    return readBytes(reinterpret_cast<uint8_t*>(&value), sizeof(value));
  }

  bool readBytes(uint8_t* output, size_t size) {
    if (size > m_remaining) return false;

    if (size != 0) {
      std::memcpy(output, m_data, size);
      m_data += size;
      m_remaining -= size;
    }

    return true;
  }

  std::optional<folly::ByteRange> readRange(size_t size) {
    if (size > m_remaining) return std::nullopt;

    auto const result = folly::ByteRange{m_data, size};
    m_data += size;
    m_remaining -= size;
    return result;
  }

  size_t remaining() const { return m_remaining; }

private:
  const uint8_t* m_data;
  size_t m_remaining;
};

struct EncodedCheckpoint {
  std::vector<uint8_t> bytes;
  EncodedRecordCount recordCount;
};

std::optional<EncodedCheckpoint>
encodeContProfCheckpoint(const std::vector<ContProfProfileRecord>& records) {
  // TODO(jtwarren): add support for selecting hottest and truncating the rest.
  if (records.empty() || records.size() > kMaxCheckpointRecords) {
    return std::nullopt;
  }

  Writer writer{kMaxCheckpointBytes};

  if (!writer.writeValue(EncodedRecordCount{0})) return std::nullopt;

  EncodedRecordCount recordCount{};
  for (size_t i = 0; i < records.size(); ++i) {
    if (i != 0 &&
        !(records[i - 1].header.funcKey < records[i].header.funcKey)) {
      return std::nullopt;
    }

    auto const encoded = serializeContProfProfileRecord(records[i]);
    if (!encoded || sizeof(EncodedRecordSize) > writer.remaining() ||
        encoded->size() > writer.remaining() - sizeof(EncodedRecordSize)) {
      continue;
    }

    if (!writer.writeValue(static_cast<EncodedRecordSize>(encoded->size())) ||
        !writer.writeBytes(encoded->data(), encoded->size())) {
      return std::nullopt;
    }

    ++recordCount;
  }

  if (recordCount == 0 ||
      !writer.replaceValue(0, recordCount)) {
    return std::nullopt;
  }

  return EncodedCheckpoint{std::move(writer).takeBytes(), recordCount};
}

}

std::optional<std::vector<uint8_t>>
serializeContProfCheckpoint(const std::vector<ContProfProfileRecord>& records) {
  auto encoded = encodeContProfCheckpoint(records);
  if (!encoded) return std::nullopt;
  return std::move(encoded->bytes);
}

std::optional<std::vector<ContProfProfileRecord>>
deserializeContProfCheckpoint(folly::ByteRange encoded) {
  if (encoded.size() < sizeof(EncodedRecordCount) ||
      encoded.size() > kMaxCheckpointBytes) {
    return std::nullopt;
  }

  Reader reader{encoded};

  EncodedRecordCount recordCount{};
  if (!reader.readValue(recordCount) || recordCount == 0 ||
      recordCount > kMaxCheckpointRecords) {
    return std::nullopt;
  }

  std::vector<ContProfProfileRecord> records;
  records.reserve(recordCount);

  for (EncodedRecordCount i = 0; i < recordCount; ++i) {
    EncodedRecordSize recordSize{};
    if (!reader.readValue(recordSize) || recordSize == 0) return std::nullopt;

    auto const frame = reader.readRange(recordSize);
    if (!frame) return std::nullopt;

    auto record = deserializeContProfProfileRecord(*frame);
    if (!record) return std::nullopt;

    if (!records.empty() &&
        !(records.back().header.funcKey < record->header.funcKey)) {
      return std::nullopt;
    }

    records.push_back(std::move(*record));
  }

  if (reader.remaining() != 0) return std::nullopt;
  return records;
}

bool writeContProfCheckpointFile(
  const std::string& path,
  const std::vector<ContProfProfileRecord>& records
) {
  auto onFailure = folly::makeGuard([] {
    s_checkpointWriteFailures->increment();
  });
  if (path.empty()) return false;

  auto const encoded = encodeContProfCheckpoint(records);
  if (!encoded) return false;

  auto const& bytes = encoded->bytes;
  auto const contents = folly::StringPiece{
    reinterpret_cast<const char*>(bytes.data()),
    bytes.size()
  };

  if (folly::writeFileAtomicNoThrow(
    path,
    contents,
    folly::WriteFileAtomicOptions{}
  ) != 0) return false;

  s_checkpointRecords->setValue(encoded->recordCount);
  s_checkpointBytes->setValue(bytes.size());
  onFailure.dismiss();
  return true;
}

bool pruneContProfCheckpointFiles(const std::string& currentPath) {
  auto const current = std::filesystem::path{currentPath};
  auto const directory = current.parent_path();

  struct CheckpointFile {
    std::filesystem::path path;
    std::filesystem::file_time_type mtime;
  };
  std::vector<CheckpointFile> files;
  bool foundCurrent = false;
  std::error_code error;
  std::filesystem::directory_iterator iterator{directory, error};
  auto const end = std::filesystem::directory_iterator{};
  for (; !error && iterator != end; iterator.increment(error)) {
    auto const& entry = *iterator;
    auto const filename = entry.path().filename();
    if (!isContProfCheckpointFilename(filename.native())) continue;

    std::error_code entryError;
    auto const status = entry.symlink_status(entryError);
    if (entryError || !std::filesystem::is_regular_file(status)) continue;

    if (filename == current.filename()) {
      foundCurrent = true;
      continue;
    }
    auto const mtime = entry.last_write_time(entryError);
    if (!entryError) files.push_back({entry.path(), mtime});
  }
  if (error || !foundCurrent) return false;

  std::sort(files.begin(), files.end(), [](auto const& lhs, auto const& rhs) {
    return std::tie(lhs.mtime, lhs.path) > std::tie(rhs.mtime, rhs.path);
  });

  bool success = true;
  for (size_t i = kMaxContProfCheckpointFiles - 1; i < files.size(); ++i) {
    auto const& file = files[i];
    auto const status = std::filesystem::symlink_status(file.path, error);
    if (error || !std::filesystem::is_regular_file(status)) continue;

    // Another writer may have replaced the file since this scan.
    auto const mtime = std::filesystem::last_write_time(file.path, error);
    if (error || mtime != file.mtime) continue;

    std::filesystem::remove(file.path, error);
    if (error) success = false;
  }
  return success;
}

std::optional<std::vector<ContProfProfileRecord>>
readContProfCheckpointFile(const std::string& path) {
  if (path.empty()) return std::nullopt;

  std::vector<uint8_t> encoded;
  if (!folly::readFile(path.c_str(), encoded, kMaxCheckpointBytes + 1) ||
      encoded.size() > kMaxCheckpointBytes) {
    return std::nullopt;
  }

  return deserializeContProfCheckpoint(
    folly::ByteRange{encoded.data(), encoded.size()}
  );
}

namespace {

std::string makeContProfCheckpointPath() {
  auto const& directory = Cfg::Jit::ContProfCheckpointDirectory;
  if (directory.empty()) return {};

  struct stat info{};
  if (::stat(directory.c_str(), &info) != 0 || !S_ISDIR(info.st_mode)) {
    Logger::Warning(
      "Invalid cont-prof checkpoint directory: %s",
      directory.c_str()
    );
    return {};
  }

  auto const now =
    std::chrono::duration_cast<std::chrono::microseconds>(
      std::chrono::system_clock::now().time_since_epoch()
    ).count();

  auto const separator = directory.back() == '/' ? "" : "/";

  return fmt::format(
    "{}{}cont-prof-{}-{}.cprof",
    directory,
    separator,
    static_cast<int64_t>(::getpid()),
    now
  );
}

enum class CaptureEligibility {
  Eligible,
  NotServerMode,
  CaptureDisabled,
  ZeroInterval,
  DeserializedProfData,
};

CaptureEligibility contProfCaptureEligibility() {
  if (!Cfg::Server::Mode) return CaptureEligibility::NotServerMode;
  if (!Cfg::Jit::ContProfCaptureEnabled) {
    return CaptureEligibility::CaptureDisabled;
  }
  if (Cfg::Jit::ContProfCheckpointIntervalSeconds == 0) {
    return CaptureEligibility::ZeroInterval;
  }
  if (ProfData::wasDeserialized()) {
    return CaptureEligibility::DeserializedProfData;
  }
  return CaptureEligibility::Eligible;
}

struct ContProfCheckpointWriter {
  bool running() const {
    return m_state.load(std::memory_order_acquire) == State::Running;
  }

  void start() {
    if (m_thread.joinable()) return;

    s_writerStartAttempts->increment();
    switch (contProfCaptureEligibility()) {
      case CaptureEligibility::NotServerMode:
        s_writerNotServerMode->increment();
        return;
      case CaptureEligibility::CaptureDisabled:
        s_writerCaptureDisabled->increment();
        return;
      case CaptureEligibility::ZeroInterval:
        s_writerZeroInterval->increment();
        return;
      case CaptureEligibility::DeserializedProfData:
        s_writerDeserializedProfData->increment();
        return;
      case CaptureEligibility::Eligible:
        break;
    }

    m_path = makeContProfCheckpointPath();
    if (m_path.empty()) {
      s_writerInvalidDirectory->increment();
      s_writerStartFailures->increment();
      return;
    }

    m_state.store(State::Starting, std::memory_order_release);
    try {
      m_thread = std::thread{[this] { run(); }};
    } catch (const std::exception& exception) {
      m_state.store(State::Stopped, std::memory_order_release);
      s_writerThreadStartFailure->increment();
      s_writerStartFailures->increment();
      Logger::Warning(
        "Failed to start cont-prof checkpoint writer: %s",
        exception.what()
      );
      m_path.clear();
      return;
    }

    auto expected = State::Starting;
    m_state.compare_exchange_strong(
      expected,
      State::Running,
      std::memory_order_acq_rel,
      std::memory_order_acquire
    );
  }

  void stop() {
    if (!m_thread.joinable()) return;

    m_state.store(State::Stopped, std::memory_order_release);

    {
      std::lock_guard<std::mutex> lock{m_shutdownMutex};
      m_stopping = true;
    }

    // Wake the writer instead of waiting out the checkpoint interval.
    m_shutdownCondition.notify_one();
    m_thread.join();
  }

private:
  enum class State : uint8_t {
    Stopped,
    Starting,
    Running,
  };

  void writeCheckpoint() noexcept {
    s_writerCheckpointAttempts->increment();
    try {
      auto const recordCount = numContProfProfileRecords();
      if (recordCount == 0) {
        s_writerEmptyCheckpointSkips->increment();
        return;
      }

      // Records are immutable after their first snapshot, so an unchanged
      // count means there is nothing new to write.
      if (recordCount == m_lastWrittenRecordCount) return;

      auto const records = [] {
        Treadmill::Session session{Treadmill::SessionKind::ProfData};
        return snapshotContProfProfileRecords();
      }();

      if (!writeContProfCheckpointFile(m_path, records)) {
        Logger::Warning(
          "Failed to write cont-prof checkpoint: %s",
          m_path.c_str()
        );
        return;
      }

      m_lastWrittenRecordCount = records.size();
      if (!pruneContProfCheckpointFiles(m_path)) {
        Logger::Warning(
          "Failed to prune cont-prof checkpoints: %s",
          Cfg::Jit::ContProfCheckpointDirectory.c_str()
        );
      }
    } catch (const std::exception& exception) {
      Logger::Warning(
        "Failed to build cont-prof checkpoint: %s",
        exception.what()
      );
    } catch (...) {
      Logger::Warning("Failed to build cont-prof checkpoint");
    }
  }

  void run() noexcept {
    bool failed = false;
    try {
      hphp_thread_init(true /* skipExtensions */);
      SCOPE_EXIT { hphp_thread_exit(true /* skipExtensions */); };

      folly::setThreadName("cont-prof");

      auto const interval = std::chrono::seconds{
        Cfg::Jit::ContProfCheckpointIntervalSeconds
      };

      std::unique_lock<std::mutex> lock{m_shutdownMutex};

      while (!m_shutdownCondition.wait_for(
        lock,
        interval,
        [this] { return m_stopping; }
      )) {
        lock.unlock();
        writeCheckpoint();
        lock.lock();
      }

      lock.unlock();
      writeCheckpoint();
    } catch (const std::exception& exception) {
      failed = true;
      Logger::Warning(
        "Cont-prof checkpoint writer stopped unexpectedly: %s",
        exception.what()
      );
    } catch (...) {
      failed = true;
      Logger::Warning("Cont-prof checkpoint writer stopped unexpectedly");
    }

    auto const previous = m_state.exchange(
      State::Stopped, std::memory_order_acq_rel
    );
    if (failed) {
      s_writerUnexpectedStop->increment();
      if (previous == State::Starting) s_writerStartFailures->increment();
    }
  }

  std::string m_path;
  std::thread m_thread;
  std::mutex m_shutdownMutex;
  std::condition_variable m_shutdownCondition;
  std::atomic<State> m_state{State::Stopped};
  size_t m_lastWrittenRecordCount{0};
  bool m_stopping{false};
};

ContProfCheckpointWriter& contProfCheckpointWriter() {
  static auto const writer = new ContProfCheckpointWriter;
  return *writer;
}

// Read the actual writer state when exported. Updating a separate gauge from
// start() could overwrite the worker's zero after an early worker exit.
ServiceData::CounterCallback s_writerActive{
  [](ServiceData::CounterMap& counters) {
    counters.emplace(
      "jit.cprof.writer.active", contProfCheckpointWriter().running()
    );
  },
  "jit.cprof.writer.active"
};

}

bool contProfActive() {
  return
    contProfCaptureEligibility() == CaptureEligibility::Eligible &&
    contProfCheckpointWriter().running();
}

void startContProfCheckpointWriter() { contProfCheckpointWriter().start(); }

void stopContProfCheckpointWriter() { contProfCheckpointWriter().stop(); }

namespace {

InitFiniNode s_contProfCheckpointInit{
  startContProfCheckpointWriter,
  InitFiniNode::When::ProcessInit,
  "cont-prof checkpoint writer"
};

InitFiniNode s_contProfCheckpointFini{
  stopContProfCheckpointWriter,
  InitFiniNode::When::ProcessExit,
  "cont-prof checkpoint writer"
};

}

}
