# Setup the following Makefile variables:
# EXTERNAL_CC - path to the C compiler to use with this externals
# EXTERNAL_CXX - path to the C++ compiler to use with this externals
# EXTERNAL_SWIG - path to swig
# EXTERNAL_CPPFLAGS - Any preprocessor flag (like header file paths) to use
# EXTERNAL_LDFLAGS - Any extra linker flags
# EXTERNAL_STATIC_LIBS - Link string to use when performing a static link
# EXTERNAL_SHARED_LIBS - Link string to use when performing a shared link

EXTERNAL_CXX := $(if $(USE_LLVM), /data/llvm/bin/g++, $(if $(USE_ICC),$(ICC)/bin/icpc $(ICC_ARGS),g++))
EXTERNAL_CC = $(if $(USE_LLVM), /data/llvm/bin/gcc, $(if $(USE_ICC),$(ICC)/bin/icc $(ICC_ARGS),gcc))
EXTERNAL_SWIG = swig

ifndef NO_GOLD
LD_CMD += -B$(EXT_DIR)/binutils/
endif

ifdef MAC_OS_X

EXTERNAL_CPPFLAGS += \
  -I /usr/local/include \
  -I /usr/local/include/boost-1_37 \
  -I /usr/local/mysql/include \

else

EXTERNAL_CPPFLAGS += \
  -isystem $(EXT_DIR)/binutils \
  -isystem $(EXT_DIR)/pcre/include \
  -isystem $(EXT_DIR)/libevent/include \
  -isystem $(EXT_DIR)/libcurl/include \
  -isystem $(EXT_DIR)/libafdt/include \
  -isystem $(EXT_DIR)/gd/include \
  -isystem /usr/local/include \
  -isystem $(EXT_DIR)/boost/include/boost-1_37 \
  -isystem $(EXT_DIR)/mysql/include \
  -isystem $(EXT_DIR)/sqlite/include \
  -isystem $(EXT_DIR)/libxml2/include/libxml2 \
  -isystem $(EXT_DIR)/libfbml/include \
  -isystem $(EXT_DIR)/libmbfl/include \
  -isystem $(EXT_DIR)/oniguruma/include \
  -isystem $(EXT_DIR)/ldap/include \
  -isystem $(EXT_DIR)/oracle/include \
  -isystem $(ICU_INCLUDEDIR) \
  -isystem $(EXT_DIR)/xhp/include \
  -isystem $(EXT_DIR)/libmcc/include \
  -isystem $(EXT_DIR)/libch/include \
  -isystem $(EXT_DIR)/timelib/include \
  -isystem $(EXT_DIR)/tbb/include \
  -isystem $(EXT_DIR)/libmcrypt/include \
  -isystem $(EXT_DIR)/libfbi/include \
  -isystem $(EXT_DIR)/readline/include \
  -isystem $(EXT_DIR)/libmemcached/include \
  -isystem $(EXT_DIR)/jemalloc/include \
  -isystem $(EXT_DIR)/google-perftools/include \
  -isystem $(EXT_DIR)/libcap/include \
  -isystem $(EXT_DIR)/libpng/include \
  -isystem $(EXT_DIR)/imap/include \
  -isystem $(EXT_DIR)/zlib/include \
  -isystem $(EXT_DIR)/snappy/include \

endif


# Linking

ifndef NO_RPATH
EXTERNAL_LDFLAGS += -Wl,-rpath -Wl,/usr/local/hphp/lib
endif

EXTERNAL_LDFLAGS	+= \
  -L/usr/local/lib

ifdef MAC_OS_X
LINK_LIBS = -lpthread -lstdc++ -ldl
else
BFD_LIBS = -L$(EXT_DIR)/binutils/ -lbfd -liberty -ldl
LINK_LIBS = -lpthread $(BFD_LIBS) -lrt -lstdc++ -lresolv -lbz2
endif

# 2. Common Libraries
#
# Common but not essential.

ifeq ($(GCC_VERSION), 4.4.0)
BOOST_LIBS = \
	$(EXT_DIR)/boost/lib/libboost_program_options-gcc44-mt.a \
	$(EXT_DIR)/boost/lib/libboost_filesystem-gcc44-mt.a \
	$(EXT_DIR)/boost/lib/libboost_system-gcc44-mt.a \

