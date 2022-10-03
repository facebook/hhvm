/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "watchman/watchman_system.h"

#include <folly/String.h>
#include <folly/portability/GTest.h>
#include "watchman/thirdparty/jansson/jansson.h"
#include "watchman/thirdparty/wildmatch/wildmatch.h"

#define WILDMATCH_TEST_JSON_FILE "watchman/test/WildmatchTest.json"

static void run_test(const json_ref& test_case_data) {
  auto wildmatch_should_succeed = test_case_data.at(0).asBool();
  auto wildmatch_flags = test_case_data.at(1).asInt();
  auto text_to_match = json_string_value(test_case_data.at(2));
  auto pattern_to_use = json_string_value(test_case_data.at(3));

  auto wildmatch_succeeded =
      wildmatch(pattern_to_use, text_to_match, wildmatch_flags, nullptr) ==
      WM_MATCH;
  EXPECT_EQ(wildmatch_succeeded, wildmatch_should_succeed)
      << "Pattern [" << pattern_to_use << "] matching text [" << text_to_match
      << "] with flags " << wildmatch_flags;
}

TEST(WildMatch, tests) {
  FILE* test_cases_file;
  json_error_t error;

  test_cases_file = fopen(WILDMATCH_TEST_JSON_FILE, "r");
#ifdef WATCHMAN_TEST_SRC_DIR
  if (!test_cases_file) {
    test_cases_file =
        fopen(WATCHMAN_TEST_SRC_DIR "/" WILDMATCH_TEST_JSON_FILE, "r");
  }
#endif
  if (!test_cases_file) {
    test_cases_file = fopen("watchman/" WILDMATCH_TEST_JSON_FILE, "r");
  }
  if (!test_cases_file) {
    throw std::runtime_error(fmt::format(
        "Couldn't open {}: {}",
        WILDMATCH_TEST_JSON_FILE,
        folly::errnoStr(errno)));
  }
  auto test_cases = json_loadf(test_cases_file, 0, &error);
  if (!test_cases) {
    throw std::runtime_error(fmt::format(
        "Error decoding JSON: {} (source={}, line={}, col={})",
        error.text,
        error.source,
        error.line,
        error.column));
  }
  EXPECT_EQ(fclose(test_cases_file), 0);
  EXPECT_TRUE(test_cases.value().isArray())
      << "Expected JSON in " << WILDMATCH_TEST_JSON_FILE << "  to be an array";

  for (auto& test_case_data : test_cases.value().array()) {
    run_test(test_case_data);
  }
}
