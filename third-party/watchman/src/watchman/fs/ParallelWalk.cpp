/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "watchman/fs/ParallelWalk.h"
#include <fmt/core.h>
#include <folly/concurrency/UnboundedQueue.h>
#include <folly/executors/CPUThreadPoolExecutor.h>
#include <folly/system/HardwareConcurrency.h>

namespace watchman {

template <typename T>
using Queue = folly::UMPMCQueue<std::optional<T>, true /* MayBlock */>;

struct ParallelWalkerContext {
  // Input.
  std::shared_ptr<FileSystem> fileSystem;
  folly::Executor* executor;
  std::optional<FileInformation> rootStat;

  // Task tracking. Other task states live in the Executor.
  std::atomic<size_t> readDirTaskCount{0};
  std::atomic<bool> stopped{false};

  // Output.
  Queue<ReadDirResult> resultQueue{};
  Queue<IoErrorWithPath> errorQueue{};

  ParallelWalkerContext(
      std::shared_ptr<FileSystem> fileSystem,
      folly::Executor* executor,
      std::optional<FileInformation> rootStat)
      : fileSystem{std::move(fileSystem)},
        executor{executor},
        rootStat{rootStat} {}

  // Helper for (resultQueue or errorQueue).dequeue.
  // If no tasks are running, return nullopt.
  template <typename T>
  std::optional<T> taskAwareDequeue(Queue<T>& queue) {
    // Try non-blocking deque first.
    auto maybe = queue.try_dequeue();
    if (maybe) {
      return std::move(maybe).value();
    }
    if (readDirTaskCount.load(std::memory_order_acquire) == 0) {
      return std::nullopt;
    }
    // readDirTaskCount > 0. ~ReadDirTaskCounter will push nullopt to queue.
    // So this will not block forever.
    auto value = queue.dequeue();
    return value;
  }
};

namespace {

folly::Executor::KeepAlive<> createExecutor(size_t threadCountHint) {
  size_t hwThreadCount = folly::hardware_concurrency();
  size_t threadCount = threadCountHint
      ? std::min(threadCountHint, hwThreadCount)
      : hwThreadCount;
  return new folly::CPUThreadPoolExecutor(
      threadCount, std::make_unique<folly::NamedThreadFactory>("pwalk"));
}

// Executor used by the readDir tasks. The executor is global to avoid spawning
// too many threads walking multiple roots.
folly::Executor* getExecutor(size_t threadCountHint) {
  static folly::Executor::KeepAlive<> e = createExecutor(threadCountHint);
  return e.get();
}

folly::fbstring pathJoin(
    const folly::fbstring& dirName,
    const folly::fbstring& baseName) {
  if (dirName.empty()) {
    return baseName;
  }
  if (baseName.empty()) {
    return dirName;
  }
  return fmt::format("{}/{}", dirName, baseName);
}

// Update readDirTaskCount. +1 on construction. -1 on destruction.
class ReadDirTaskCounter {
 public:
  ~ReadDirTaskCounter() {
    if (!context_) {
      return;
    }
    size_t count =
        context_->readDirTaskCount.fetch_sub(1, std::memory_order_acq_rel);
    if (count == 1) {
      // Last task ends. Mark queues as "ended".
      context_->resultQueue.enqueue(std::nullopt);
      context_->errorQueue.enqueue(std::nullopt);
    }
  }

  explicit ReadDirTaskCounter(std::shared_ptr<ParallelWalkerContext> context)
      : context_(std::move(context)) {
    context_->readDirTaskCount.fetch_add(1, std::memory_order_relaxed);
  }

  ReadDirTaskCounter(const ReadDirTaskCounter& rhs) {
    context_ = rhs.context_;
    context_->readDirTaskCount.fetch_add(1, std::memory_order_relaxed);
  }

  ReadDirTaskCounter() = delete;
#if defined(_MSC_VER)
  // MSVC (tested with cl 19.32.31332 from VS 2022) does not do
  // copy/move-elision for lambda capture.
  ReadDirTaskCounter(ReadDirTaskCounter&&) = default;
#else
  ReadDirTaskCounter(ReadDirTaskCounter&&) = delete;
#endif
  ReadDirTaskCounter& operator=(const ReadDirTaskCounter&) = delete;
  ReadDirTaskCounter& operator=(ReadDirTaskCounter&&) = delete;