else
BOOST_LIBS = \
	$(EXT_DIR)/boost/lib/libboost_program_options-gcc40-mt.a \
	$(EXT_DIR)/boost/lib/libboost_filesystem-gcc40-mt.a \
	$(EXT_DIR)/boost/lib/libboost_system-gcc40-mt.a \

endif

ifdef MAC_OS_X

MYSQL_LIBS = -L/usr/local/mysql/lib -lmysqlclient

else

MYSQL_LIBS = $(EXT_DIR)/mysql/lib/mysql/libmysqlclient_r.a \
	-lssl -lcrypto -lcrypt
ORACLE_LIBS = $(EXT_DIR)/oracle/lib/libclntsh.so.11.1

SQLITE_LIBS = $(EXT_DIR)/sqlite/lib/libsqlite3.a

PCRE_LIBS = $(EXT_DIR)/pcre/lib/libpcre.a

HTTP_LIBS = $(EXT_DIR)/libafdt/lib/libafdt.a $(EXT_DIR)/libevent/lib/libevent.a

MCC_LIBS = $(EXT_DIR)/libmcc/lib/libmcc.a $(EXT_DIR)/libch/lib/libch.a \
	$(EXT_DIR)/libevent/lib/libevent.a

LIBMEMCACHED_LIBS = $(EXT_DIR)/libmemcached/lib/libmemcached.a

LIBCAP_LIBS = $(EXT_DIR)/libcap/lib64/libcap.a

IMAP_LIBS = $(EXT_DIR)/imap/lib/c-client.a -lpam -lkrb5

EXTERNAL_LIBJPEG_LIBS = -ljpeg

GD_LIBS = $(EXT_DIR)/gd/lib/libgd.a $(EXT_DIR)/libpng/lib/libpng.a \
	$(EXTERNAL_LIBJPEG_LIBS) -lfreetype -lfontconfig

MOZILLA_LIBS = $(EXT_DIR)/mozilla/libmozutil_s.a \
               $(EXT_DIR)/mozilla/libexpat_s.a \
               $(EXT_DIR)/mozilla/libsaxp.a \
               $(EXT_DIR)/mozilla/libunicharutil_s.a \
               $(EXT_DIR)/mozilla/libxptcmd.a \
               $(EXT_DIR)/mozilla/libxptcall.a \
               $(EXT_DIR)/mozilla/libxptinfo.a \
               $(EXT_DIR)/mozilla/libxpt.a \
               $(EXT_DIR)/mozilla/libxpcomcomponents_s.a \
               $(EXT_DIR)/mozilla/libxpcomproxy_s.a \
               $(EXT_DIR)/mozilla/libxpcomio_s.a \
               $(EXT_DIR)/mozilla/libxpcomds_s.a \
               $(EXT_DIR)/mozilla/libxpcomglue.a \
               $(EXT_DIR)/mozilla/libxpcombase_s.a \
               $(EXT_DIR)/mozilla/libxpcomthreads_s.a \
               $(EXT_DIR)/mozilla/libstring_s.a \
               $(EXT_DIR)/mozilla/libplc4.a \
               $(EXT_DIR)/mozilla/libplds4.a \
               $(EXT_DIR)/mozilla/libnspr4.a

CURL_LIBS = $(EXT_DIR)/libcurl/lib/libcurl.a

LIBXML_LIBS = $(EXT_DIR)/libxml2/lib/libxml2.a -lexpat

TIME_LIBS = $(EXT_DIR)/timelib/lib/libtimelib.a

ifdef DEBUG
FBML_LIBS = $(EXT_DIR)/libfbml/lib/libfbml-debug.a $(MOZILLA_LIBS)
else
FBML_LIBS = $(EXT_DIR)/libfbml/lib/libfbml.a $(MOZILLA_LIBS)
endif

MBFL_LIBS = $(EXT_DIR)/libmbfl/lib/libmbfl.a \
	$(EXT_DIR)/oniguruma/lib/libonig.a

LIB_UNWIND = $(EXT_DIR)/libunwind/lib/libunwind.a

ifdef USE_JEMALLOC
JEMALLOC_LIBS = $(EXT_DIR)/jemalloc/lib/libjemalloc.a
endif

