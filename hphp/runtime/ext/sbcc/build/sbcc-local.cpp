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

// SBCC compiler: source files → .sbcc.
//
// Architecture (from D96943800):
//   N compile workers → 1 writer thread → SBCCWriter
//   1 progress thread for observability
//
// Each worker: read source → per-file RepoOptions → mangleUnitSha1 →
//   compile_unit() → BlobEncoder → enqueue for writer.

#include "hphp/runtime/ext/sbcc/build/sbcc-local.h"

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <fstream>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>

#include <unistd.h>

#include <folly/FileUtil.h>
#include <folly/Range.h>

#include "hphp/runtime/base/execution-context.h"
#include "hphp/runtime/base/runtime-option.h"
#include "hphp/runtime/base/program-functions.h"
#include "hphp/runtime/ext/sbcc/format/sbcc-writer.h"
#include "hphp/runtime/base/unit-cache.h"
#include "hphp/runtime/vm/unit-emitter.h"
#include "hphp/runtime/vm/unit-parser.h"

#include "hphp/util/blob-encoder.h"
#include "hphp/util/logger.h"
#include "hphp/util/sha1.h"
#include "hphp/zend/zend-string.h"

namespace HPHP {

namespace {

using Clock = std::chrono::steady_clock;

///////////////////////////////////////////////////////////////////////////////
// Encoded unit ready for the writer thread.

struct EncodedSBCCUnit {
  SHA1 mangledSha1;
  std::vector<char> blob;
};

///////////////////////////////////////////////////////////////////////////////
// Thread-safe write queue with backpressure.

struct SBCCWriteQueue {
  std::mutex mu;
  std::condition_variable cvPush;
  std::condition_variable cvPop;
  std::queue<EncodedSBCCUnit> pending;
  std::atomic<bool> done{false};
  size_t queueBytes{0};
  static constexpr size_t kMaxQueueBytes = 256 * 1024 * 1024; // 256 MB

  void push(EncodedSBCCUnit unit) {
    std::unique_lock lock(mu);
    // Backpressure: block if queue is too large. Also unblock if done
    // (writer thread exited) to avoid permanent deadlock.
    cvPush.wait(lock, [&] {
      return queueBytes < kMaxQueueBytes || done.load(std::memory_order_relaxed);
    });
    if (done.load(std::memory_order_relaxed)) return;
    queueBytes += unit.blob.size();
    pending.push(std::move(unit));
    cvPop.notify_one();
  }

  std::optional<EncodedSBCCUnit> pop() {
    std::unique_lock lock(mu);
    cvPop.wait(lock, [&] { return !pending.empty() || done; });
    if (pending.empty()) return std::nullopt;
    auto entry = std::move(pending.front());
    pending.pop();
    queueBytes -= entry.blob.size();
    cvPush.notify_one();
    return entry;
  }

  void finish() {
    {
      std::lock_guard lock(mu);
      done = true;
    }
    cvPop.notify_all();
  }
};

///////////////////////////////////////////////////////////////////////////////
// Progress tracking.

struct Progress {
  std::atomic<size_t> compiled{0};
  std::atomic<size_t> written{0};
  std::atomic<size_t> errors{0};
  std::atomic<size_t> unexpectedErrors{0};
  std::atomic<size_t> bytesRead{0};
  std::atomic<size_t> bytesStaged{0};
  size_t total{0};
  Clock::time_point start;

  void printInteractive() const {
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
        Clock::now() - start);
    double secs = static_cast<double>(elapsed.count()) / 1000.0;
    auto comp = compiled.load(std::memory_order_relaxed);
    auto writ = written.load(std::memory_order_relaxed);
    auto errs = errors.load(std::memory_order_relaxed);
    auto uerrs = unexpectedErrors.load(std::memory_order_relaxed);
    double rate = secs > 0 ? static_cast<double>(comp) / secs : 0;

    fprintf(stderr,
            "\r\033[K[%5.1fs] compiled %zu/%zu | written %zu | skipped %zu "
            "| unexpected_errors %zu | %.0f files/s",
            secs, comp, total, writ, errs, uerrs, rate);
  }

  void printBatch() const {
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
        Clock::now() - start);
    double secs = static_cast<double>(elapsed.count()) / 1000.0;
    auto comp = compiled.load(std::memory_order_relaxed);
    auto writ = written.load(std::memory_order_relaxed);
    auto errs = errors.load(std::memory_order_relaxed);
    auto uerrs = unexpectedErrors.load(std::memory_order_relaxed);

    Logger::FInfo("[{:.1f}s] compiled {}/{}, written {}, skipped {}, unexpected_errors {}",
                  secs, comp, total, writ, errs, uerrs);
  }
};

