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

#include "hphp/util/cache/cache-type.h"

#include <string>

#include "hphp/util/cache/cache-manager.h"
#include "hphp/util/file-cache.h"
#include "gtest/gtest.h"

namespace HPHP {

using std::string;

class TestCacheType : public testing::Test {
 protected:
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

TEST_F(TestCacheType, OldCache) {
  FileCache fc;
  fc.write("test_entry", false);

  string temp_dir;
  ASSERT_TRUE(makeTempDir(&temp_dir));

  string cache_fn(temp_dir);
  cache_fn.append("/cache.dump");

  fc.save(cache_fn.c_str());

  CacheType ct;
  ASSERT_FALSE(ct.isNewCache(cache_fn));

  ASSERT_EQ(unlink(cache_fn.c_str()), 0);
  ASSERT_EQ(rmdir(temp_dir.c_str()), 0);
}

TEST_F(TestCacheType, NewCache) {
  CacheManager cm;
  ASSERT_TRUE(cm.addEmptyEntry("test_entry"));

  string temp_dir;
  ASSERT_TRUE(makeTempDir(&temp_dir));

  string cache_fn(temp_dir);
  cache_fn.append("/cache.dump");

  ASSERT_TRUE(cm.saveCache(cache_fn));

  CacheType ct;
  ASSERT_TRUE(ct.isNewCache(cache_fn));

  ASSERT_EQ(unlink(cache_fn.c_str()), 0);
  ASSERT_EQ(rmdir(temp_dir.c_str()), 0);
}

}  // namespace HPHP