ifdef GOOGLE_HEAP_PROFILER
GOOGLE_LIBS = $(EXT_DIR)/google-perftools/lib/libprofiler.a \
	$(EXT_DIR)/google-perftools/lib/libtcmalloc.a $(LIB_UNWIND)
else
GOOGLE_LIBS =
ifdef GOOGLE_CPU_PROFILER
GOOGLE_LIBS = $(EXT_DIR)/google-perftools/lib/libprofiler.a $(LIB_UNWIND)
endif
ifdef GOOGLE_TCMALLOC
GOOGLE_LIBS += $(EXT_DIR)/google-perftools/lib/libtcmalloc_minimal.a \
	$(LIB_UNWIND)
endif
endif

ICU_VERSION = 4_6
ICU_LIBDIR = $(EXT_DIR)/icu-$(ICU_VERSION)/lib
ICU_INCLUDEDIR = $(EXT_DIR)/icu-$(ICU_VERSION)/include
ICU_LIBS = \
	$(ICU_LIBDIR)/libicui18n.a \
	$(ICU_LIBDIR)/libicuuc.a \
	$(ICU_LIBDIR)/libicudata.a \

endif

XHP_LIBS = $(EXT_DIR)/xhp/lib/libxhp.a

TBB_LIBS = -L$(EXT_DIR)/tbb/lib/ $(EXT_DIR)/tbb/lib/libtbb.a

MCRYPT_LIBS = $(EXT_DIR)/libmcrypt/lib/libmcrypt.a

FBI_LIBS = $(EXT_DIR)/libfbi/lib/libfbi.a

LDAP_LIBS = $(EXT_DIR)/ldap/lib/libldap.a $(EXT_DIR)/ldap/lib/liblber.a

READLINE_LIBS = $(EXT_DIR)/readline/lib/libreadline.a \
	$(EXT_DIR)/readline/lib/libhistory.a -lcurses

ZLIB_LIBS = $(EXT_DIR)/zlib/lib/libz.a

ifdef HAVE_SNAPPY
SNAPPY_LIBS = $(EXT_DIR)/snappy/lib/libsnappy.a
endif

EXTERNAL_STATIC_LIBS = $(CURL_LIBS) $(PCRE_LIBS) $(BOOST_LIBS) \
	$(MYSQL_LIBS) $(SQLITE_LIBS) $(MCC_LIBS) \
	$(GD_LIBS) $(LIBXML_LIBS) $(FBML_LIBS) $(MBFL_LIBS) \
	$(MCRYPT_LIBS) $(JEMALLOC_LIBS) $(GOOGLE_LIBS) $(ICU_LIBS) \
	$(HTTP_LIBS) $(XHP_LIBS) $(TIME_LIBS) $(TBB_LIBS) $(FBI_LIBS) \
	$(LDAP_LIBS) $(READLINE_LIBS) $(LIBMEMCACHED_LIBS) \
	$(LIBCAP_LIBS) $(IMAP_LIBS) $(ORACLE_LIBS) $(ZLIB_LIBS) \
	$(SNAPPY_LIBS)

EXTERNAL_LIB_PATHS = \
  /usr/local/hphp/lib \
  $(EXT_DIR)/binutils \
  $(sort $(foreach L,$(filter-out -%, $(EXTERNAL_STATIC_LIBS)), $(dir $(L))))


# Create the list of shared libraries from the externals tree

EXCEPTIONS := $(EXT_DIR)/mozilla/%.a $(EXT_DIR)/imap/lib/c-client.a \
              $(EXT_DIR)/google-perftools/lib/libtcmalloc.a \
              $(EXT_DIR)/google-perftools/lib/libtcmalloc_minimal.a \
              $(EXT_DIR)/jemalloc/lib/libjemalloc.a \

STATIC_LIBS = $(filter-out $(EXCEPTIONS), $(EXTERNAL_STATIC_LIBS))

SHARED_LIBS = $(EXT_DIR)/binutils/libbinutils.so \
              $(EXT_DIR)/imap/lib/libc-client.so.2007 \
              $(shell find $(EXT_DIR)/mozilla -name "*.so") \
              $(patsubst %.a, %.so, $(STATIC_LIBS)) \
              $(LINK_LIBS) \

EXTERNAL_SHARED_LIBS = $(SHARED_LIBS)
