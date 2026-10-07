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

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <exception>
#include <mutex>
#include <string>
#include <sys/stat.h>
#include <thread>
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

namespace HPHP::jit::cprof {

namespace {

using EncodedRecordCount = uint32_t;
using EncodedRecordSize = uint32_t;

constexpr size_t kMaxCheckpointBytes = 256U << 20;
constexpr EncodedRecordCount kMaxCheckpointRecords = 100'000;

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

}

std::optional<std::vector<uint8_t>>
serializeContProfCheckpoint(const std::vector<ContProfProfileRecord>& records) {
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

  return std::move(writer).takeBytes();
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
  if (path.empty()) return false;

  auto const encoded = serializeContProfCheckpoint(records);
  if (!encoded) return false;

  auto const contents = folly::StringPiece{
    reinterpret_cast<const char*>(encoded->data()),
    encoded->size()
  };

  return folly::writeFileAtomicNoThrow(
    path,
    contents,
    folly::WriteFileAtomicOptions{}
  ) == 0;
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

bool contProfCaptureEligible() {
  return
    Cfg::Server::Mode &&
    Cfg::Jit::ContProfCaptureEnabled &&
    Cfg::Jit::ContProfCheckpointIntervalSeconds != 0 &&
    !ProfData::wasDeserialized();
}

struct ContProfCheckpointWriter {
  bool running() const {
    return m_state.load(std::memory_order_acquire) == State::Running;
  }

  void start() {
    if (!contProfCaptureEligible() || m_thread.joinable()) return;

    m_path = makeContProfCheckpointPath();
    if (m_path.empty()) return;

    m_state.store(State::Starting, std::memory_order_release);
    try {
      m_thread = std::thread{[this] { run(); }};
    } catch (const std::exception& exception) {
      m_state.store(State::Stopped, std::memory_order_release);
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
    try {
      // Records are immutable after their first snapshot, so an unchanged
      // count means there is nothing new to write.
      if (numContProfProfileRecords() == m_lastWrittenRecordCount) return;

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
      Logger::Warning(
        "Cont-prof checkpoint writer stopped unexpectedly: %s",
        exception.what()
      );
    } catch (...) {
      Logger::Warning("Cont-prof checkpoint writer stopped unexpectedly");
    }

    m_state.store(State::Stopped, std::memory_order_release);
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

}

bool contProfActive() {
  return
    contProfCaptureEligible() &&
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
