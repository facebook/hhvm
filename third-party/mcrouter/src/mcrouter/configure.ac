# Copyright (c) Facebook, Inc. and its affiliates.
#
# This source code is licensed under the MIT license found in the
# LICENSE file in the root directory of this source tree.

# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.69)

m4_define([mcrouter_version_str], m4_esyscmd_s([cut -d '-' -f 1 VERSION]))
m4_define([mcrouter_version_suffix], m4_esyscmd_s([sed 's/^[0-9]*:[0-9]*//' VERSION]))
m4_define([mcrouter_version], m4_translit(mcrouter_version_str, [:], [.]))
m4_append([mcrouter_version], [.0])
m4_append([mcrouter_version], [mcrouter_version_suffix])
AC_INIT([mcrouter], [mcrouter_version], mcrouter@fb.com)

AC_ARG_VAR([FBTHRIFT_BIN], [Path to FBThrift compiler])
AC_CHECK_PROG(have_fbthrift, thrift1, yes, no, path = $FBTHRIFT_BIN)
AS_IF(
  [test "x$have_fbthrift" = "xyes"],
  [FBTHRIFT="$FBTHRIFT_BIN/thrift1 "],
  [AC_MSG_ERROR([Unable to find FBThrift Compiler])]
)
AC_SUBST([FBTHRIFT])

AC_SUBST([INSTALL_DIR], [$INSTALL_DIR])

AC_DEFINE_UNQUOTED([MCROUTER_PACKAGE_STRING], ["AC_PACKAGE_VERSION AC_PACKAGE_NAME"], [Full name and version of this package, with version coming first for libmemcached compatibility])

AC_CONFIG_SRCDIR([CarbonRouterInstance.h])
AC_CONFIG_HEADERS([config.h:config.hin])
AC_CONFIG_LINKS([config-impl.h:mcrouter_config-impl.h])
AC_CONFIG_LINKS([test/config.py:test/mcrouter_config.py])
AC_CONFIG_LINKS([RouterRegistry.h:RouterRegistry-impl.h])
AC_CONFIG_LINKS([ThriftAcceptor.h:ThriftAcceptor-impl.h])
AC_CONFIG_LINKS([mcrouter_sr_deps.h:mcrouter_sr_deps-impl.h])
AC_CONFIG_LINKS([HostWithShard-fwd.h:HostWithShard-fwd-impl.h])
AC_CONFIG_AUX_DIR([build-aux])

AM_INIT_AUTOMAKE([1.14 foreign dist-bzip2 nostdinc subdir-objects parallel-tests])

AC_CONFIG_MACRO_DIR([m4])

AC_PROG_INSTALL
LT_INIT

AC_LANG([C++])

# Checks for programs.
AC_PROG_CXX
AC_PROG_CC
AC_PROG_CPP
AX_PYTHON_DEVEL()
AX_CXX_COMPILE_STDCXX_14([],[mandatory])
LT_INIT

CXXFLAGS="-fno-strict-aliasing -std=c++17 $CXXFLAGS"
CXXFLAGS="-W -Wall -Wextra -Wno-unused-parameter $CXXFLAGS"
CXXFLAGS=" -Wno-missing-field-initializers -Wno-deprecated $CXXFLAGS"
CXXFLAGS="-DLIBMC_FBTRACE_DISABLE -DDISABLE_COMPRESSION $CXXFLAGS"

CFLAGS="-DLIBMC_FBTRACE_DISABLE -DDISABLE_COMPRESSION $CFLAGS"

# Checks for glog and gflags
# There are no symbols with C linkage, so we do a try-run
AC_HAVE_LIBRARY([glog],[],[AC_MSG_ERROR(
                [Please install google-glog library])])
AC_CACHE_CHECK(
  [for glog viability],
  [mcrouter_cv_prog_cc_glog],
  [AC_RUN_IFELSE(
    [AC_LANG_SOURCE[
      #include <glog/logging.h>
      int main(int argc, char** argv) {
        google::InitGoogleLogging(argv[0]);
        google::ShutdownGoogleLogging();
        return 0;
      }
    ]],
    [mcrouter_cv_prog_cc_glog=yes],
    [mcrouter_cv_prog_cc_glog=no]
  )]
)

if test "$mcrouter_cv_prog_cc_glog" != "yes"; then
  AC_MSG_ERROR(["libglog invalid, see config.log for details"])
fi

AC_HAVE_LIBRARY([gflags],[],[AC_MSG_ERROR(
                [Please install google-gflags library])])
AC_CACHE_CHECK(
  [for gflags viability],
  [mcrouter_cv_prog_cc_gflags],
  [AC_RUN_IFELSE(
    [AC_LANG_SOURCE[
      #include <gflags/gflags.h>
      DEFINE_bool(mcrouter_truthy, true, "Sample truthy flag");
      DEFINE_bool(mcrouter_falsey, false, "Sample falsey flag");
      int main(int argc, char** argv) {
        return (FLAGS_mcrouter_truthy && !FLAGS_mcrouter_falsey) ? 0 : 1;
      }
    ]],
    [mcrouter_cv_prog_cc_gflags=yes],
    [mcrouter_cv_prog_cc_gflags=no]
  )]
)

if test "$mcrouter_cv_prog_cc_gflags" != "yes"; then
  AC_MSG_ERROR(["libgflags invalid, see config.log for details"])
