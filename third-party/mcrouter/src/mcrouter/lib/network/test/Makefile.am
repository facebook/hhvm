# Copyright (c) Facebook, Inc. and its affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

noinst_PROGRAMS = mock_mc_server
noinst_LIBRARIES = libtest_util.a
check_PROGRAMS = mcrouter_network_test

mock_mc_server_SOURCES = \
  MockMc.cpp \
  MockMc.h \
  MockMcOnRequest.h \
  MockMcServer.cpp
mock_mc_server_CPPFLAGS = -I$(top_srcdir)/..
# Link order matters.
mock_mc_server_LDADD = \
	$(top_builddir)/lib/libmcrouter.a \
	-lthriftcpp2 \
	-ltransport \
  -lthriftanyrep \
	-lthrifttype \
	-lthrifttyperep \
	-lthriftprotocol \
	-lrpcmetadata \
	-lthriftmetadata \
	-lasync \
	-lconcurrency \
	-lthrift-core \
	-lfmt \
	-lfizz \
	-lwangle \
	-lfolly

mock_mc_thrift_server_SOURCES = \
  MockMc.cpp \
  MockMc.h \
  MockMcOnRequest.h \
  MockMcThriftServer.cpp
mock_mc_thrift_server_CPPFLAGS = -I$(top_srcdir)/..
# Link order matters.
mock_mc_thrift_server_LDADD = \
	$(top_builddir)/lib/libmcrouter.a \
	-lthriftcpp2 \
	-ltransport \
	-lthriftprotocol \
	-lrpcmetadata \
	-lasync \
	-lconcurrency \
	-lthrift-core \
	-lfmt \
	-lfizz \
	-lwangle \
	-lfolly

mock_mc_server_dual_SOURCES = \
  MockMc.cpp \
  MockMc.h \
  MockMcOnRequest.h \
  MockMcServerDual.cpp
mock_mc_server_dual_CPPFLAGS = -I$(top_srcdir)/..
# Link order matters.
mock_mc_server_dual_LDADD = \
	$(top_builddir)/lib/libmcrouter.a \
	-lthriftcpp2 \
	-ltransport \
	-lthriftprotocol \
	-lrpcmetadata \
	-lasync \
	-lconcurrency \
	-lthrift-core \
	-lfmt \
	-lfizz \
	-lwangle \
	-lfolly

libtest_util_a_SOURCES = \
  ClientSocket.cpp \
  ClientSocket.h \
  ListenSocket.cpp \
  ListenSocket.h
libtest_util_a_CPPFLAGS = -I$(top_srcdir)/..

mcrouter_network_test_SOURCES = \
  AccessPointTest.cpp \
  AsyncMcClientTestSync.cpp \
  CarbonMessageDispatcherTest.cpp \
  CarbonMockMcTest.cpp \
  CarbonQueueAppenderTest.cpp \
  gen/CarbonTestMessages.cpp \
  McAsciiParserTest.cpp \
  McParserTest.cpp \
  McServerAsciiParserTest.cpp \
  MockMc.cpp \
  MockMcServer.cpp \
  SessionTest.cpp \
  SessionTestHarness.cpp \
  SessionTestHarness.h \
  TestClientServerUtil.cpp \
  TestClientServerUtil.h \
  TestMcAsciiParserUtil.cpp \
  TestMcAsciiParserUtil.h

mcrouter_network_test_CPPFLAGS = \
	-I$(top_srcdir)/.. \
	-isystem $(top_srcdir)/lib/gtest/include

mcrouter_network_test_LDADD = \
  $(top_builddir)/lib/libmcrouter.a \
  $(top_builddir)/lib/libtestmain.la \
  $(top_builddir)/lib/network/libtest_util.a \
  -lthriftcpp2 \
  -lthriftprotocol \
  -lrpcmetadata \
  -ltransport \
  -lwangle \
  -lfizz \
  -lsodium \
  -lfolly
