
AUTOMAKE_OPTIONS = foreign
ACLOCAL_AMFLAGS = -I m4

if ENABLE_EXAMPLES
  EXAMPLES_DIR=examples
endif

SUBDIRS = src $(EXAMPLES_DIR) doc

EXTRA_DIST = libedit.pc.in
pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = libedit.pc