fi

# check for boost libs
AX_BOOST_BASE([1.65.1], [], [AC_MSG_ERROR(
              [Please install boost >= 1.65.1 (context, filesystem, program_options, regex and system)])])
# TODO: also check boost::ctx, not only boost::context
AX_BOOST_CONTEXT
AX_BOOST_FILESYSTEM
AX_BOOST_PROGRAM_OPTIONS
AX_BOOST_REGEX
AX_BOOST_SYSTEM
AX_BOOST_THREAD

# Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS([arpa/inet.h fcntl.h limits.h netdb.h stdint.h string.h sys/file.h sys/time.h syslog.h unistd.h malloc.h])

AC_CHECK_LIB([event], [event_set], [], [AC_MSG_ERROR([Unable to find libevent])])
AC_CHECK_LIB([crypto], [MD5_Init], [], [AC_MSG_ERROR([Unable to find libcrypto])])
AC_CHECK_LIB([ssl], [SSL_CTX_new], [], [AC_MSG_ERROR([Unable to find ssl])])
AC_CHECK_LIB([z], [gzread], [], [AC_MSG_ERROR([Unable to find zlib])])
AC_CHECK_LIB([double-conversion],[ceil],[],[AC_MSG_ERROR(
             [Please install double-conversion library])])
AC_CHECK_LIB([dl], [dlopen], [])
AC_CHECK_LIB([iberty], [cplus_demangle_v3_callback], [])
AC_CHECK_LIB([folly],[getenv],[],[AC_MSG_ERROR(
             [Please install the folly library])])
AC_CHECK_LIB([sodium],[sodium_init],[],[AC_MSG_ERROR(
             [Please install the libsodium library])])
AC_CHECK_LIB([fizz],[getenv],[],[AC_MSG_ERROR(
             [Please install the fizz library])])
AC_CHECK_HEADER([folly/Likely.h], [], [AC_MSG_ERROR(
                [Could not find folly, please download from https://github.com/facebook/folly])], [])

# Commenting out the wangle dependency from here, because there is a tricky
# inter-library dependency that resolves with the right order of LDFLAGS, and
# this dependency here messes the order.
#AC_CHECK_LIB([wangle], [getenv], [], [AC_MSG_ERROR(
#             [Please install the wangle library])])

PKG_CHECK_MODULES([GTEST], [gtest], [], [
  # gtest-devel shipped by CentOS doesn't include a .pc file, so let's fallback
  # to using normal library and header checks and just set defaults for the
  # GTEST_* variables.
  #
  # The libgtest-dev package in Ubuntu 18.04 is worse, it doesn't even ship
  # a built library, and only sources... Update the Ubuntu scripts to build
  # it from source on that platform.
  AC_CHECK_LIB([gtest], [main], [GTEST_LIBS="-lgtest"], [
    AC_MSG_ERROR([Please install the gtest library])
  ])
  AC_CHECK_HEADER([gtest/gtest.h], [], [
    AC_MSG_ERROR([Please install the gtest library])
  ])
])

# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STDBOOL
AC_C_CONST
AC_C_INLINE
AC_TYPE_SIZE_T
AC_HEADER_TIME
AC_C_VOLATILE
AC_FUNC_FORK
AC_TYPE_INT32_T
AC_TYPE_INT64_T
AC_TYPE_MODE_T
AC_TYPE_PID_T
AC_TYPE_SSIZE_T
AC_TYPE_UINT16_T
AC_TYPE_UINT32_T
AC_TYPE_UINT64_T

AC_CHECK_TYPES([ptrdiff_t])

# Checks for library functions.
AC_CHECK_FUNCS([gettimeofday \
                localtime_r \
                memchr \
                memset \
                mkdir \
                socket \
                strcasecmp \
                strdup \
                strerror \
                strtol \
                dup2 \
                ftruncate])

LIBS="$LIBS $BOOST_LDFLAGS $BOOST_CONTEXT_LIB $BOOST_FILESYSTEM_LIB \
      $BOOST_PROGRAM_OPTIONS_LIB $BOOST_SYSTEM_LIB $BOOST_REGEX_LIB \
      $BOOST_THREAD_LIB -lpthread -pthread -ldl -lunwind \
      -lbz2 -llz4 -llzma -lsnappy -lzstd"

AM_PATH_PYTHON([2.6],, [:])
AM_CONDITIONAL([HAVE_PYTHON], [test "$PYTHON" != :])

AH_BOTTOM([#include "mcrouter_config.h"])
AH_BOTTOM([#include "RouterRegistry.h"])

# Output
AC_CONFIG_FILES([Makefile
                 lib/Makefile
                 lib/test/Makefile
                 lib/carbon/Makefile
                 lib/config/Makefile
                 lib/config/test/Makefile
                 lib/fbi/Makefile
                 lib/fbi/test/Makefile
                 lib/fbi/cpp/Makefile
                 lib/fbi/cpp/test/Makefile
                 lib/network/gen/Makefile
                 lib/network/test/Makefile
                 routes/Makefile
                 routes/test/Makefile
                 test/Makefile
                 test/cpp_unit_tests/Makefile
                 tools/Makefile
                 tools/mcpiper/Makefile])

AC_OUTPUT
