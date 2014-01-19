/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2013 Facebook, Inc. (http://www.facebook.com)     |
   +----------------------------------------------------------------------+
   | This source file is subject to version 3.01 of the PHP license,      |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | http://www.php.net/license/3_01.txt                                  |
   | If you did not receive a copy of the PHP license and are unable to   |
   | obtain it through the world-wide-web, please send a note to          |
   | license@php.net so we can mail you a copy immediately.               |
   +----------------------------------------------------------------------+
*/

#include "hphp/util/cache/cache-manager.h"

#include <string>

#include <gtest/gtest.h>

namespace HPHP {

using std::string;

class TestCacheManager : public testing::Test {
 protected:
  bool makeTempFile(const string& contents, string* fn) {
    char fn_tmpl[] = "/tmp/hhvm_unit_test.XXXXXX";
    int fd = mkstemp(fn_tmpl);

    if (fd < 0) {
      return false;
    }

    ssize_t ret = write(fd, contents.c_str(), contents.length());
    close(fd);

    if (ret != contents.length()) {
      return false;
    }

    *fn = fn_tmpl;
    return true;
  }

  bool makeTempDir(string* dir) {
    char dir_tmpl[] = "/tmp/hhvm_unit_test.XXXXXX";
    const char* temp = mkdtemp(dir_tmpl);

    if (temp == nullptr) {
      return false;
    }

    dir->assign(temp);
    return true;
  }
};

TEST_F(TestCacheManager, BadLoadPath) {
  CacheManager cm;
  EXPECT_FALSE(cm.addFileContents("/test/name", "/invalid/path"));
}

TEST_F(TestCacheManager, LocalStoreAndFetch) {
  CacheManager cm;

  string name = "/test/name";

  EXPECT_FALSE(cm.entryExists(name));
  EXPECT_FALSE(cm.fileExists(name));
  EXPECT_FALSE(cm.dirExists(name));

  const char* data;
  uint64_t data_len;
  bool compressed;

  EXPECT_FALSE(cm.getFileContents(name, &data, &data_len, &compressed));

  string uncompressed;
  EXPECT_FALSE(cm.getDecompressed(name, &uncompressed));

  uint64_t uncompressed_size;
  EXPECT_FALSE(cm.getUncompressedFileSize(name, &uncompressed_size));

  // Add a real (highly compressible) file for it to read in.

  string test_data;

  for (int i = 0; i < 1024; ++i) {
    test_data.append("123456789012345678901234567890");
  }

  string data_fn;
  ASSERT_TRUE(makeTempFile(test_data, &data_fn));

  ASSERT_TRUE(cm.addFileContents(name, data_fn));
  ASSERT_EQ(unlink(data_fn.c_str()), 0);

  EXPECT_TRUE(cm.entryExists(name));
  EXPECT_TRUE(cm.fileExists(name));
  EXPECT_FALSE(cm.dirExists(name));

  // The "/test" of "/test/name" should have been added as a directory.
  EXPECT_TRUE(cm.dirExists("/test"));

  EXPECT_TRUE(cm.getFileContents(name, &data, &data_len, &compressed));
  EXPECT_TRUE(compressed);

  // It's compressed, so it'll be different.
  ASSERT_NE(memcmp(test_data.c_str(), data, data_len), 0);

  ASSERT_TRUE(cm.getUncompressedFileSize(name, &uncompressed_size));
  EXPECT_EQ(test_data.length(), uncompressed_size);

  // So let's decompress it.
  ASSERT_TRUE(cm.getDecompressed(name, &uncompressed));

  ASSERT_EQ(test_data.length(), uncompressed.length());
  ASSERT_EQ(test_data, uncompressed);
}

TEST_F(TestCacheManager, Empty) {
  CacheManager cm;

  string name = "/test/name";

  EXPECT_FALSE(cm.entryExists(name));
  EXPECT_FALSE(cm.fileExists(name));
  EXPECT_FALSE(cm.dirExists(name));

  ASSERT_TRUE(cm.addEmptyEntry(name));

  EXPECT_TRUE(cm.entryExists(name));
  EXPECT_FALSE(cm.fileExists(name));
  EXPECT_FALSE(cm.dirExists(name));

  const char* data;
  uint64_t data_len;
  bool compressed;
  EXPECT_FALSE(cm.getFileContents(name, &data, &data_len, &compressed));

  string uncompressed;
  EXPECT_FALSE(cm.getDecompressed(name, &uncompressed));

  uint64_t file_size;
  EXPECT_FALSE(cm.getUncompressedFileSize(name, &file_size));
}

TEST_F(TestCacheManager, Duplicates) {
  CacheManager cm;

  string name = "/test/name";
  EXPECT_TRUE(cm.addEmptyEntry(name));

  EXPECT_FALSE(cm.addEmptyEntry(name));
  EXPECT_FALSE(cm.addFileContents(name, "/invalid/path"));
}

TEST_F(TestCacheManager, AddUncompressibleData) {
  string test_data = "let's hope this won't compress too terribly well.";
  string data_fn;
  ASSERT_TRUE(makeTempFile(test_data, &data_fn));

  CacheManager cm;
  string name = "/test/name";

  ASSERT_TRUE(cm.addFileContents(name, data_fn));
  ASSERT_EQ(unlink(data_fn.c_str()), 0);
  ASSERT_TRUE(cm.dirExists("/test"));

  // Read it back from the cache.

  const char* data;
  uint64_t data_len;
  bool compressed;

  EXPECT_TRUE(cm.getFileContents(name, &data, &data_len, &compressed));
  ASSERT_FALSE(compressed);

  EXPECT_EQ(data_len, test_data.length());
  ASSERT_EQ(memcmp(test_data.c_str(), data, data_len), 0);

  uint64_t file_size;
  ASSERT_TRUE(cm.getUncompressedFileSize(name, &file_size));
  EXPECT_EQ(file_size, test_data.length());
}

TEST_F(TestCacheManager, SaveAndLoad) {
  CacheManager cm;

  // Create an uncompressed file.

                      //1234567890123456
  string test_data_1 = "don't compress!!";
  string data_fn_1;
  ASSERT_TRUE(makeTempFile(test_data_1, &data_fn_1));
  string name_1 = "/test/name/not_compressible12345";

  ASSERT_TRUE(cm.addFileContents(name_1, data_fn_1));
  ASSERT_EQ(unlink(data_fn_1.c_str()), 0);

  ASSERT_TRUE(cm.dirExists("/test"));
  ASSERT_TRUE(cm.dirExists("/test/name"));

  // Create a compressed file.

  string test_data_2;
  test_data_2.assign(1048576, 0x55);
  string data_fn_2;

  ASSERT_TRUE(makeTempFile(test_data_2, &data_fn_2));
  string name_2 = "/test/name/very/compressible1234";

  ASSERT_TRUE(cm.addFileContents(name_2, data_fn_2));
  ASSERT_EQ(unlink(data_fn_2.c_str()), 0);

  ASSERT_TRUE(cm.dirExists("/test"));
  ASSERT_TRUE(cm.dirExists("/test/name"));
  ASSERT_TRUE(cm.dirExists("/test/name/very"));

  // Create an empty entry.

  string name_3 = "/test/with/a/very/long/name/for/an/empty/entry";
  ASSERT_TRUE(cm.addEmptyEntry(name_3));

  // Write it out ...

  string temp_dir;
  ASSERT_TRUE(makeTempDir(&temp_dir));

  string save_path(temp_dir);
  save_path.append("/cm.save");

  ASSERT_TRUE(cm.saveCache(save_path));

  // ... and load it back in.

  CacheManager cm2;
  ASSERT_TRUE(cm2.loadCache(save_path));

  ASSERT_TRUE(cm2.fileExists(name_1));
  ASSERT_TRUE(cm2.fileExists(name_2));
  ASSERT_TRUE(cm2.entryExists(name_3));

  EXPECT_TRUE(cm.dirExists("/test"));
  EXPECT_TRUE(cm.dirExists("/test/name"));

  // Empty entries also get their parent directories added.
  EXPECT_TRUE(cm.dirExists("/test/with"));

  const char* compare_1;
  uint64_t compare_len_1;
  bool compressed_1;
  ASSERT_TRUE(cm2.getFileContents(name_1, &compare_1, &compare_len_1,
                                  &compressed_1));

  ASSERT_FALSE(compressed_1);
  ASSERT_EQ(test_data_1.length(), compare_len_1);

  ASSERT_EQ(memcmp(test_data_1.c_str(), compare_1, compare_len_1), 0);

  string compare_2;
  ASSERT_TRUE(cm2.getDecompressed(name_2, &compare_2));
  EXPECT_EQ(compare_2, test_data_2);

  // Clean up the mess.
  ASSERT_EQ(unlink(save_path.c_str()), 0);
  ASSERT_EQ(rmdir(temp_dir.c_str()), 0);
}

// This is a great way to make mmap fail, since it hates a length of 0 bytes.
TEST_F(TestCacheManager, LoadEmptyCache) {
  string data;
  string fn;
  ASSERT_TRUE(makeTempFile(data, &fn));

  CacheManager cm;
  ASSERT_FALSE(cm.loadCache(fn));

  ASSERT_EQ(unlink(fn.c_str()), 0);
}

// Long enough to get past the "mmap punts at 0 bytes" thing, but
// short enough to trip an attempted read of the magic number.
TEST_F(TestCacheManager, LoadTruncatedCache) {
  string data = "X";
  string fn;
  ASSERT_TRUE(makeTempFile(data, &fn));

  CacheManager cm;
  ASSERT_FALSE(cm.loadCache(fn));

  ASSERT_EQ(unlink(fn.c_str()), 0);
}

// Long enough for the magic number read and check to happen (and fail).
TEST_F(TestCacheManager, LoadNonCache) {
  string data = "_________________nope____________________";
  string fn;
  ASSERT_TRUE(makeTempFile(data, &fn));

  CacheManager cm;
  ASSERT_FALSE(cm.loadCache(fn));

  ASSERT_EQ(unlink(fn.c_str()), 0);
}

TEST_F(TestCacheManager, SaveNothing) {
  string temp_dir;
  ASSERT_TRUE(makeTempDir(&temp_dir));

  string save_path(temp_dir);
  save_path.append("/cm.save");

  CacheManager cm;
  ASSERT_TRUE(cm.saveCache(save_path));

  ASSERT_EQ(unlink(save_path.c_str()), 0);
}

TEST_F(TestCacheManager, SaveBadPath) {
  CacheManager cm;
  ASSERT_FALSE(cm.saveCache("/__invalid__/__path__/"));
}

TEST_F(TestCacheManager, NoLeadingSlash) {
  CacheManager cm;
  ASSERT_TRUE(cm.addEmptyEntry("foo/bar/test.php"));

  EXPECT_TRUE(cm.dirExists("foo"));
  EXPECT_TRUE(cm.dirExists("foo/bar"));
  EXPECT_TRUE(cm.entryExists("foo/bar/test.php"));
}

}  // namespace HPHP
