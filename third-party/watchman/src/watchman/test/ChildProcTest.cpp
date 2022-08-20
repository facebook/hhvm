/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <folly/portability/GTest.h>
#include <folly/test/TestUtils.h>
#include <list>
#include "watchman/ChildProcess.h"
#include "watchman/watchman_system.h"

using watchman::ChildProcess;
using Options = ChildProcess::Options;

TEST(ChildProcess, pipe) {
  Options opts;
  opts.pipeStdout();
  ChildProcess echo(
      {
#ifdef _WIN32
          "cmd /c echo hello",
#else
          "echo",
          "hello",
#endif
      },
      std::move(opts));

  auto outputs = echo.communicate();
  EXPECT_EQ(0, echo.wait())
      << "\nstdout:\n"
      << outputs.first << "\nstderr:" << outputs.second << "\n";

  w_string_piece line(outputs.first);
  EXPECT_TRUE(line.startsWith("hello"))
      << "\nstdout:\n"
      << outputs.first << "\nstderr:" << outputs.second << "\n";
}

void test_pipe_input(bool threaded) {
#ifndef _WIN32
  Options opts;
  opts.pipeStdout();
  opts.pipeStdin();
  ChildProcess cat({"cat", "-"}, std::move(opts));

  std::vector<std::string> expected{"one", "two", "three"};
  std::list<std::string> lines{"one\n", "two\n", "three\n"};

  auto writable = [&lines](watchman::FileDescriptor& fd) {
    if (lines.empty()) {
      return true;
    }
    auto str = lines.front();
    if (write(fd.fd(), str.data(), str.size()) == -1) {
      throw std::runtime_error("write to child failed");
    }
    lines.pop_front();
    return false;
  };

  auto outputs =
      threaded ? cat.threadedCommunicate(writable) : cat.communicate(writable);
  cat.wait();

  std::vector<std::string> resultLines;
  w_string_piece(outputs.first).split(resultLines, '\n');
  EXPECT_EQ(resultLines.size(), 3);
  EXPECT_EQ(resultLines, expected);
#else
  (void)threaded;
#endif
}

TEST(ChildProcess, stresstest_pipe_output) {
  SKIP_IF(folly::kIsWindows);

  bool okay = true;
  for (int i = 0; i < 3000; ++i) {
    Options opts;
    opts.pipeStdout();
    ChildProcess proc({"head", "-n20", "/dev/urandom"}, std::move(opts));
    auto outputs = proc.communicate();
    w_string_piece out(outputs.first);
    proc.wait();
    if (out.empty() || out[out.size() - 1] != '\n') {
      okay = false;
      break;
    }
  }
  EXPECT_TRUE(okay);
}

TEST(ChildProcess, inputThreaded) {
  test_pipe_input(true);
}

TEST(ChildProcess, inputNotThreaded) {
  test_pipe_input(false);
}
