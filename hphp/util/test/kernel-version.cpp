/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2016 Facebook, Inc. (http://www.facebook.com)     |
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

TEST(KernelVersion, SimpleTest1) {
  KernelVersion minKv("3.2.28-72_fbk12");
  EXPECT_EQ(3, minKv.m_major);
  EXPECT_EQ(2, minKv.m_minor);
  EXPECT_EQ(28, minKv.m_release);
  EXPECT_EQ(72, minKv.m_build);
  EXPECT_EQ(0, strcmp(minKv.m_release_str.c_str(), "28"));
  EXPECT_EQ(0, strcmp(minKv.m_build_str.c_str(), "72"));
  EXPECT_EQ(12, minKv.m_fbk);
}

TEST(KernelVersion, SimpleTest2) {
  KernelVersion minKvB("3.14-1-amd64_fbk64");
  EXPECT_EQ(3, minKvB.m_major);
  EXPECT_EQ(14, minKvB.m_minor);
  EXPECT_EQ(1, minKvB.m_release);
  EXPECT_EQ(-1, minKvB.m_build);
  EXPECT_EQ(0, strcmp(minKvB.m_release_str.c_str(), "1"));
  EXPECT_EQ(0, strcmp(minKvB.m_build_str.c_str(), "amd64"));
  EXPECT_EQ(64, minKvB.m_fbk);
}

TEST(KernelVersion, SimpleTest3) {
  KernelVersion minKvC("3.16-trunk-amd64");
  EXPECT_EQ(3, minKvC.m_major);
  EXPECT_EQ(16, minKvC.m_minor);
  EXPECT_EQ(-1, minKvC.m_release);
  EXPECT_EQ(-1, minKvC.m_build);
  EXPECT_EQ(0, strcmp(minKvC.m_release_str.c_str(), "trunk"));
  EXPECT_EQ(0, strcmp(minKvC.m_build_str.c_str(), "amd64"));
  EXPECT_EQ(-1, minKvC.m_fbk);
}

TEST(KernelVersion, SimpleTest4) {
  KernelVersion minKvD("3.16-trunk-amd64_fbk12");
  EXPECT_EQ(3, minKvD.m_major);
  EXPECT_EQ(16, minKvD.m_minor);
  EXPECT_EQ(-1, minKvD.m_release);
  EXPECT_EQ(-1, minKvD.m_build);
  EXPECT_EQ(0, strcmp(minKvD.m_release_str.c_str(), "trunk"));
  EXPECT_EQ(0, strcmp(minKvD.m_build_str.c_str(), "amd64"));
  EXPECT_EQ(12, minKvD.m_fbk);
}

TEST(KernelVersion, SimpleTest5) {
  KernelVersion minKvE("3.14-1-i386");
  EXPECT_EQ(3, minKvE.m_major);
  EXPECT_EQ(14, minKvE.m_minor);
  EXPECT_EQ(1, minKvE.m_release);
  EXPECT_EQ(-1, minKvE.m_build);
  EXPECT_EQ(0, strcmp(minKvE.m_release_str.c_str(), "1"));
  EXPECT_EQ(0, strcmp(minKvE.m_build_str.c_str(), "i386"));
  EXPECT_EQ(-1, minKvE.m_fbk);
}

TEST(KernelVersion, SimpleTest6) {
  KernelVersion minKvF("3.14");
  EXPECT_EQ(3, minKvF.m_major);
  EXPECT_EQ(14, minKvF.m_minor);
  EXPECT_EQ(-1, minKvF.m_release);
  EXPECT_EQ(-1, minKvF.m_build);
  EXPECT_EQ(0, strcmp(minKvF.m_release_str.c_str(), ""));
  EXPECT_EQ(0, strcmp(minKvF.m_build_str.c_str(), ""));
  EXPECT_EQ(-1, minKvF.m_fbk);
}

}
