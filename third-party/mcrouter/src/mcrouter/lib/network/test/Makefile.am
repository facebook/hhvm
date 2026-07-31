# Copyright (c) Facebook, Inc. and its affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

SUBDIRS = gen

noinst_PROGRAMS = mock_mc_server
noinst_LIBRARIES = libtest_util.a
check_PROGRAMS = mcrouter_network_test

# FBTHRIFT_STATIC_LIBS comes from configure so that Automake treats it as an
# opaque substitution; see the comment on AC_SUBST in configure.ac.

mock_mc_server_SOURCES = \
  MockMc.cpp \
  MockMc.h \
  MockMcOnRequest.h \
  MockMcServer.cpp
mock_mc_server_CPPFLAGS = -I$(top_srcdir)/..
mock_mc_server_LINK = $(CXXLD) $(AM_CXXFLAGS) $(CXXFLAGS) $(AM_LDFLAGS) $(LDFLAGS) -o $@
# Link order matters.
mock_mc_server_LDADD = \
	$(top_builddir)/lib/libmcrouter.a \
	$(FBTHRIFT_STATIC_LIBS) \
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
	$(FBTHRIFT_STATIC_LIBS) \
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
	$(FBTHRIFT_STATIC_LIBS) \
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
  gen/CarbonTestMessagesThrift.cpp \
  gen/gen-cpp2/CarbonTest_types.cpp \
  gen/gen-cpp2/CarbonTest_types_binary.cpp \
  gen/gen-cpp2/CarbonTest_types_compact.cpp \
  gen/gen-cpp2/CarbonTest_types_serialization.cpp \
  gen/gen-cpp2/CarbonTest_data.cpp \
  gen/gen-cpp2/CarbonTest_constants.cpp \
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
  libtest_util.a \
  $(FBTHRIFT_STATIC_LIBS)
