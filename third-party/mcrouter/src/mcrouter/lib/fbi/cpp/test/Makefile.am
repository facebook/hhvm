# Copyright (c) Facebook, Inc. and its affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

check_PROGRAMS = mcrouter_fbi_cpp_test

mcrouter_fbi_cpp_test_SOURCES = \
	main.cpp

mcrouter_fbi_cpp_test_CPPFLAGS = \
	-I$(top_srcdir)/.. \
	-isystem $(top_srcdir)/lib/gtest/include

mcrouter_fbi_cpp_test_LDADD = \
  $(top_builddir)/lib/libmcrouter.a \
  $(top_builddir)/lib/libtestmain.la
