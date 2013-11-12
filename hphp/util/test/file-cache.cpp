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

#include "hphp/util/file-cache.h"

#include <gtest/gtest.h>

namespace HPHP {

static const char* kTestData = "some test data for serialization";

class TestFileCache : public testing::Test {
 protected:
  virtual void SetUp() {
    FileCache::UseNewCache = true;
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

TEST_F(TestFileCache, SourceRoot) {
  FileCache fc;

  FileCache::SourceRoot = "/sr";
  EXPECT_EQ(fc.GetRelativePath("/sr/1234"), "/1234");
  EXPECT_EQ(fc.GetRelativePath("/sr/1234/"), "/1234");
  EXPECT_EQ(fc.GetRelativePath("/invalid/abcd"), "/invalid/abcd");
  EXPECT_EQ(fc.GetRelativePath(""), "");
  EXPECT_EQ(fc.GetRelativePath("/x"), "/x");
  EXPECT_EQ(fc.GetRelativePath("//"), "/");

  FileCache::SourceRoot.clear();
  EXPECT_EQ(fc.GetRelativePath("/sr/1234"), "/sr/1234");
  EXPECT_EQ(fc.GetRelativePath("/sr/1234/"), "/sr/1234");
  EXPECT_EQ(fc.GetRelativePath("/invalid/abcd"), "/invalid/abcd");
  EXPECT_EQ(fc.GetRelativePath(""), "");
  EXPECT_EQ(fc.GetRelativePath("/x"), "/x");
  EXPECT_EQ(fc.GetRelativePath("//"), "/");
}

TEST_F(TestFileCache, WriteAndReadBack) {
  // Set up something for FileCache to read in.

  char data_fn[] = "/tmp/hhvm_unit_test-testdata.XXXXXX";
  int data_fd = mkstemp(data_fn);
  ASSERT_GT(data_fd, 0);

  FILE* f = fdopen(data_fd, "w");
  ASSERT_TRUE(f != nullptr);

  fprintf(f, "%s", kTestData);
  fclose(f);

  // Set up a cache and put this data file in it.

  FileCache fc;
  fc.write("_unit_test_one_", data_fn);
  fc.write("_unit_test_two_");
  fc.write("/__invalid__/path/with/directories");

  string temp_dir;
  ASSERT_TRUE(makeTempDir(&temp_dir));

  string cache_fn(temp_dir);
  cache_fn.append("/cache.dump");

  // Flush to disk.

  fc.save(cache_fn.c_str());

  // Read back into another cache.

  FileCache fc2;
  fc2.loadMmap(cache_fn.c_str());

  EXPECT_TRUE(fc2.fileExists("_unit_test_one_"));

  int read_len;
  bool compressed = false;
  const char* read_data = fc2.read("_unit_test_one_", read_len, compressed);

  EXPECT_STREQ(kTestData, read_data);
  EXPECT_EQ(fc2.fileSize("_unit_test_one_", false), strlen(kTestData));

  EXPECT_TRUE(fc2.fileExists("_unit_test_two_"));
  EXPECT_FALSE(fc2.fileExists("_unit_test_three_"));

  EXPECT_TRUE(fc2.dirExists("/__invalid__"));
  EXPECT_TRUE(fc2.dirExists("/__invalid__/path"));
  EXPECT_TRUE(fc2.dirExists("/__invalid__/path/with"));
  EXPECT_TRUE(fc2.fileExists("/__invalid__/path/with/directories"));

  // -1 is a magic value... here it means "it's a PHP file"...
  EXPECT_EQ(fc2.fileSize("unit_test_two_", false), -1);

  // ... and here it means "this thing does not exist".
  EXPECT_EQ(fc2.fileSize("unit_test_three_", false), -1);

  fc2.dump();

  // Clean up the mess.

  ASSERT_EQ(unlink(cache_fn.c_str()), 0);
  ASSERT_EQ(rmdir(temp_dir.c_str()), 0);
  ASSERT_EQ(unlink(data_fn), 0);
}

TEST_F(TestFileCache, HighlyCompressibleData) {
  char data_fn[] = "/tmp/hhvm_unit_test-testdata.XXXXXX";
  int data_fd = mkstemp(data_fn);
  ASSERT_GT(data_fd, 0);

  FILE* f = fdopen(data_fd, "w");
  ASSERT_TRUE(f != nullptr);

  string test_path = "/path/to/data";
  string test_data;

  for (int i = 0; i < 10; ++i) {
    test_data.append("AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA");
  }

  fprintf(f, "%s", test_data.c_str());
  fclose(f);

  FileCache fc;
  fc.write(test_path.c_str(), data_fn);

  // Flush to disk.

  string temp_dir;
  ASSERT_TRUE(makeTempDir(&temp_dir));

  string cache_fn(temp_dir);
  cache_fn.append("/cache.dump");

  fc.save(cache_fn.c_str());

  // Read back into another cache.

  int read_len;
  bool compressed;
  const char* read_data;

  FileCache fc3;
  fc3.loadMmap(cache_fn.c_str());
  fc3.dump();

  ASSERT_TRUE(fc3.fileExists(test_path.c_str()));

  // Ask for uncompressed data (a holdover from the original API).
  compressed = false;
  read_data = fc3.read(test_path.c_str(), read_len, compressed);

  // FileCache::read() takes compressed as a non-const reference (!)
  // and changes the value according to what it found...

  // ... so this means "I just gave you compressed data".
  EXPECT_TRUE(compressed);

  // ... so these can't match.
  EXPECT_NE(test_data, read_data);
  EXPECT_NE(test_data.length(), read_len);

  // But this always gets the uncompressed size no matter what.
  EXPECT_EQ(test_data.length(), fc3.fileSize(test_path.c_str(), false));

  // So now let's actually ask for compressed data this time.
  compressed = true;
  read_data = fc3.read(test_path.c_str(), read_len, compressed);

  // Same conditions should hold.
  EXPECT_TRUE(compressed);
  EXPECT_NE(test_data, read_data);
  EXPECT_EQ(test_data.length(), fc3.fileSize(test_path.c_str(), false));

  // Clean up the mess.

  ASSERT_EQ(unlink(cache_fn.c_str()), 0);
  ASSERT_EQ(rmdir(temp_dir.c_str()), 0);
  ASSERT_EQ(unlink(data_fn), 0);
}

}  // namespace HPHP
