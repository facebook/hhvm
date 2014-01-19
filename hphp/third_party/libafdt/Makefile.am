AUTOMAKE_OPTIONS = foreign
ACLOCAL_AMFLAGS = -I m4

AM_CPPFLAGS = $(LIBEVENT_CPPFLAGS)
AM_LDFLAGS = $(LIBEVENT_LDFLAGS)


lib_LTLIBRARIES = libafdt.la

libafdt_la_SOURCES = \
	src/util.c \
	src/lowlevel.c \
	src/sync.c \
	src/strlcpy.c \
	src/internal.h

# Only enable the asynchronous interface if libevent is available.
if HAVE_LIBEVENT
libafdt_la_SOURCES += src/async.c
endif

include_HEADERS = \
	src/afdt.h


# The test programs require some newer libevent interfaces.
if HAVE_LIBEVENT_143

noinst_PROGRAMS = catter sync_catter
PROGRAMS_CPPFLAGS = $(LIBEVENT_CPPFLAGS) -I $(top_srcdir)/src
PROGRAMS_LDADD = libafdt.la $(LIBEVENT_LIBS)

catter_SOURCES = test/catter.c
catter_CPPFLAGS = $(PROGRAMS_CPPFLAGS)
catter_LDADD = $(PROGRAMS_LDADD)

sync_catter_SOURCES = test/sync_catter.c
sync_catter_CPPFLAGS = $(PROGRAMS_CPPFLAGS)
sync_catter_LDADD = $(PROGRAMS_LDADD)

# The http test requires some really new libevent interfaces.
if HAVE_LIBEVENT_209
noinst_PROGRAMS += server
server_SOURCES = test/server.c
server_CPPFLAGS = $(PROGRAMS_CPPFLAGS)
server_LDADD = $(PROGRAMS_LDADD)
else
TESTS_ENVIRONMENT = env NO_HTTP_TEST=1
endif

# The test driver uses Python.
if HAVE_PYTHON
TESTS = test/runtests.py
endif

endif


EXTRA_DIST = bootstrap.sh cleanup.sh test/runtests.py Doxyfile
