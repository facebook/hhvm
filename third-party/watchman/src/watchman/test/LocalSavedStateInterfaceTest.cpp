/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "watchman/saved_state/LocalSavedStateInterface.h"
#include <folly/portability/GTest.h>
#include <folly/test/TestUtils.h>
#include "watchman/Errors.h"
#include "watchman/test/lib/FakeFileSystem.h"
#include "watchman/thirdparty/jansson/jansson.h"

using namespace watchman;

void expect_query_parse_error(
    const json_ref& config,
    const char* expectedError) {
  EXPECT_THROW_RE(
      LocalSavedStateInterface interface(config, nullptr),
      QueryParseError,
      expectedError);
}

TEST(LocalSavedStateInterfaceTest, max_commits) {
  auto localStoragePath =
      w_string_to_json(w_string(FAKEFS_ROOT "absolute/path"));
  auto project = w_string_to_json("foo");
  expect_query_parse_error(
      json_object(
          {{"local-storage-path", localStoragePath},
           {"project", project},
           {"max-commits", w_string_to_json("string")}}),
      "failed to parse query: 'max-commits' must be an integer");
  expect_query_parse_error(
      json_object(
          {{"local-storage-path", localStoragePath},
           {"project", project},
           {"max-commits", json_integer(0)}}),
      "failed to parse query: 'max-commits' must be a positive integer");
  expect_query_parse_error(
      json_object(
          {{"local-storage-path", localStoragePath},
           {"project", project},
           {"max-commits", json_integer(-1)}}),
      "failed to parse query: 'max-commits' must be a positive integer");
  LocalSavedStateInterface interface(
      json_object(
          {{"local-storage-path", localStoragePath},
           {"project", project},
           {"max-commits", json_integer(1)}}),
      nullptr);
  EXPECT_TRUE(true) << "expected constructor to succeed";
}

TEST(LocalSavedStateInterfaceTest, localStoragePath) {
  auto project = w_string_to_json(w_string("foo"));
  expect_query_parse_error(
      json_object({{"project", project}}),
      "failed to parse query: 'local-storage-path' must be present in saved state config");
  expect_query_parse_error(
      json_object(
          {{"project", project}, {"local-storage-path", json_integer(5)}}),
      "failed to parse query: 'local-storage-path' must be a string");
  expect_query_parse_error(
      json_object(
          {{"project", project},
           {"local-storage-path", w_string_to_json("relative/path")}}),
      "failed to parse query: 'local-storage-path' must be an absolute path");
  LocalSavedStateInterface interface(
      json_object(
          {{"project", project},
           {"local-storage-path",
            w_string_to_json(FAKEFS_ROOT "absolute/path")}}),
      nullptr);
  EXPECT_TRUE(true) << "expected constructor to succeed";
}

TEST(LocalSavedStateInterfaceTest, project) {
  auto localStoragePath = w_string_to_json(FAKEFS_ROOT "absolute/path");
  expect_query_parse_error(
      json_object({{"local-storage-path", localStoragePath}}),
      "failed to parse query: 'project' must be present in saved state config");
  expect_query_parse_error(
      json_object(
          {{"local-storage-path", localStoragePath},
           {"project", json_integer(5)}}),
      "failed to parse query: 'project' must be a string");
  expect_query_parse_error(
      json_object(
          {{"local-storage-path", localStoragePath},
           {"project", w_string_to_json(FAKEFS_ROOT "absolute/path")}}),
      "failed to parse query: 'project' must be a relative path");
  LocalSavedStateInterface interface(
      json_object(
          {{"local-storage-path", localStoragePath},
           {"project", w_string_to_json("foo")}}),
      nullptr);
  EXPECT_TRUE(true) << "expected constructor to succeed";
}

TEST(LocalSavedStateInterfaceTest, project_metadata) {
  auto localStoragePath = w_string_to_json(FAKEFS_ROOT "absolute/path");
  expect_query_parse_error(
      json_object(
          {{"local-storage-path", localStoragePath},
           {"project", w_string_to_json("relative/path")},
           {"project-metadata", json_integer(5)}}),
      "failed to parse query: 'project-metadata' must be a string");
  LocalSavedStateInterface interface(
      json_object(
          {{"local-storage-path", localStoragePath},
           {"project", w_string_to_json("foo")},
           {"project-metadata", w_string_to_json("meta")}}),
      nullptr);
  EXPECT_TRUE(true) << "expected constructor to succeed";
}

TEST(LocalSavedStateInterfaceTest, path) {
  auto localStoragePath = w_string_to_json(FAKEFS_ROOT "absolute/path");
  LocalSavedStateInterface interface(
      json_object(
          {{"local-storage-path", localStoragePath},
           {"project", w_string_to_json("foo")}}),
      nullptr);
  auto path = interface.getLocalPath("hash");
  w_string_piece expectedPath = FAKEFS_ROOT "absolute/path/foo/hash";
  EXPECT_EQ(path, expectedPath);
  interface = LocalSavedStateInterface(
      json_object(
          {{"local-storage-path", localStoragePath},
           {"project", w_string_to_json("foo")},
           {"project-metadata", w_string_to_json("meta")}}),
      nullptr);
  path = interface.getLocalPath("hash");
  expectedPath = FAKEFS_ROOT "absolute/path/foo/hash_meta";
  EXPECT_EQ(path, expectedPath);
}