///////////////////////////////////////////////////////////////////////////////
// Worker thread: compile one file at a time.

void compileWorker(
    const std::vector<SBCCSourceFile>& files,
    std::atomic<size_t>& nextFile,
    SBCCWriteQueue& writeQueue,
    Progress& progress,
    bool skipMissing) {
  // Compile-only thread init — no session/request init needed.
  // Follows the compiler/package.cpp::parseInit() model:
  // hphp_thread_init(true) + g_context.getCheck(), no hphp_session_init.
  hphp_thread_init(true /* skipExtensions */);
  g_context.getCheck();
  SCOPE_EXIT {
    hphp_thread_exit(true /* skipExtensions */);
  };

  while (true) {
    auto idx = nextFile.fetch_add(1, std::memory_order_relaxed);
    if (idx >= files.size()) break;

    const auto& entry = files[idx];

    // 1. Read source file.
    std::string contents;
    try {
      if (!folly::readFile(entry.absPath.c_str(), contents)) {
        // File doesn't exist or can't be opened.
        if (skipMissing) {
          progress.errors.fetch_add(1, std::memory_order_relaxed);
          progress.compiled.fetch_add(1, std::memory_order_relaxed);
          continue;
        }
        Logger::FError("Cannot read file: {}", entry.absPath);
        progress.unexpectedErrors.fetch_add(1, std::memory_order_relaxed);
        progress.compiled.fetch_add(1, std::memory_order_relaxed);
        continue;
      }
    } catch (const std::exception& e) {
      Logger::FError("Unexpected read error for {}: {}", entry.absPath, e.what());
      progress.unexpectedErrors.fetch_add(1, std::memory_order_relaxed);
      progress.compiled.fetch_add(1, std::memory_order_relaxed);
      continue;
    } catch (...) {
      Logger::FError("Unexpected read error for {}", entry.absPath);
      progress.unexpectedErrors.fetch_add(1, std::memory_order_relaxed);
      progress.compiled.fetch_add(1, std::memory_order_relaxed);
      continue;
    }
    progress.bytesRead.fetch_add(contents.size(), std::memory_order_relaxed);

    // 2. Per-file RepoOptions (critical for hash compatibility).
    auto const& repoOptions = RepoOptions::forFile(entry.absPath.c_str());
    auto const& repoFlags = repoOptions.flags();
    auto const attributes = UnitEmitterAttributes::forAbsolutePath(
      entry.absPath, repoOptions
    );

    // 3. Compute mangled SHA1 (runtime-compatible key).
    auto fileSha1 = string_sha1(folly::StringPiece(contents));
    auto mangled = mangleUnitSha1(
      fileSha1, entry.relativePath, repoFlags, attributes
    );
    SHA1 sha1{mangled};

    // 4. Compile source to UnitEmitter.
    std::unique_ptr<UnitEmitter> ue;
    try {
      ue = compile_unit(
          folly::StringPiece(contents),
          CodeSource::User,
          entry.relativePath.c_str(),
          sha1,
          nullptr,   // extension
          false,     // isSystemLib
          false,     // forDebuggerEval
          repoFlags,
          attributes,
          CompileAbortMode::OnlyICE,
          nullptr    // decl provider
      );
    } catch (const std::exception& e) {
      Logger::FError("Compile error for {}: {}", entry.relativePath, e.what());
      progress.unexpectedErrors.fetch_add(1, std::memory_order_relaxed);
      progress.compiled.fetch_add(1, std::memory_order_relaxed);
      continue;
    } catch (...) {
      Logger::FError("Compile error for {}", entry.relativePath);
      progress.unexpectedErrors.fetch_add(1, std::memory_order_relaxed);
      progress.compiled.fetch_add(1, std::memory_order_relaxed);
      continue;
    }

    // Release source memory before serialization.
    contents.clear();
    contents.shrink_to_fit();

    progress.compiled.fetch_add(1, std::memory_order_relaxed);

    if (!ue) {
      Logger::FError("Compile produced no output for {}", entry.relativePath);
      progress.unexpectedErrors.fetch_add(1, std::memory_order_relaxed);
      continue;
    }

    if (ue->m_ICE || ue->m_fatalUnit) {
      Logger::FInfo("SBCC: skipping fatal unit for {}", entry.relativePath);
      progress.errors.fetch_add(1, std::memory_order_relaxed);
      continue;
    }

    // 5. Serialize UnitEmitter to blob.
    BlobEncoder encoder;
    ue->serde(encoder, false);
    ue.reset(); // release UE memory

    std::vector<char> blob(encoder.size());
    memcpy(blob.data(), encoder.data(), encoder.size());

    progress.bytesStaged.fetch_add(blob.size(), std::memory_order_relaxed);

    // 6. Enqueue for writer thread.
    writeQueue.push(EncodedSBCCUnit{
      sha1,
      std::move(blob)
    });
  }
}