 private:
  std::shared_ptr<ParallelWalkerContext> context_;
};

/**
 * Approximate bytes per entry for a directory. Used to provide a hint
 * to reserve ReadDirResult::entries.
 *
 * 32 is picked by running the following Python script on a mac:
 *
 *     import os, glob
 *     def size_per_entry(d):
 *         try: return os.stat(d).st_size / len(os.listdir(d))
 *         except Exception: return None
 *     print(min(filter(None, map(size_per_entry, glob.glob('/''*''/''*')))))
 *
 * Therefore it is optimized for APFS.
 * Other filesystems probably have different optimal values.
 */
const size_t kApproximateSizePerEntry = 32;

/**
 * Read and stat dirPath's direct children.
 * Push ReadDirResult to context->resultQueue.
 * Push errors to context->errorQueue.
 * Spawn readDirTask for subdirectories.
 */
void readDirTask(
    std::shared_ptr<ParallelWalkerContext> context,
    AbsolutePath dirFullPath,
    ReadDirTaskCounter&& counter,
    size_t dirSizeHint = 0) {
  if (context->stopped.load(std::memory_order_acquire)) {
    return;
  }

  std::unique_ptr<DirHandle> dir;
  try {
    dir = context->fileSystem->openDir(dirFullPath.c_str());
  } catch (const std::system_error& err) {
    IoErrorWithPath error{std::move(dirFullPath), err, "opendir"};
    context->errorQueue.enqueue(error);
    return;
  }

  // openDir() returns nullptr. It is used to ignore a directory.
  if (!dir) {
    return;
  }

  std::vector<DirEntryOwned> entries;
  entries.reserve(dirSizeHint);

  size_t subdirCount = 0;
  while (const DirEntry* dirent = dir->readDir()) {
    // Skip . and ..
    const char* d_name = dirent->d_name;
    if (d_name[0] == '.' &&
        (d_name[1] == 0 || (d_name[1] == '.' && d_name[2] == 0))) {
      continue;
    }
    // Get stat() information.
    PathComponent name(d_name);
    FileInformation st;
    if (dirent->has_stat) {
      st = dirent->stat;
    } else {
      auto fileFullPath = pathJoin(dirFullPath, name);
      try {
        st = context->fileSystem->getFileInformation(fileFullPath.c_str());
      } catch (const std::system_error& err) {
        IoErrorWithPath error{
            std::move(fileFullPath), err, "getFileInformation"};
        context->errorQueue.enqueue(error);
        // Contine checking other entries.
        continue;
      }
    }
    if (st.isDir()) {
      subdirCount += 1;
    }
    DirEntryOwned entry{
        std::move(name),
        st,
    };
    entries.push_back(entry);
  }

  // Figure out subdirs to read before losing ownership of entries.
  std::vector<std::pair<AbsolutePath, size_t>> subdirsToRead;
  subdirsToRead.reserve(subdirCount);
  for (const auto& entry : entries) {
    if (entry.stat.isDir()) {
      AbsolutePath subdirPath = pathJoin(dirFullPath, entry.name);
      if (!context->rootStat ||
          isOnSameMount(*context->rootStat, entry.stat, subdirPath.c_str())) {
        size_t sizeHint = entry.stat.size / kApproximateSizePerEntry;
        subdirsToRead.push_back(std::make_pair(subdirPath, sizeHint));
      }
    }
  }

  // Enqueue ReadDirResult before reading subdirs.
  ReadDirResult result{std::move(dirFullPath), std::move(entries), subdirCount};
  context->resultQueue.enqueue(result);

  // Spawn tasks to read subdirs.
  for (auto& pair : subdirsToRead) {
    auto& path = pair.first;
    auto sizeHint = pair.second;
    auto task = [context = context,
                 path = std::move(path),
                 counter = counter,
                 sizeHint = sizeHint]() mutable {
      readDirTask(
          std::move(context), std::move(path), std::move(counter), sizeHint);
    };
    context->executor->add(std::move(task));
  }
}

} // namespace

ParallelWalker::ParallelWalker(
    std::shared_ptr<FileSystem> fileSystem,
    AbsolutePath rootPath,
    std::optional<FileInformation> rootStat,
    size_t threadCountHint) {
  auto executor = getExecutor(threadCountHint);
  context_ = std::make_shared<ParallelWalkerContext>(
      std::move(fileSystem), executor, rootStat);
  auto task = [context = context_,
               path = std::move(rootPath),
               counter = ReadDirTaskCounter(context_)]() mutable {
    readDirTask(std::move(context), std::move(path), std::move(counter));
  };
  context_->executor->add(std::move(task));
}

ParallelWalker::~ParallelWalker() {
  context_->stopped.store(true, std::memory_order_release);
}

std::optional<ReadDirResult> ParallelWalker::nextResult() {
  return context_->taskAwareDequeue(context_->resultQueue);
}

std::optional<IoErrorWithPath> ParallelWalker::nextError() {
  return context_->taskAwareDequeue(context_->errorQueue);
}

} // namespace watchman
