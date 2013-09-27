/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2013 Facebook, Inc. (http://www.facebook.com)     |
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
#include "hphp/util/kernel-version.h"
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

