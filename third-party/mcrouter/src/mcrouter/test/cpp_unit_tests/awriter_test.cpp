/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <condition_variable>
#include <memory>
#include <mutex>

#include <gtest/gtest.h>

#include <folly/File.h>
#include <folly/FileUtil.h>
#include <folly/Range.h>
#include <folly/experimental/TestUtil.h>

#include "mcrouter/AsyncWriter.h"
#include "mcrouter/AsyncWriterEntry.h"
#include "mcrouter/Proxy.h"

using namespace facebook::memcache::mcrouter;

using folly::test::TemporaryFile;

namespace {
constexpr folly::StringPiece kTestMessage = "abc\n";

class AtomicCounter {
 public:
  void notify(size_t v) {
    {
      std::lock_guard<std::mutex> lg(lock_);
      cnt_ += v;
    }
    cond_.notify_all();
  }

  void reset() {
    std::lock_guard<std::mutex> lg(lock_);
    cnt_ = 0;
  }

  void wait(std::function<bool(int)> f) {
    std::unique_lock<std::mutex> ulock(lock_);
    cond_.wait(ulock, [&]() { return f(cnt_); });
  }

 private:
  std::condition_variable cond_;
  std::mutex lock_;
  size_t cnt_{0};
};

struct counts {
  size_t success{0};
  size_t failure{0};
  AtomicCounter cnt;

  void reset() {
    success = 0;
    failure = 0;
    cnt.reset();
  }
};

struct writelog_entry_t {
  std::shared_ptr<folly::File> file;
  std::string buf;
  awriter_entry_t awentry;
};

struct testing_context_t {
  counts* counter;
  writelog_entry_t log_context;
};

void callback_counter(awriter_entry_t* e, int result) {
  testing_context_t* w = (testing_context_t*)e->context;
  if (result) {
    w->counter->failure++;
  } else {
    w->counter->success++;
  }

  w->counter->cnt.notify(1);
}

int test_entry_writer(awriter_entry_t* e) {
  writelog_entry_t* entry = &((testing_context_t*)e->context)->log_context;
  ssize_t size =
      folly::writeFull(entry->file->fd(), entry->buf.data(), entry->buf.size());
  if (size == -1) {
    return errno;
  }
  if (static_cast<size_t>(size) < entry->buf.size()) {
    return EIO;
  }
  return 0;
}

const awriter_callbacks_t test_callbacks = {
    &callback_counter,
    &test_entry_writer};
} // namespace

// Simple test that creates a number of async writers and
// then destroys them.
TEST(awriter, create_destroy) {
  const int num_entries = 20;
  std::unique_ptr<AsyncWriter> w[num_entries];
  size_t i;

  for (i = 0; i < num_entries; i++) {
    w[i] = std::make_unique<AsyncWriter>(i);
  }
}

// Test that creates an async writer, writes a few things
// to it and checks that the writes complete with succes
// and that the file is written to.
TEST(awriter, sanity) {
  counts testCounter;
  TemporaryFile f("awriter_test");
  const int num_entries = 3;
  testing_context_t e[num_entries];
  struct stat s;
  auto fd = std::make_shared<folly::File>(f.fd());

  auto w = std::make_unique<AsyncWriter>();
  EXPECT_TRUE(w->start("awriter:test"));

  for (int i = 0; i < num_entries; i++) {
    e[i].counter = &testCounter;
    e[i].log_context.file = fd;
    e[i].log_context.buf = kTestMessage.str();
    e[i].log_context.awentry.context = e + i;
    e[i].log_context.awentry.callbacks = &test_callbacks;
    EXPECT_TRUE(awriter_queue(w.get(), &e[i].log_context.awentry));
  }

  testCounter.cnt.wait([](int v) { return v >= num_entries; });

  w.reset();
  EXPECT_EQ(testCounter.success, num_entries);

  EXPECT_EQ(fstat(f.fd(), &s), 0);

  EXPECT_EQ(s.st_size, num_entries * kTestMessage.size());
}

// Test that ensures that pending items in the queue are
// flushed when the writer is stopped.
TEST(awriter, flush_queue) {
  counts testCounter;
  TemporaryFile f("awriter_test");
  const int num_entries = 10;
  testing_context_t e[num_entries];

  auto w = std::make_unique<AsyncWriter>(0);

  for (int i = 0; i < num_entries; i++) {
    e[i].counter = &testCounter;
    e[i].log_context.buf = kTestMessage.str();
    e[i].log_context.awentry.context = e + i;
    e[i].log_context.awentry.callbacks = &test_callbacks;
    EXPECT_TRUE(awriter_queue(w.get(), &e[i].log_context.awentry));
  }

  // Stop the writer even before we start the thread.
  w->stop();

  EXPECT_EQ(testCounter.failure, num_entries);
}

// Test that ensures that maximum queue length is honored.
TEST(awriter, max_queue_length) {
  counts testCounter;
  TemporaryFile f("awriter_test");
  const int maxlen = 5;
  const int num_entries = maxlen + 5;
  testing_context_t e[num_entries];
  auto fd = std::make_shared<folly::File>(f.fd());

  auto w = std::make_unique<AsyncWriter>(maxlen);
  EXPECT_TRUE(w != nullptr);

  for (int i = 0; i < num_entries; i++) {
    e[i].counter = &testCounter;
    e[i].log_context.file = fd;
    e[i].log_context.buf = kTestMessage.str();
    e[i].log_context.awentry.context = e + i;
    e[i].log_context.awentry.callbacks = &test_callbacks;
    bool ret = awriter_queue(w.get(), &e[i].log_context.awentry);
    if (i < maxlen) {
      EXPECT_TRUE(ret);
    } else {
      EXPECT_FALSE(ret);
    }
  }

  // Create the thread to process the requests and wait for all
  // of them to be completed.
  EXPECT_TRUE(w->start("awriter:test"));

  testCounter.cnt.wait([](int v) { return v >= maxlen; });

  EXPECT_EQ(testCounter.success, maxlen);

  // Make sure we can submit an entry again.
  EXPECT_TRUE(awriter_queue(w.get(), &e[0].log_context.awentry));
}

// Test that passes invalid fd and expect errors when writing.
TEST(awriter, invalid_fd) {
  counts testCounter;
  const int num_entries = 3;
  testing_context_t e[num_entries];
  auto fd = std::make_shared<folly::File>(-1);

  auto w = std::make_unique<AsyncWriter>(0);
  EXPECT_TRUE(w->start("awriter:test"));

  for (int i = 0; i < num_entries; i++) {
    e[i].counter = &testCounter;
    e[i].log_context.file = fd;
    e[i].log_context.buf = kTestMessage.str();
    e[i].log_context.awentry.context = e + i;
    e[i].log_context.awentry.callbacks = &test_callbacks;
    EXPECT_TRUE(awriter_queue(w.get(), &e[i].log_context.awentry));
  }

  testCounter.cnt.wait([](int v) { return v >= num_entries; });

  w->stop();

  EXPECT_EQ(testCounter.failure, num_entries);
}
