/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <folly/logging/xlog.h>
#include <folly/portability/GTest.h>
#include <folly/portability/SysTime.h>
#include "watchman/IgnoreSet.h"
#include "watchman/watchman_time.h"

namespace {

using namespace watchman;

// A list that looks similar to one used in one of our repos
const char* ignore_dirs[] = {
    ".buckd",
    ".idea",
    "_build",
    "buck-cache",
    "buck-out",
    "build",
    "foo/.buckd",
    "foo/buck-cache",
    "foo/buck-out",
    "bar/_build",
    "bar/buck-cache",
    "bar/buck-out",
    "baz/.buckd",
    "baz/buck-cache",
    "baz/buck-out",
    "baz/build",
    "baz/qux",
    "baz/focus-out",
    "baz/tmp",
    "baz/foo/bar/foo/build",
    "baz/foo/bar/bar/build",
    "baz/foo/bar/baz/build",
    "baz/foo/bar/qux",
    "baz/foo/baz/foo",
    "baz/bar/foo/foo/foo/foo/foo/foo",
    "baz/bar/bar/foo/foo",
    "baz/bar/bar/foo/foo"};

const char* ignore_vcs[] = {".hg", ".svn", ".git"};

struct test_case {
  const char* path;
  bool ignored;
};

void run_correctness_test(
    IgnoreSet* state,
    const struct test_case* tests,
    uint32_t num_tests) {
  uint32_t i;

  for (i = 0; i < num_tests; i++) {
    bool res = state->isIgnored(tests[i].path, strlen_uint32(tests[i].path));
    EXPECT_EQ(res, tests[i].ignored) << tests[i].path;
  }
}

void add_strings(
    IgnoreSet* ignore,
    const char** strings,
    uint32_t num_strings,
    bool is_vcs_ignore) {
  uint32_t i;
  for (i = 0; i < num_strings; i++) {
    ignore->add(w_string(strings[i], W_STRING_UNICODE), is_vcs_ignore);
  }
}

void init_state(IgnoreSet* state) {
  add_strings(state, ignore_dirs, std::size(ignore_dirs), false);
  add_strings(state, ignore_vcs, std::size(ignore_vcs), true);
}

TEST(Ignore, correctness) {
  IgnoreSet state;
  static const struct test_case tests[] = {
      {"some/path", false},
      {"buck-out/gen/foo", true},
      {".hg/wlock", false},
      {".hg/store/foo", true},
      {"buck-out", true},
      {"foo/buck-out", true},
      {"foo/hello", false},
      {"baz/hello", false},
      {".hg", false},
      {"buil", false},
      {"build", true},
      {"build/lower", true},
      {"builda", false},
      {"build/bar", true},
      {"buildfile", false},
      {"build/lower/baz", true},
      {"builda/hello", false},
  };

  init_state(&state);

  run_correctness_test(&state, tests, sizeof(tests) / sizeof(tests[0]));
}

// Load up the words data file and build a list of strings from that list.
// Each of those strings is prefixed with the supplied string.
// If there are fewer than limit entries available in the data file, we will
// abort.
std::vector<w_string> build_list_with_prefix(const char* prefix, size_t limit) {
  std::vector<w_string> strings;
  char buf[512];
  FILE* f = fopen("thirdparty/libart/tests/words.txt", "r");

#ifdef WATCHMAN_TEST_SRC_DIR
  if (!f) {
    f = fopen(
        WATCHMAN_TEST_SRC_DIR "/watchman/thirdparty/libart/tests/words.txt",
        "r");
  }
#endif

  if (!f) {
    f = fopen("watchman/thirdparty/libart/tests/words.txt", "r");
  }

  if (!f) {
    char cwd[4096];
    if (getcwd(cwd, sizeof(cwd))) {
      throw std::runtime_error{
          std::string{"Could not load words.txt when run from "} + cwd};
    } else {
      throw std::runtime_error{
          "Could not load words.txt, unknown working directory"};
    }
  }

  while (fgets(buf, sizeof buf, f)) {
    // Remove newline
    uint32_t len = strlen_uint32(buf);
    buf[len - 1] = '\0';
    strings.emplace_back(w_string::build(prefix, buf));

    if (strings.size() >= limit) {
      break;
    }
  }

  EXPECT_GE(strings.size(), limit);

  return strings;
}

static const size_t kWordLimit = 230000;

void bench_list(const char* label, const char* prefix) {
  IgnoreSet state;
  size_t i, n;
  struct timeval start, end;

  init_state(&state);
  auto strings = build_list_with_prefix(prefix, kWordLimit);

  gettimeofday(&start, nullptr);
  for (n = 0; n < 100; n++) {
    for (i = 0; i < kWordLimit; i++) {
      state.isIgnored(strings[i].data(), strings[i].size());
    }
  }
  gettimeofday(&end, nullptr);

  XLOG(ERR) << label << ": took " << w_timeval_diff(start, end);
}

TEST(Ignore, bench_all_ignores) {
  bench_list("all_ignores_tree", "baz/buck-out/gen/");
}

TEST(Ignore, bench_no_ignores) {
  bench_list("no_ignores_tree", "baz/some/path");
}

} // namespace
