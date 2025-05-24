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

#include "hphp/util/multiglob.h"

#include "tools/cxx/Resources.h"

#include <folly/portability/GTest.h>

#include <filesystem>

namespace HPHP {

static constexpr auto TEST_DATA_ROOT_DIR = "hphp/util/test/multiglob-data";

TEST(multiglob, static) {
  auto root = std::filesystem::path(build::getResourcePath(TEST_DATA_ROOT_DIR).string());
  {
    auto res = MultiGlob::matches(std::set<std::string>{"foo/bar.txt"}, root);
    std::sort(res.begin(), res.end());
    std::for_each(res.begin(), res.end(), [&](auto& path) { path = std::filesystem::relative(path, root); });
    EXPECT_EQ(res, (std::vector<std::filesystem::path>{ "foo/bar.txt" }));
  }

  {
    auto res = MultiGlob::matches(std::set<std::string>{"foo/bar.txt", "foo/baz.txt"}, root);
    std::sort(res.begin(), res.end());
    std::for_each(res.begin(), res.end(), [&](auto& path) { path = std::filesystem::relative(path, root); });
    EXPECT_EQ(res, (std::vector<std::filesystem::path>{ "foo/bar.txt", "foo/baz.txt" }));
  }
}

TEST(multiglob, pattern) {
  auto root = std::filesystem::path(build::getResourcePath(TEST_DATA_ROOT_DIR).string());
  {
    auto res = MultiGlob::matches(std::set<std::string>{"foo/*.txt"}, root);
    std::sort(res.begin(), res.end());
    std::for_each(res.begin(), res.end(), [&](auto& path) { path = std::filesystem::relative(path, root); });
    EXPECT_EQ(res, (std::vector<std::filesystem::path>{ "foo/bar.txt", "foo/baz.txt" }));
  }

  {
    auto res = MultiGlob::matches(std::set<std::string>{"foo/1/2/???.*"}, root);
    std::sort(res.begin(), res.end());
    std::for_each(res.begin(), res.end(), [&](auto& path) { path = std::filesystem::relative(path, root); });
    EXPECT_EQ(res, (std::vector<std::filesystem::path>{ "foo/1/2/bar.txt", "foo/1/2/baz.json" }));
  }
}

TEST(multiglob, wildcard) {
  auto root = std::filesystem::path(build::getResourcePath(TEST_DATA_ROOT_DIR).string());
  {
    auto res = MultiGlob::matches(std::set<std::string>{"foo/*/*/*"}, root);
    std::sort(res.begin(), res.end());
    std::for_each(res.begin(), res.end(), [&](auto& path) { path = std::filesystem::relative(path, root); });
    EXPECT_EQ(res, (std::vector<std::filesystem::path>{ "foo/1/2/bar.txt", "foo/1/2/baz.json" }));
  }

  {
    auto res = MultiGlob::matches(std::set<std::string>{"foo/*/*/bar.txt"}, root);
    std::sort(res.begin(), res.end());
    std::for_each(res.begin(), res.end(), [&](auto& path) { path = std::filesystem::relative(path, root); });
    EXPECT_EQ(res, (std::vector<std::filesystem::path>{ "foo/1/2/bar.txt" }));
  }
}

TEST(multiglob, recursivewildcard) {
  auto root = std::filesystem::path(build::getResourcePath(TEST_DATA_ROOT_DIR).string());
  {
    auto res = MultiGlob::matches(std::set<std::string>{"foo/**/bar.txt"}, root);
    std::sort(res.begin(), res.end());
    std::for_each(res.begin(), res.end(), [&](auto& path) { path = std::filesystem::relative(path, root); });
    EXPECT_EQ(res, (std::vector<std::filesystem::path>{ "foo/1/2/bar.txt", "foo/bar.txt" }));
  }

  {
    auto res = MultiGlob::matches(std::set<std::string>{"foo/**/bar.txt/**"}, root);
    std::sort(res.begin(), res.end());
    std::for_each(res.begin(), res.end(), [&](auto& path) { path = std::filesystem::relative(path, root); });
    EXPECT_EQ(res, (std::vector<std::filesystem::path>{ "foo/1/2/bar.txt", "foo/bar.txt" }));
  }

  {
    auto res = MultiGlob::matches(std::set<std::string>{"foo/**"}, root);
    std::sort(res.begin(), res.end());
    std::for_each(res.begin(), res.end(), [&](auto& path) { path = std::filesystem::relative(path, root); });
    EXPECT_EQ(res, (std::vector<std::filesystem::path>{ "foo/1/2/bar.txt", "foo/1/2/baz.json", "foo/bar.txt", "foo/baz.txt" }));
  }

  {
    auto res = MultiGlob::matches(std::set<std::string>{"foo/**/**"}, root);
    std::sort(res.begin(), res.end());
    std::for_each(res.begin(), res.end(), [&](auto& path) { path = std::filesystem::relative(path, root); });
    EXPECT_EQ(res, (std::vector<std::filesystem::path>{ "foo/1/2/bar.txt", "foo/1/2/baz.json", "foo/bar.txt", "foo/baz.txt" }));
  }
}

TEST(multiglob, all) {
  auto root = std::filesystem::path(build::getResourcePath(TEST_DATA_ROOT_DIR).string());
  {
    auto res = MultiGlob::matches(std::set<std::string>{"foo/**/*.txt"}, root);
    std::sort(res.begin(), res.end());
    std::for_each(res.begin(), res.end(), [&](auto& path) { path = std::filesystem::relative(path, root); });
    EXPECT_EQ(res, (std::vector<std::filesystem::path>{ "foo/1/2/bar.txt", "foo/bar.txt", "foo/baz.txt" }));
  }

  {
    auto res = MultiGlob::matches(std::set<std::string>{"**/*.txt"}, root);
    std::sort(res.begin(), res.end());
    std::for_each(res.begin(), res.end(), [&](auto& path) { path = std::filesystem::relative(path, root); });
    EXPECT_EQ(res, (std::vector<std::filesystem::path>{ "foo/1/2/bar.txt", "foo/bar.txt", "foo/baz.txt" }));
  }
}

TEST(multiglob, complex) {
  auto root = std::filesystem::path(build::getResourcePath(TEST_DATA_ROOT_DIR).string());
  {
    auto res = MultiGlob::matches(std::set<std::string>{"foo/**/foo.txt", "foo/baz.json"}, root);
    std::sort(res.begin(), res.end());
    std::for_each(res.begin(), res.end(), [&](auto& path) { path = std::filesystem::relative(path, root); });
    EXPECT_EQ(res, (std::vector<std::filesystem::path>{}));
  }
}

}
