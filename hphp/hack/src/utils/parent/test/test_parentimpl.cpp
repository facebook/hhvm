#include <folly/portability/GTest.h>
#include "../parentimpl.h"
#include <folly/portability/Unistd.h>

TEST(ParentImpl, WatchdogCount) {
  ::testing::FLAGS_gtest_death_test_style = "threadsafe"; // paranoia
  ASSERT_EQ(get_watchdog_count_(), 0);
  // 1000 is an arbitrary large number.
  // The thread will be spawned but it won't check for
  // the death of the parent
  exit_on_parent_exit_(1000, 1000);
  ASSERT_EQ(get_watchdog_count_(), 1);
  // try again, make sure that the number of watchdog
  // processes is still 1.
  exit_on_parent_exit_(1000, 1000);
  ASSERT_EQ(get_watchdog_count_(), 1);
}
