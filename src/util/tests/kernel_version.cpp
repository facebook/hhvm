#include "util/kernel_version.h"
#include <gtest/gtest.h>

namespace HPHP {

TEST(KernelVersion, SimpleTest) {
  KernelVersion minKv("3.2.28-72_fbk12");
  EXPECT_EQ(3, minKv.m_major);
  EXPECT_EQ(2, minKv.m_dot);
  EXPECT_EQ(28, minKv.m_dotdot);
  EXPECT_EQ(72, minKv.m_dash);
  EXPECT_EQ(12, minKv.m_fbk);
}

}

