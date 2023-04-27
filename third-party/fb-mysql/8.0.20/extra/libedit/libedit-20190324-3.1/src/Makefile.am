
BUILT_SOURCES = vi.h emacs.h common.h fcns.h help.h func.h

AHDR= vi.h emacs.h common.h 
ASRC= $(srcdir)/vi.c $(srcdir)/emacs.c $(srcdir)/common.c

vi.h: Makefile $(srcdir)/vi.c
	AWK=$(AWK) sh $(srcdir)/makelist -h $(srcdir)/vi.c > $@

emacs.h: Makefile $(srcdir)/emacs.c
	AWK=$(AWK) sh $(srcdir)/makelist -h $(srcdir)/emacs.c > $@

common.h: Makefile $(srcdir)/common.c
	AWK=$(AWK) sh $(srcdir)/makelist -h $(srcdir)/common.c > $@

fcns.h: Makefile $(AHDR)
	AWK=$(AWK) sh $(srcdir)/makelist -fh $(AHDR) > $@

help.h: Makefile $(ASRC)
	AWK=$(AWK) sh $(srcdir)/makelist -bh $(ASRC) > $@

func.h: Makefile $(AHDR)
	AWK=$(AWK) sh $(srcdir)/makelist -fc $(AHDR) > $@

CLEANFILES = $(BUILT_SOURCES)

lib_LTLIBRARIES    = libedit.la
libedit_la_SOURCES = chared.c common.c el.c eln.c emacs.c hist.c keymacro.c map.c chartype.c parse.c \
							prompt.c read.c refresh.c search.c sig.c terminal.c tty.c vi.c \
							wcsdup.c \
							tokenizer.c tokenizern.c \
							history.c historyn.c \
							filecomplete.c readline.c chared.h literal.c el.h hist.h \
							histedit.h keymacro.h map.h chartype.h parse.h prompt.h read.h refresh.h \
							search.h sig.h sys.h terminal.h tty.h vis.h filecomplete.h \
							editline/readline.h literal.h

if !HAVE_STRLCPY
libedit_la_SOURCES += strlcpy.c
endif
if !HAVE_STRLCAT
libedit_la_SOURCES += strlcat.c
endif
if !HAVE_VIS
libedit_la_SOURCES += vis.c
endif
if !HAVE_UNVIS
libedit_la_SOURCES += unvis.c
endif

EXTRA_DIST = makelist shlib_version
nobase_include_HEADERS = histedit.h editline/readline.h

nodist_libedit_la_SOURCES = $(BUILT_SOURCES)

libedit_la_LDFLAGS = -no-undefined -version-info $(LT_VERSION)

