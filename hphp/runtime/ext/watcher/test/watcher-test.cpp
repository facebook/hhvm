/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-present Facebook, Inc. (http://www.facebook.com)  |
   +----------------------------------------------------------------------+
   | This source path is subject to version 3.01 of the PHP license,      |
   | that is bundled with this package in the path LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | http://www.php.net/license/3_01.txt                                  |
   | If you did not receive a copy of the PHP license and are unable to   |
   | obtain it through the world-wide-web, please send a note to          |
   | license@php.net so we can mail you a copy immediately.               |
   +----------------------------------------------------------------------+
*/

#include <folly/portability/GMock.h>
#include <folly/portability/GTest.h>
#include <folly/testing/TestUtil.h>

#include "hphp/runtime/ext/watcher/watcher-clock.h"
#include "hphp/runtime/ext/watcher/watcher-options.h"

using ::testing::_;
using ::testing::AllOf;
using ::testing::Contains;
using ::testing::ElementsAre;
using ::testing::Eq;
using ::testing::IsEmpty;
using ::testing::NiceMock;
using ::testing::Return;
using ::testing::SaveArg;
using ::testing::SizeIs;
using ::testing::UnorderedElementsAre;

namespace HPHP {
namespace Watcher {

TEST(WatcherOptionsQueryTest, IncludePaths) {

  WatcherOptions options;
  options.include_paths = { "cows/jersey", "cows/angus", "cows/dexter" };


  auto expected = 
    folly::dynamic::array(
        "query",
        ".",
        folly::dynamic::object
          ("fields", folly::dynamic::array("name"))
          ("expression", 
            folly::dynamic::array(
              "allof",
              folly::dynamic::array("type", "f"),
              folly::dynamic::array("anyof",
                folly::dynamic::array("dirname", "cows/jersey"),
                folly::dynamic::array("dirname", "cows/angus"),
                folly::dynamic::array("dirname", "cows/dexter")
              )
            )
          )
        );

  EXPECT_THAT(options.watchmanQuery(), Eq(expected));
}

TEST(WatcherOptionsQueryTest, ExcludePaths) {

  WatcherOptions options;
  options.exclude_paths = { "cows/jersey", "cows/angus", "cows/dexter" };


  auto expected = 
    folly::dynamic::array(
        "query",
        ".",
        folly::dynamic::object
          ("fields", folly::dynamic::array("name"))
          ("expression", 
            folly::dynamic::array(
              "allof",
              folly::dynamic::array("type", "f"),
              folly::dynamic::array("not", 
                folly::dynamic::array("anyof",
                  folly::dynamic::array("dirname", "cows/jersey"),
                  folly::dynamic::array("dirname", "cows/angus"),
                  folly::dynamic::array("dirname", "cows/dexter")
                )
              )
            )
          )
        );

  EXPECT_THAT(options.watchmanQuery(), Eq(expected));
}

TEST(WatcherOptionsQueryTest, IncludeExtensions) {

  WatcherOptions options;
  options.include_extensions = { "php", "hack", "js" };


  auto expected = 
    folly::dynamic::array(
        "query",
        ".",
        folly::dynamic::object
          ("fields", folly::dynamic::array("name"))
          ("expression", 
            folly::dynamic::array(
              "allof",
              folly::dynamic::array("type", "f"),
              folly::dynamic::array("anyof",
                folly::dynamic::array("suffix", "php"),
                folly::dynamic::array("suffix", "hack"),
                folly::dynamic::array("suffix", "js")
              )
            )
          )
          ("suffix", folly::dynamic::array("php", "hack", "js"))
        );

  EXPECT_THAT(options.watchmanQuery(), Eq(expected));
}

TEST(WatcherOptionsQueryTest, ExcludeExtensions) {

  WatcherOptions options;
  options.exclude_extensions = { "php", "hack", "js" };


  auto expected = 
    folly::dynamic::array(
        "query",
        ".",
        folly::dynamic::object
          ("fields", folly::dynamic::array("name"))
          ("expression", 
            folly::dynamic::array(
              "allof",
              folly::dynamic::array("type", "f"),
              folly::dynamic::array("not",
                folly::dynamic::array("anyof",
                  folly::dynamic::array("suffix", "php"),
                  folly::dynamic::array("suffix", "hack"),
                  folly::dynamic::array("suffix", "js")
                )
              )
            )
          )
        );

  EXPECT_THAT(options.watchmanQuery(), Eq(expected));
}

TEST(WatcherOptionsQueryTest, RootAndRelativeRoot) {

  WatcherOptions options;
  options.root = "some_root";
  options.relative_root = "some_subdirectory";

  auto expected = 
    folly::dynamic::array(
        "query",
        "some_root",
        folly::dynamic::object
          ("fields", folly::dynamic::array("name"))
          ("relative_root", "some_subdirectory")
        );

  EXPECT_THAT(options.watchmanQuery(), Eq(expected));
}

TEST(WatcherOptionsQueryTest, SinceMergebase) {

  WatcherOptions options;
  options.clock = WatcherClock::fromClock(WatcherClockType::MERGEBASE, "abc123");

  auto expected = 
    folly::dynamic::array(
        "query",
        ".",
        folly::dynamic::object
          ("fields", folly::dynamic::array("name"))
          ("since", folly::dynamic::object("scm", folly::dynamic::object("mergebase-with", "abc123")))
        );

  EXPECT_THAT(options.watchmanQuery(), Eq(expected));
}

TEST(WatcherOptionsQueryTest, SinceClock) {

  WatcherOptions options;
  options.clock = WatcherClock::fromClock(WatcherClockType::SINCE, "abc123");

  auto expected = 
    folly::dynamic::array(
        "query",
        ".",
        folly::dynamic::object
          ("fields", folly::dynamic::array("name"))
          ("since", "abc123")
        );

  EXPECT_THAT(options.watchmanQuery(), Eq(expected));
}

TEST(WatcherOptionsQueryTest, AllTheThings) {

  WatcherOptions options;
  options.root = "/home/hack/site";
  options.relative_root = "some_subdirectory";
  options.include_paths = { "orange", "banana", "pineapple" };
  options.exclude_paths = { "orange/mandarin", "banana/cavendish" };
  options.include_extensions = { "php", "hack" };
  options.exclude_extensions = { "js" };
  options.fields = { "name", "content.sha1hex" };
  options.clock = WatcherClock::fromClock(WatcherClockType::SINCE, "abc123");

  auto expected =
    folly::dynamic::array(
        "query",
        "/home/hack/site",
        folly::dynamic::object
          ("expression",
            folly::dynamic::array(
              "allof",
              folly::dynamic::array("type", "f"),
              folly::dynamic::array("anyof",
                folly::dynamic::array("dirname", "orange"),
                folly::dynamic::array("dirname", "banana"),
                folly::dynamic::array("dirname", "pineapple")
              ),
              folly::dynamic::array("anyof",
                folly::dynamic::array("suffix", "php"),
                folly::dynamic::array("suffix", "hack")
              ),
              folly::dynamic::array("not",
                folly::dynamic::array("anyof",
                  folly::dynamic::array("dirname", "orange/mandarin"),
                  folly::dynamic::array("dirname", "banana/cavendish")
                )
              ),
              folly::dynamic::array("not",
                folly::dynamic::array("suffix", "js")
              )
            )
          )
          ("fields", folly::dynamic::array("name", "content.sha1hex"))
          ("relative_root", "some_subdirectory")
          ("suffix", folly::dynamic::array("php", "hack"))
          ("since", "abc123")
        );

  EXPECT_THAT(options.watchmanQuery(), Eq(expected));
}

} // namespace Watcher
} // namespace HPHP