///////////////////////////////////////////////////////////////////////////////
// Writer thread: single-threaded SBCCWriter interaction.

void writerThread(
    SBCCWriter& writer,
    SBCCWriteQueue& writeQueue,
    Progress& progress) {
  while (auto unit = writeQueue.pop()) {
    writer.addUnit(
      unit->mangledSha1,
      unit->blob.data(),
      unit->blob.size());
    progress.written.fetch_add(1, std::memory_order_relaxed);
  }
}

///////////////////////////////////////////////////////////////////////////////
// Progress reporter thread.

void progressThread(const Progress& progress, std::atomic<bool>& stop) {
  bool interactive = isatty(STDERR_FILENO);
  auto lastPrint = Clock::now();

  while (!stop.load(std::memory_order_relaxed)) {
    std::this_thread::sleep_for(std::chrono::milliseconds(
        interactive ? 100 : 5000));

    if (interactive) {
      progress.printInteractive();
    } else {
      auto now = Clock::now();
      auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(
          now - lastPrint);
      if (elapsed.count() >= 5) {
        progress.printBatch();
        lastPrint = now;
      }
    }
  }

  if (interactive) {
    fprintf(stderr, "\r\033[K");
  }
  progress.printBatch();
}

} // anonymous namespace

///////////////////////////////////////////////////////////////////////////////
// Public entry point.

int compileToSBCC(
    const SBCCCompileOptions& options,
    const std::vector<SBCCSourceFile>& files) {
  if (files.empty()) {
    Logger::Error("No files to compile");
    return 1;
  }

  int numThreads = options.maxLocal;
  if (numThreads <= 0) {
    numThreads = std::thread::hardware_concurrency();
    if (numThreads <= 0) numThreads = 1;
  }

  Logger::FInfo("SBCC compile: {} files, {} threads, output={}",
                files.size(), numThreads, options.outputPath);

  // Set up progress tracking.
  Progress progress;
  progress.total = files.size();
  progress.start = Clock::now();

  // Open the streaming SBCC writer.
  SBCCWriter writer(options.outputPath);

  SBCCWriteQueue writeQueue;
  std::atomic<size_t> nextFile{0};

  // Start progress reporter.
  std::atomic<bool> stopProgress{false};
  std::thread progressTh(progressThread,
                         std::cref(progress), std::ref(stopProgress));

  // Start writer thread.
  std::thread writerTh(writerThread,
                       std::ref(writer),
                       std::ref(writeQueue), std::ref(progress));

  // Start compile workers.
  std::vector<std::thread> workers;
  workers.reserve(numThreads);
  for (int i = 0; i < numThreads; i++) {
    workers.emplace_back(compileWorker,
                         std::cref(files),
                         std::ref(nextFile),
                         std::ref(writeQueue),
                         std::ref(progress),
                         options.skipMissing);
  }

  // Join all threads on any exit path (normal or exception) to avoid
  // destroying joinable threads, which calls std::terminate.
  SCOPE_EXIT {
    for (auto& w : workers) {
      if (w.joinable()) w.join();
    }
    writeQueue.finish();
    if (writerTh.joinable()) writerTh.join();
    stopProgress.store(true, std::memory_order_relaxed);
    if (progressTh.joinable()) progressTh.join();
  };

  // Wait for all workers to finish.
  for (auto& w : workers) {
    w.join();
  }

  // Signal writer to drain and finish.
  writeQueue.finish();
  writerTh.join();

  // Stop progress reporter.
  stopProgress.store(true, std::memory_order_relaxed);
  progressTh.join();

  // Finalize the .sbcc file.
  Logger::FInfo("Writing {}...", options.outputPath);
  writer.finish();

  auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
      Clock::now() - progress.start);
  auto totalErrors = progress.errors.load();
  auto totalUnexpected = progress.unexpectedErrors.load();

  Logger::FInfo(
      "Done: {} files compiled, {} written, {} skipped (missing), "
      "{} unexpected errors in {:.1f}s",
      progress.compiled.load(), progress.written.load(),
      totalErrors, totalUnexpected,
      static_cast<double>(elapsed.count()) / 1000.0);

  if (totalUnexpected > 0) {
    Logger::FError(
        "Build failed: {} unexpected error(s) (read/compile/serialization). "
        "Only missing-file skips are allowed via --skip-missing.",
        totalUnexpected);
    return 1;
  }

  return 0;
}

} // namespace HPHP
