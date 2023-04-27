/* Copyright (c) 2018, 2019, Oracle and/or its affiliates. All rights reserved.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License, version 2.0,
   as published by the Free Software Foundation.

   This program is also distributed with certain software (including
   but not limited to OpenSSL) that is licensed under separate terms,
   as designated in a particular file or component or in included license
   documentation.  The authors of MySQL hereby grant you an additional
   permission to link the program and your derivative works with the
   separately licensed software that they have included with MySQL.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License, version 2.0, for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA */

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <stddef.h>
#include <algorithm>
#include <iterator>

#include "my_inttypes.h"
#include "my_io.h"
#include "my_sys.h"

// Check that various mysys path functions produce a valid
// ('\0'-terminated) c-string, do not write more than FN_REFLEN bytes
// into the destination buffer.

using ::testing::MatchesRegex;
using ::testing::StartsWith;

namespace mysys_pathfuncs {
char dest[FN_REFLEN];
// Assigns values to a T[SZ] array with a value s of type S which must be
// static_cast'able to T. The array argument a is passed as a reference to an
// array of T[SZ]. The default value for argument s is the default
// constructor for S, (0 for integer types).
template <typename T, size_t SZ, typename S>
void aset(T (&a)[SZ], S s = S()) {
  std::fill(std::begin(a), std::end(a), static_cast<T>(s));
}

// Sets up dst array of size SZ and a src array of size SZ + 10 on the stack
// and invokes the testcase tc argument with these arrays (decayed to
// pointers) and their sizes. The type TC must be a callable type
// implementing void operator()(char *, size_t, const char*. size_t),
// (typically a lambda).
// Dst buffer initialized to 0xde. Src buffer set to aaa... terminated with
// '\0'. Verifies that testcase has filled the whole dst buffer and placed a
// '\0' at the end.
template <size_t SZ, typename TC>
void null_term_setup(TC &&tc) {
  char dst[SZ];
  aset(dst, 0xde);
  char src[SZ + 10];
  aset(src, 'a');
  src[sizeof(src) - 1] = '\0';
  tc(dst, sizeof(dst), src, sizeof(src));
  constexpr size_t dlen = sizeof(dst) - 1;
  EXPECT_EQ('\0', dst[dlen]);
  EXPECT_NE(static_cast<char>(0xde), dst[dlen - 1]);
}

TEST(Mysys, CleanupDirnameOverflow) {
  null_term_setup<FN_REFLEN>(
      [](char *d, size_t, const char *s, size_t) { cleanup_dirname(d, s); });
}

TEST(Mysys, NormalizeDirnameOverflow) {
  null_term_setup<FN_REFLEN>(
      [](char *d, size_t, const char *s, size_t) { normalize_dirname(d, s); });
}

TEST(Mysys, UnpackDirnameOverflow) {
  null_term_setup<FN_REFLEN>(
      [](char *d, size_t, const char *s, size_t) { unpack_dirname(d, s); });
}

TEST(Mysys, UnpackFilenameOverflow) {
  null_term_setup<FN_REFLEN>(
      [](char *d, size_t, const char *s, size_t) { unpack_filename(d, s); });
}

TEST(Mysys, SystemFilenameOverflow) {
  null_term_setup<FN_REFLEN>(
      [](char *d, size_t, const char *s, size_t) { system_filename(d, s); });
}

TEST(Mysys, InternFilenameOverflow) {
  null_term_setup<FN_REFLEN>(
      [](char *d, size_t, const char *s, size_t) { intern_filename(d, s); });
}

TEST(Mysys, DirnamePartOverflow) {
  char dst[FN_REFLEN];
  aset(dst, 0xaa);

  char src[FN_REFLEN + 5];
  aset(src, 'a');
  src[sizeof(src) - 1] = '\0';
  src[sizeof(src) - 3] = FN_LIBCHAR;
  size_t arglen = 0;
  constexpr size_t dlen = sizeof(dst) - 1;

  const size_t rlen = dirname_part(dst, src, &arglen);
  EXPECT_EQ('\0', dst[dlen]);
  EXPECT_EQ(dlen, arglen);
  EXPECT_EQ(sizeof(src) - 2, rlen);
}

TEST(Mysys, ConvertDirnameOverflow) {
  null_term_setup<FN_REFLEN>([](char *d, size_t, const char *s, size_t sl) {
    convert_dirname(d, s, s + sl);
  });
}

TEST(Mysys, LoadPathNoPrefixOverflow) {
  null_term_setup<FN_REFLEN>([](char *d, size_t, const char *s, size_t) {
    my_load_path(d, s, nullptr);
  });
}

TEST(Mysys, LoadPathOverflow) {
  char dst[FN_REFLEN];
  aset(dst, 0xaa);

  char prefix[FN_REFLEN + 5];
  aset(prefix, 'a');
  prefix[sizeof(prefix) - 1] = '\0';
  EXPECT_EQ(dst, my_load_path(dst, "123", prefix));
  EXPECT_EQ('\0', dst[FN_REFLEN - 1]);
  EXPECT_EQ('a', dst[FN_REFLEN - 2]);
}

#ifdef HAVE_O_TMPFILE
TEST(Mysys, CreateTempFile) {
  char dst[FN_REFLEN];
  aset(dst, 0xaa);

  char prefix[FN_REFLEN + 5];
  aset(prefix, 'a');
  prefix[sizeof(prefix) - 1] = '\0';

  File fileno = create_temp_file(dst, "/tmp", prefix, 42, UNLINK_FILE, 0);
  EXPECT_GE(fileno, 0);
  my_close(fileno, 0);
  // Regex needs to match Windows tempfile names too
  EXPECT_THAT(dst, MatchesRegex("/tmp/[a]+(fd=[0-9]+|[a-zA-Z0-9]+)"));
  aset(dst, 0xaa);

  char *env_tmpdir = getenv("TMPDIR");
  fileno = create_temp_file(dst, nullptr, prefix, 42, UNLINK_FILE, 0);
  EXPECT_GE(fileno, 0);
  if (env_tmpdir != nullptr) {
    EXPECT_THAT(dst, StartsWith(env_tmpdir));
  } else {
    EXPECT_THAT(dst, StartsWith("/tmp"));
  }
  my_close(fileno, 0);
  aset(dst, 0xaa);

  char longdirname[FN_REFLEN];
  aset(longdirname, 'x');
  longdirname[0] = '/';
  fileno = create_temp_file(dst, longdirname, "hello", 42, UNLINK_FILE, 0);
  EXPECT_LT(fileno, 0);
  EXPECT_EQ(errno, ENAMETOOLONG);
}
#endif  // HAVE_O_TMPFILE

// Verify that unpack_dirname works correctly with ~/ and ~user
TEST(Mysys, UnpackDirname) {
  char dst[FN_REFLEN];
  aset(dst, 0xaa);

  char src[FN_REFLEN + 5];
  aset(src, 'a');
  std::fill_n(src + FN_REFLEN, 4, 'b');
  src[FN_REFLEN + 4] = '\0';

  // Verify that destination array does not overflow when source is larger
  unpack_dirname(dst, src);
  EXPECT_EQ('\0', dst[FN_REFLEN - 1]);
  EXPECT_EQ(FN_LIBCHAR, dst[FN_REFLEN - 2]);
  EXPECT_EQ('a', dst[FN_REFLEN - 3]);

  aset(dst, 0xaa);
  unpack_dirname(dst, "/an/absolute/path");
  EXPECT_STREQ(
      FN_ROOTDIR "an" FN_ROOTDIR "absolute" FN_ROOTDIR "path" FN_ROOTDIR, dst);

  aset(dst, 0xaa);
  unpack_dirname(dst, "a/relative/path");
  EXPECT_STREQ("a" FN_ROOTDIR "relative" FN_ROOTDIR "path" FN_ROOTDIR, dst);

  // Verify that ~ is expanded to home_dir+/
  // If home_dir is not set (WIN32) tilde expansion does not happen.
  std::string hd{home_dir ? home_dir : "~"};
  hd.append(1, FN_LIBCHAR);
  aset(dst, 0xaa);
  unpack_dirname(dst, "~");
  EXPECT_EQ(hd, std::string{dst});

  // Verify that /~ is expanded to home_dir+/
  aset(dst, 0xaa);
  unpack_dirname(dst, "~/");
  EXPECT_EQ(hd, std::string{dst});

  // Verify that ~root is expanded to somthing starting with / and which
  // contains /dir (exact name of root's home dir varies between platforms)
  aset(dst, 0xaa);
  unpack_dirname(dst, "~root/dir");
#ifdef HAVE_GETPWNAM
  EXPECT_EQ('/', dst[0]);
  EXPECT_LE(dst, strstr(dst, "/dir"));
#else
  // On platforms which do not have getpwnam no expansion of ~user is performed
  EXPECT_STREQ("~root" FN_ROOTDIR "dir" FN_ROOTDIR, dst);
#endif
  // Verify that ~ is not expanded when the user does not exist
  unpack_dirname(dst, "~___/dir");
  EXPECT_STREQ("~___" FN_ROOTDIR "dir" FN_ROOTDIR, dst);
}

}  // namespace mysys_pathfuncs
