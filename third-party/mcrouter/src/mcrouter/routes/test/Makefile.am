# Copyright (c) Facebook, Inc. and its affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

check_PROGRAMS = mcrouter_routes_test

mcrouter_routes_test_SOURCES = \
  BigValueRouteTest.cpp \
  BigValueRouteTestBase.h \
  ConstShardHashFuncTest.cpp \
  FailoverWithExptimeRouteTest.cpp \
  Main.cpp \
  PoolRouteTest.cpp \
  RateLimitRouteTest.cpp \
  RouteHandleTestUtil.h \
  ShadowRouteTest.cpp \
  SlowWarmUpRouteTest.cpp \
  WarmUpRouteTest.cpp

mcrouter_routes_test_CPPFLAGS = \
 -I$(top_srcdir)/.. \
 -isystem $(top_srcdir)/lib/gtest/include

mcrouter_routes_test_LDADD = \
  $(top_builddir)/libmcroutercore.a \
  $(top_builddir)/lib/libmcrouter.a \
  $(top_builddir)/lib/libtestmain.la \
  -lwangle \
  -lfizz \
  -lsodium \
  -lfolly \
  -lfollybenchmark
