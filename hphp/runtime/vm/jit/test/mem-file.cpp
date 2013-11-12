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

#include <string>

#include <gtest/gtest.h>

#include "hphp/runtime/base/mem-file.h"
#include "hphp/runtime/server/static-content-cache.h"

using std::string;

static bool makeTempFile(const string& contents, string* fn) {
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

namespace HPHP {

TEST(TestMemFile, DataConstructor) {
  const char* test_data = "some test data";
  MemFile mf(test_data, strlen(test_data));

  ASSERT_EQ(mf.getc(), 's');
  ASSERT_EQ(mf.getc(), 'o');
  ASSERT_EQ(mf.getc(), 'm');
  ASSERT_EQ(mf.getc(), 'e');
  ASSERT_EQ(mf.getc(), ' ');
  ASSERT_EQ(mf.getc(), 't');
  ASSERT_EQ(mf.getc(), 'e');
  ASSERT_EQ(mf.getc(), 's');
  ASSERT_EQ(mf.getc(), 't');
  ASSERT_EQ(mf.getc(), ' ');
  ASSERT_EQ(mf.getc(), 'd');
  ASSERT_EQ(mf.getc(), 'a');
  ASSERT_EQ(mf.getc(), 't');
  ASSERT_EQ(mf.getc(), 'a');
  ASSERT_EQ(mf.getc(), EOF);
}

TEST(TestMemFile, BadReadFromCache) {
  StaticContentCache::TheFileCache = FileCachePtr(new FileCache());

  MemFile mf;
  ASSERT_FALSE(mf.open("/some/random/file", "r"));
}

TEST(TestMemFile, BadOpenModes) {
  MemFile mf;
  ASSERT_FALSE(mf.open("/some/file1", "+"));
  ASSERT_FALSE(mf.open("/some/file2", "a"));
  ASSERT_FALSE(mf.open("/some/file3", "w"));
}

TEST(TestMemFile, EmptyFileInCache) {
  StaticContentCache::TheFileCache = FileCachePtr(new FileCache());
  StaticContentCache::TheFileCache->write("/no/content/entry");

  MemFile mf;

  // The file itself...
  ASSERT_FALSE(mf.open("/no/content/entry", "r"));

  // ... and one of the automatically-created "directory" entries.
  ASSERT_FALSE(mf.open("/no/content", "r"));
}

TEST(TestMemFile, NotInCache) {
  StaticContentCache::TheFileCache = FileCachePtr(new FileCache());

  MemFile mf;
  ASSERT_FALSE(mf.open("/not/even/there", "r"));
}

TEST(TestMemFile, RealFileInCache) {
  string temp_fn;
  ASSERT_TRUE(makeTempFile("123abc", &temp_fn));

  StaticContentCache::TheFileCache = FileCachePtr(new FileCache());
  StaticContentCache::TheFileCache->write("/content", temp_fn.c_str());

  MemFile mf;
  ASSERT_TRUE(mf.open("/content", "r"));

  ASSERT_EQ(mf.getc(), '1');
  ASSERT_EQ(mf.getc(), '2');
  ASSERT_EQ(mf.getc(), '3');
  ASSERT_EQ(mf.getc(), 'a');
  ASSERT_EQ(mf.getc(), 'b');
  ASSERT_EQ(mf.getc(), 'c');

  ASSERT_EQ(mf.getc(), EOF);

  ASSERT_TRUE(mf.close());

  ASSERT_EQ(unlink(temp_fn.c_str()), 0);
}

}  // namespace HPHP
