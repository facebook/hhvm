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

#include <exception>
#include <memory>

#include <folly/futures/Future.h>
#include <folly/portability/GMock.h>
#include <folly/portability/GTest.h>

#include "hphp/runtime/ext/facts/static-watcher.h"

using ::testing::ByMove;
using ::testing::ElementsAre;
using ::testing::Return;
using ::testing::WithArg;

namespace HPHP {
namespace Facts {
namespace {

TEST(StaticWatcherTest, sinceAndClockAndFilesArePassedThrough) {
  auto watcher = make_static_watcher({"a.hck"});

  auto since = Clock{.m_clock = {}};
  auto result = watcher->getChanges(since).get();
  EXPECT_EQ(result.m_lastClock, since);
  EXPECT_EQ(result.m_newClock, Clock{.m_clock = "1"});
  EXPECT_THAT(
      result.m_files,
      ElementsAre(Watcher::ResultFile{
          .m_path = "a.hck", .m_exists = true, .m_watcher_hash = {}}));
}

TEST(StaticWatcherTest, incrementsClock) {
  auto watcher = make_static_watcher({"a.hck"});

  auto since = Clock{.m_clock = "42"};
  auto result = watcher->getChanges(since).get();
  EXPECT_EQ(result.m_lastClock, since);
  EXPECT_EQ(result.m_newClock, Clock{.m_clock = "43"});
  EXPECT_THAT(
      result.m_files,
      ElementsAre(Watcher::ResultFile{
          .m_path = "a.hck", .m_exists = true, .m_watcher_hash = {}}));
}

} // namespace
} // namespace Facts
} // namespace HPHP
