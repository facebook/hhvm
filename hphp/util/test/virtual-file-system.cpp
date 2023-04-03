/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-present Facebook, Inc. (http://www.facebook.com)  |
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

#include "hphp/util/virtual-file-system.h"

#include <folly/FileUtil.h>
#include <gtest/gtest.h>

namespace HPHP {

struct TestVirtualFileSystem : testing::Test {
 protected:
  const std::string kFooContent = "foocontent";
  const std::string kBarContent = "foocontent asdasdasd";

  void SetUp() override {
  }

  std::string makeTempDir() {
    char dir_tmpl[] = "/tmp/hhvm_unit_test.XXXXXX";
    const char* temp = mkdtemp(dir_tmpl);
    EXPECT_NE(temp, nullptr);

    std::string dir;
    dir.assign(temp);
    dir.append("/");
    return dir;
  }

  std::string makeTempFile(const std::string& content) {
    char file_tmpl[] = "/tmp/hhvm_unit_test-testdata.XXXXXX";
    int fd = mkstemp(file_tmpl);
    EXPECT_GE(fd, 0);

    auto written = folly::writeFull(fd, content.data(), content.size());
    EXPECT_EQ(written, content.size());

    close(fd);

    std::string tmpPath;
    tmpPath.assign(file_tmpl);
    return tmpPath;
  }

  std::string makeVFS() {
    std::string content_path(makeTempDir());
    content_path.append("content.hvfs");

    auto writer = VirtualFileSystemWriter(content_path);

    // Write some empty files
    EXPECT_TRUE(writer.addFileWithoutContent("foo.dat"));
    EXPECT_TRUE(writer.addFileWithoutContent("bar.dat"));
    EXPECT_TRUE(writer.addFileWithoutContent("baz/foo.dat"));
    EXPECT_TRUE(writer.addFileWithoutContent("baz/bar.dat"));

    // Can not add absolute paths
    EXPECT_FALSE(writer.addFileWithoutContent("/baz/bar.dat"));

    // Can not add the same path twice
    EXPECT_FALSE(writer.addFileWithoutContent("foo.dat"));

    // Write some regular files
    EXPECT_TRUE(writer.addFile("r/foo.dat", makeTempFile(kFooContent)));
    EXPECT_TRUE(writer.addFile("r/bar.dat", makeTempFile(kBarContent)));

    // Can not add absolute paths
    EXPECT_FALSE(writer.addFile("/r/bar.dat", makeTempFile(kBarContent)));

    // Can not add the same path twice
    EXPECT_FALSE(writer.addFile("r/foo.dat", makeTempFile(kFooContent)));

    writer.finish();

    return content_path;
  }

  void expect_eq_ignore_order(const std::vector<std::string>& a,
                              const std::vector<std::string>& b) {
    auto aSorted = a;
    std::sort(aSorted.begin(), aSorted.end());
    auto bSorted = b;
    std::sort(bSorted.begin(), bSorted.end());
    EXPECT_EQ(aSorted, bSorted);
  }

  void expect_same_content(const Optional<VirtualFileSystem::Content>& content,
                           const std::string& expected) {
    EXPECT_TRUE(content.has_value());
    EXPECT_EQ(content->size, expected.size());
    EXPECT_EQ(memcmp(content->buffer, expected.data(), expected.size()), 0);
  }
};

TEST_F(TestVirtualFileSystem, exists) {
  std::string content_path = makeVFS();
  auto vfs = VirtualFileSystem(content_path, "/var/www/");

  // We run everything twice to make sure the internal caches we uses also works
  for (auto i = 0; i < 2; i++) {
    EXPECT_TRUE(vfs.exists("foo.dat"));
    EXPECT_TRUE(vfs.exists("bar.dat"));
    EXPECT_TRUE(vfs.exists("baz/foo.dat"));
    EXPECT_TRUE(vfs.exists("baz/bar.dat"));
    EXPECT_TRUE(vfs.exists("r/foo.dat"));
    EXPECT_TRUE(vfs.exists("r/bar.dat"));
    EXPECT_FALSE(vfs.exists("baz.dat"));
    EXPECT_FALSE(vfs.exists("baz/baz.dat"));

    EXPECT_TRUE(vfs.exists("/var/www/foo.dat"));
    EXPECT_TRUE(vfs.exists("/var/www/bar.dat"));
    EXPECT_TRUE(vfs.exists("/var/www/baz/foo.dat"));
    EXPECT_TRUE(vfs.exists("/var/www/baz/bar.dat"));
    EXPECT_TRUE(vfs.exists("/var/www/r/foo.dat"));
    EXPECT_TRUE(vfs.exists("/var/www/r/bar.dat"));
    EXPECT_FALSE(vfs.exists("/var/www/baz.dat"));
    EXPECT_FALSE(vfs.exists("/var/www/baz/baz.dat"));

    // Check that directories exists
    EXPECT_TRUE(vfs.exists(""));
    EXPECT_TRUE(vfs.exists("baz"));
    EXPECT_TRUE(vfs.exists("r"));
    EXPECT_TRUE(vfs.exists("baz/"));
    EXPECT_TRUE(vfs.exists("r/"));

    EXPECT_TRUE(vfs.exists("/var/www/"));
    EXPECT_TRUE(vfs.exists("/var/www/baz"));
    EXPECT_TRUE(vfs.exists("/var/www/r"));
    EXPECT_TRUE(vfs.exists("/var/www/baz/"));
    EXPECT_TRUE(vfs.exists("/var/www/r/"));
  }
}

TEST_F(TestVirtualFileSystem, fileExists) {
  std::string content_path = makeVFS();
  auto vfs = VirtualFileSystem(content_path, "/var/www/");

  // We run everything twice to make sure the internal caches we uses also works
  for (auto i = 0; i < 2; i++) {
    EXPECT_TRUE(vfs.fileExists("foo.dat"));
    EXPECT_TRUE(vfs.fileExists("bar.dat"));
    EXPECT_TRUE(vfs.fileExists("baz/foo.dat"));
    EXPECT_TRUE(vfs.fileExists("baz/bar.dat"));
    EXPECT_TRUE(vfs.fileExists("r/foo.dat"));
    EXPECT_TRUE(vfs.fileExists("r/bar.dat"));
    EXPECT_FALSE(vfs.fileExists("baz.dat"));
    EXPECT_FALSE(vfs.fileExists("baz/baz.dat"));

    EXPECT_TRUE(vfs.fileExists("/var/www/foo.dat"));
    EXPECT_TRUE(vfs.fileExists("/var/www/bar.dat"));
    EXPECT_TRUE(vfs.fileExists("/var/www/baz/foo.dat"));
    EXPECT_TRUE(vfs.fileExists("/var/www/baz/bar.dat"));
    EXPECT_TRUE(vfs.fileExists("/var/www/r/foo.dat"));
    EXPECT_TRUE(vfs.fileExists("/var/www/r/bar.dat"));
    EXPECT_FALSE(vfs.fileExists("/var/www/baz.dat"));
    EXPECT_FALSE(vfs.fileExists("/var/www/baz/baz.dat"));

    // Check that directories return false
    EXPECT_FALSE(vfs.fileExists(""));
    EXPECT_FALSE(vfs.fileExists("baz"));
    EXPECT_FALSE(vfs.fileExists("r"));

    EXPECT_FALSE(vfs.fileExists("/var/www/"));
    EXPECT_FALSE(vfs.fileExists("/var/www/baz"));
    EXPECT_FALSE(vfs.fileExists("/var/www/r"));
  }
}

TEST_F(TestVirtualFileSystem, dirExists) {
  std::string content_path = makeVFS();
  auto vfs = VirtualFileSystem(content_path, "/var/www/");

  // We run everything twice to make sure the internal caches we uses also works
  for (auto i = 0; i < 2; i++) {
    EXPECT_FALSE(vfs.dirExists("foo.dat"));
    EXPECT_FALSE(vfs.dirExists("bar.dat"));
    EXPECT_FALSE(vfs.dirExists("baz/foo.dat"));
    EXPECT_FALSE(vfs.dirExists("baz/bar.dat"));
    EXPECT_FALSE(vfs.dirExists("r/foo.dat"));
    EXPECT_FALSE(vfs.dirExists("r/bar.dat"));
    EXPECT_FALSE(vfs.dirExists("baz.dat"));
    EXPECT_FALSE(vfs.dirExists("baz/baz.dat"));

    EXPECT_FALSE(vfs.dirExists("/var/www/foo.dat"));
    EXPECT_FALSE(vfs.dirExists("/var/www/bar.dat"));
    EXPECT_FALSE(vfs.dirExists("/var/www/baz/foo.dat"));
    EXPECT_FALSE(vfs.dirExists("/var/www/baz/bar.dat"));
    EXPECT_FALSE(vfs.dirExists("/var/www/r/foo.dat"));
    EXPECT_FALSE(vfs.dirExists("/var/www/r/bar.dat"));
    EXPECT_FALSE(vfs.dirExists("/var/www/baz.dat"));
    EXPECT_FALSE(vfs.dirExists("/var/www/baz/baz.dat"));

    // Check that directories exists
    EXPECT_TRUE(vfs.dirExists(""));
    EXPECT_TRUE(vfs.dirExists("baz"));
    EXPECT_TRUE(vfs.dirExists("r"));

    EXPECT_TRUE(vfs.dirExists("/var/www/"));
    EXPECT_TRUE(vfs.dirExists("/var/www/baz"));
    EXPECT_TRUE(vfs.dirExists("/var/www/r"));
  }
}

TEST_F(TestVirtualFileSystem, fileSize) {
  std::string content_path = makeVFS();
  auto vfs = VirtualFileSystem(content_path, "/var/www/");

  // We run everything twice to make sure the internal caches we uses also works
  for (auto i = 0; i < 2; i++) {
    EXPECT_EQ(*vfs.fileSize("r/foo.dat"), kFooContent.size());
    EXPECT_EQ(*vfs.fileSize("r/bar.dat"), kBarContent.size());

    EXPECT_EQ(*vfs.fileSize("/var/www/r/foo.dat"), kFooContent.size());
    EXPECT_EQ(*vfs.fileSize("/var/www/r/bar.dat"), kBarContent.size());

    // Empty files and directories should all not have a filesize
    EXPECT_TRUE(!vfs.fileSize("foo.dat"));
    EXPECT_TRUE(!vfs.fileSize("bar.dat"));
    EXPECT_TRUE(!vfs.fileSize("baz/foo.dat"));
    EXPECT_TRUE(!vfs.fileSize("baz/bar.dat"));
    EXPECT_TRUE(!vfs.fileSize("baz.dat"));
    EXPECT_TRUE(!vfs.fileSize("baz/baz.dat"));
    EXPECT_TRUE(!vfs.fileSize(""));
    EXPECT_TRUE(!vfs.fileSize("baz"));

    EXPECT_TRUE(!vfs.fileSize("/var/www/foo.dat"));
    EXPECT_TRUE(!vfs.fileSize("/var/www/bar.dat"));
    EXPECT_TRUE(!vfs.fileSize("/var/www/baz/foo.dat"));
    EXPECT_TRUE(!vfs.fileSize("/var/www/baz/bar.dat"));
    EXPECT_TRUE(!vfs.fileSize("/var/www/baz.dat"));
    EXPECT_TRUE(!vfs.fileSize("/var/www/baz/baz.dat"));
    EXPECT_TRUE(!vfs.fileSize("/var/www/"));
    EXPECT_TRUE(!vfs.fileSize("/var/www/baz"));
  }
}

TEST_F(TestVirtualFileSystem, content) {
  std::string content_path = makeVFS();
  auto vfs = VirtualFileSystem(content_path, "/var/www/");

  // We run everything twice to make sure the internal caches we uses also works
  for (auto i = 0; i < 2; i++) {
    expect_same_content(vfs.content("r/foo.dat"), kFooContent);
    expect_same_content(vfs.content("r/bar.dat"), kBarContent);

    expect_same_content(vfs.content("/var/www/r/foo.dat"), kFooContent);
    expect_same_content(vfs.content("/var/www/r/bar.dat"), kBarContent);

    // Empty files and directories should all not have content
    EXPECT_TRUE(!vfs.content("foo.dat"));
    EXPECT_TRUE(!vfs.content("bar.dat"));
    EXPECT_TRUE(!vfs.content("baz/foo.dat"));
    EXPECT_TRUE(!vfs.content("baz/bar.dat"));
    EXPECT_TRUE(!vfs.content("baz.dat"));
    EXPECT_TRUE(!vfs.content("baz/baz.dat"));
    EXPECT_TRUE(!vfs.content(""));
    EXPECT_TRUE(!vfs.content("baz"));

    EXPECT_TRUE(!vfs.content("/var/www/foo.dat"));
    EXPECT_TRUE(!vfs.content("/var/www/bar.dat"));
    EXPECT_TRUE(!vfs.content("/var/www/baz/foo.dat"));
    EXPECT_TRUE(!vfs.content("/var/www/baz/bar.dat"));
    EXPECT_TRUE(!vfs.content("/var/www/baz.dat"));
    EXPECT_TRUE(!vfs.content("/var/www/baz/baz.dat"));
    EXPECT_TRUE(!vfs.content("/var/www/"));
    EXPECT_TRUE(!vfs.content("/var/www/baz"));
  }
}

TEST_F(TestVirtualFileSystem, listDirectory) {
  std::string content_path = makeVFS();
  auto vfs = VirtualFileSystem(content_path, "/var/www/");

  // We run everything twice to make sure the internal caches we uses also works
  for (auto i = 0; i < 2; i++) {

    // Files and non existing files should just return empty
    expect_eq_ignore_order(vfs.listDirectory("foo.dat"),
                           std::vector<std::string>());
    expect_eq_ignore_order(vfs.listDirectory("bar.dat"),
                           std::vector<std::string>());
    expect_eq_ignore_order(vfs.listDirectory("baz/foo.dat"),
                           std::vector<std::string>());
    expect_eq_ignore_order(vfs.listDirectory("baz/bar.dat"),
                           std::vector<std::string>());
    expect_eq_ignore_order(vfs.listDirectory("baz.dat"),
                           std::vector<std::string>());
    expect_eq_ignore_order(vfs.listDirectory("baz/baz.dat"),
                           std::vector<std::string>());
    expect_eq_ignore_order(vfs.listDirectory("r/foo.dat"),
                           std::vector<std::string>());
    expect_eq_ignore_order(vfs.listDirectory("r/bar.dat"),
                           std::vector<std::string>());

    expect_eq_ignore_order(vfs.listDirectory("/var/www/foo.dat"),
                           std::vector<std::string>());
    expect_eq_ignore_order(vfs.listDirectory("/var/www/bar.dat"),
                           std::vector<std::string>());
    expect_eq_ignore_order(vfs.listDirectory("/var/www/baz/foo.dat"),
                           std::vector<std::string>());
    expect_eq_ignore_order(vfs.listDirectory("/var/www/baz/bar.dat"),
                           std::vector<std::string>());
    expect_eq_ignore_order(vfs.listDirectory("/var/www/baz.dat"),
                           std::vector<std::string>());
    expect_eq_ignore_order(vfs.listDirectory("/var/www/baz/baz.dat"),
                           std::vector<std::string>());
    expect_eq_ignore_order(vfs.listDirectory("/var/www/r/foo.dat"),
                           std::vector<std::string>());
    expect_eq_ignore_order(vfs.listDirectory("/var/www/r/bar.dat"),
                           std::vector<std::string>());

    // Check directories
    std::vector<std::string> expectedFooBar = {"foo.dat", "bar.dat"};
    expect_eq_ignore_order(vfs.listDirectory("r"), expectedFooBar);
    expect_eq_ignore_order(vfs.listDirectory("baz"), expectedFooBar);

    expect_eq_ignore_order(vfs.listDirectory("/var/www/r"), expectedFooBar);
    expect_eq_ignore_order(vfs.listDirectory("/var/www/baz"), expectedFooBar);

    std::vector<std::string> expectedRoot = {"foo.dat", "bar.dat", "baz", "r"};
    expect_eq_ignore_order(vfs.listDirectory(""), expectedRoot);

    expect_eq_ignore_order(vfs.listDirectory("/var/www/"), expectedRoot);
  }
}

}
