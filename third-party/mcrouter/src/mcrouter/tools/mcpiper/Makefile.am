# Copyright (c) Facebook, Inc. and its affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

bin_PROGRAMS = mcpiper

mcpiper_SOURCES = \
	AnsiColorCodeStream-inl.h \
	AnsiColorCodeStream.cpp \
	AnsiColorCodeStream.h \
	ClientServerMcParser-inl.h \
	ClientServerMcParser.h \
	Color.h \
	Config.cpp \
	Config.h \
	FifoReader.cpp \
	FifoReader.h \
	main.cpp \
	McPiper.cpp \
	McPiper.h \
	MessagePrinter-inl.h \
	MessagePrinter.cpp \
	MessagePrinter.h \
	PrettyFormat.h \
	StyleAwareStream.h \
	SnifferParser-inl.h \
	SnifferParser.h \
	StyleAwareStream-inl.h \
	StyleAwareStream.h \
	StyledString.cpp \
	StyledString.h \
	Util.cpp \
	Util.h \
	ValueFormatter.h

mcpiper_LDADD = \
	$(top_srcdir)/lib/libmcrouter.a \
	-lthriftcpp2 \
	-ltransport \
	-lthriftanyrep \
	-lthrifttype \
	-lthrifttyperep \
	-lthriftprotocol \
	-lrpcmetadata \
	-lasync \
	-lconcurrency \
	-lthrift-core \
	-lfizz \
	-lfmt \
	-lwangle \
	-lfolly

mcpiper_CPPFLAGS = -I$(top_srcdir)/..
