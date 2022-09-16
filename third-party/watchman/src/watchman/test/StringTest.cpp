/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <folly/portability/GTest.h>
#include <string>
#include "watchman/watchman_string.h"

TEST(String, fmt) {
  EXPECT_EQ(w_string::format("hello {}", "world"), w_string("hello world"));

  // Tickle the boundary case between the stack buffer and the dynamic
  // string construction in w_string::vformat
  for (size_t i = 1023; i <= 1025; ++i) {
    std::string a(i, 'a');
    auto s = w_string::format("{}", a);
    EXPECT_EQ(s, w_string(a.data(), a.size()));
  }
}

TEST(String, integrals) {
  EXPECT_EQ(w_string::build(int8_t(1)), w_string("1"));
  EXPECT_EQ(w_string::build(int16_t(1)), w_string("1"));
  EXPECT_EQ(w_string::build(int32_t(1)), w_string("1"));
  EXPECT_EQ(w_string::build(int64_t(1)), w_string("1"));

  EXPECT_EQ(w_string::build(int8_t(-1)), w_string("-1"));
  EXPECT_EQ(w_string::build(int16_t(-1)), w_string("-1"));
  EXPECT_EQ(w_string::build(int32_t(-1)), w_string("-1"));
  EXPECT_EQ(w_string::build(int64_t(-1)), w_string("-1"));

  EXPECT_EQ(w_string::build(uint8_t(1)), w_string("1"));
  EXPECT_EQ(w_string::build(uint16_t(1)), w_string("1"));
  EXPECT_EQ(w_string::build(uint32_t(1)), w_string("1"));
  EXPECT_EQ(w_string::build(uint64_t(1)), w_string("1"));

  EXPECT_EQ(w_string::build(uint8_t(255)), w_string("255"));
  EXPECT_EQ(w_string::build(uint16_t(255)), w_string("255"));
  EXPECT_EQ(w_string::build(uint32_t(255)), w_string("255"));
  EXPECT_EQ(w_string::build(uint64_t(255)), w_string("255"));

  EXPECT_EQ(w_string::build(int8_t(-127)), w_string("-127"));

  EXPECT_EQ(w_string::build(bool(true)), w_string("true"));
  EXPECT_EQ(w_string::build(bool(false)), w_string("false"));
}

TEST(String, strings) {
  {
    auto hello = w_string::build("hello");
    EXPECT_EQ(hello, w_string("hello"));
    EXPECT_EQ(hello.size(), 5);
    EXPECT_TRUE(!strcmp("hello", hello.c_str()))
        << "looks nul terminated `" << hello.c_str() << "` "
        << strlen_uint32(hello.c_str());
  }

  {
    w_string_piece piece("hello");
    EXPECT_EQ(piece.size(), 5);
    auto hello = w_string::build(piece);
    EXPECT_EQ(hello.size(), 5);
    EXPECT_TRUE(!strcmp("hello", hello.c_str())) << "looks nul terminated";
  }

  {
    char foo[] = "foo";
    auto str = w_string::build(foo);
    EXPECT_EQ(str.size(), 3);
    EXPECT_FALSE(str.empty());
    EXPECT_TRUE(!strcmp("foo", foo)) << "foo matches";
  }

  {
    w_string defaultStr;
    EXPECT_TRUE(defaultStr.empty())
        << "default constructed string should be empty";

    w_string movedFrom{"hello"};
    w_string{std::move(movedFrom)};
    EXPECT_TRUE(movedFrom.empty()) << "moved-from string should be empty";

    EXPECT_TRUE(w_string_piece().empty())
        << "default constructed string piece shouldbe empty";

    EXPECT_TRUE(w_string::build("").empty()) << "empty string is empty";
  }
}

TEST(String, pointers) {
  bool foo = true;
  char lowerBuf[20];

  auto str = w_string::build(fmt::ptr(&foo));
  snprintf(
      lowerBuf, sizeof(lowerBuf), "0x%" PRIx64, (uint64_t)(uintptr_t)(&foo));
  EXPECT_EQ(str.size(), strlen_uint32(lowerBuf))
      << "reasonable seeming bool pointer len, got " << str.size()
      << " vs expected " << strlen_uint32(lowerBuf);
  EXPECT_EQ(str.size(), strlen_uint32(str.c_str()))
      << "string is really nul terminated, size " << str.size()
      << " strlen of c_str " << strlen_uint32(str.c_str());
  EXPECT_TRUE(!strcmp(lowerBuf, str.c_str()))
      << "bool pointer rendered right hex value sprintf->" << lowerBuf
      << " str->" << str.c_str();

  str = w_string::build(nullptr);
  EXPECT_GT(str.size(), 0);
  EXPECT_EQ(str, w_string("0x0"));

  void* zero = 0;
  EXPECT_EQ(w_string::build(zero), "0x0");
}

TEST(String, double) {
  auto str = w_string::build(5.5);
  EXPECT_EQ(str, w_string("5.5"));
}

TEST(String, canon_path) {
  EXPECT_EQ("foo", w_string_canon_path("foo"));
  EXPECT_EQ("foo", w_string_canon_path("foo/"));
  EXPECT_EQ("foo", w_string_canon_path("foo//"));
  EXPECT_EQ("/foo", w_string_canon_path("/foo"));
  EXPECT_EQ("foo/bar", w_string_canon_path("foo/bar"));
}

TEST(String, concat) {
  auto str = w_string::build("one", 2, "three", 1.2, false, w_string_piece{});
  EXPECT_EQ(str, w_string("one2three1.2false"));
}

TEST(String, lowercase_suffix) {
  EXPECT_FALSE(w_string("").asLowerCaseSuffix());
  EXPECT_EQ(w_string(".").asLowerCaseSuffix(), std::nullopt);
  EXPECT_EQ(w_string("endwithdot.").asLowerCaseSuffix(), std::nullopt);
  EXPECT_FALSE(w_string("nosuffix").asLowerCaseSuffix());
  EXPECT_EQ(
      w_string(".beginwithdot").asLowerCaseSuffix(), w_string("beginwithdot"));
  EXPECT_EQ(
      w_string("MainActivity.java").asLowerCaseSuffix(), w_string("java"));
  EXPECT_EQ(w_string("README.TXT").asLowerCaseSuffix(), w_string("txt"));
  EXPECT_EQ(
      w_string("README.camelCaseSuffix").asLowerCaseSuffix(),
      w_string("camelcasesuffix"));
  EXPECT_EQ(w_string("foo/bar").asLowerCaseSuffix(), std::nullopt);
  EXPECT_EQ(w_string("foo.wat/bar").asLowerCaseSuffix(), std::nullopt);
  EXPECT_EQ(w_string("foo.wat/bar.xml").asLowerCaseSuffix(), w_string("xml"));
  EXPECT_EQ(w_string("foo\\bar").asLowerCaseSuffix(), std::nullopt);
  EXPECT_EQ(w_string("foo\\bar.lU").asLowerCaseSuffix(), w_string("lu"));

#ifdef _WIN32
  EXPECT_EQ(w_string("foo.wat\\bar").asLowerCaseSuffix(), std::nullopt);
#else
  EXPECT_EQ(w_string("foo.wat\\bar").asLowerCaseSuffix(), w_string("wat\\bar"));
#endif

  // 255 is the longest suffix among some systems
  std::string longName(255, 'a');
  auto str = w_string::build(".", longName.c_str());
  EXPECT_EQ(str.asLowerCaseSuffix()->size(), 255);
}

TEST(String, string_piece_suffix) {
  EXPECT_EQ(w_string_piece().suffix(), "");
  EXPECT_EQ(w_string_piece("").suffix(), "");
  EXPECT_EQ(w_string_piece(".").suffix(), "");
  EXPECT_EQ(w_string_piece("endwithdot.").suffix(), "");
  EXPECT_EQ(w_string_piece("nosuffix").suffix(), "");
  EXPECT_EQ(
      w_string_piece(".beginwithdot").suffix(), w_string_piece("beginwithdot"));
  EXPECT_EQ(
      w_string_piece("MainActivity.java").suffix(), w_string_piece("java"));
  EXPECT_EQ(w_string_piece("README.TXT").suffix(), w_string_piece("TXT"));
  EXPECT_EQ(
      w_string_piece("README.camelCaseSuffix").suffix(),
      w_string_piece("camelCaseSuffix"));
  EXPECT_EQ(w_string_piece("foo/bar").suffix(), "");
  EXPECT_EQ(w_string_piece("foo.wat/bar").suffix(), "");
  EXPECT_EQ(w_string_piece("foo.wat/bar.xml").suffix(), "xml");
  EXPECT_EQ(w_string_piece("foo\\bar").suffix(), "");
  EXPECT_EQ(w_string_piece("foo\\bar.lU").suffix(), "lU");

#ifdef _WIN32
  EXPECT_EQ(w_string_piece("foo.wat\\bar").suffix(), "");
#else
  EXPECT_EQ(
      w_string_piece("foo.wat\\bar").suffix(), w_string_piece("wat\\bar"));
#endif

  // 255 is the longest suffix among some systems
  std::string longName(255, 'a');
  auto str = w_string::build(".", longName.c_str());
  auto sp = w_string_piece(str.data(), str.size());
  EXPECT_EQ(sp.asLowerCaseSuffix()->size(), 255);
}

TEST(String, string_piece_lowercase_suffix) {
  EXPECT_EQ(w_string_piece().asLowerCaseSuffix(), std::nullopt);
  EXPECT_EQ(w_string_piece("").asLowerCaseSuffix(), std::nullopt);
  EXPECT_EQ(w_string_piece(".").asLowerCaseSuffix(), std::nullopt);
  EXPECT_EQ(w_string_piece("endwithdot.").asLowerCaseSuffix(), std::nullopt);
  EXPECT_FALSE(w_string_piece("nosuffix").asLowerCaseSuffix());
  EXPECT_EQ(
      w_string_piece(".beginwithdot").asLowerCaseSuffix(),
      w_string("beginwithdot"));
  EXPECT_EQ(
      w_string_piece("MainActivity.java").asLowerCaseSuffix(),
      w_string("java"));
  EXPECT_EQ(w_string_piece("README.TXT").asLowerCaseSuffix(), w_string("txt"));
  EXPECT_EQ(
      w_string_piece("README.camelCaseSuffix").asLowerCaseSuffix(),
      w_string("camelcasesuffix"));
  EXPECT_EQ(w_string_piece("foo/bar").asLowerCaseSuffix(), std::nullopt);
  EXPECT_EQ(w_string_piece("foo.wat/bar").asLowerCaseSuffix(), std::nullopt);
  EXPECT_EQ(
      w_string_piece("foo.wat/bar.xml").asLowerCaseSuffix(), w_string("xml"));
  EXPECT_EQ(w_string_piece("foo\\bar").asLowerCaseSuffix(), std::nullopt);
  EXPECT_EQ(w_string_piece("foo\\bar.lU").asLowerCaseSuffix(), w_string("lu"));

#ifdef _WIN32
  EXPECT_EQ(w_string_piece("foo.wat\\bar").asLowerCaseSuffix(), std::nullopt);
#else
  EXPECT_EQ(
      w_string_piece("foo.wat\\bar").asLowerCaseSuffix(), w_string("wat\\bar"));
#endif

  // 255 is the longest suffix among some systems
  std::string longName(255, 'a');
  auto str = w_string::build(".", longName.c_str());
  auto sp = w_string_piece(str.c_str(), str.size());
  EXPECT_EQ(sp.asLowerCaseSuffix()->size(), 255);
}

TEST(String, path_cat) {
  auto str = w_string::pathCat({"foo", ""});
  EXPECT_EQ("foo", str);
  EXPECT_EQ(3, str.size());

  str = w_string::pathCat({"", "foo"});
  EXPECT_EQ("foo", str);
  EXPECT_EQ(3, str.size());

  str = w_string::pathCat({"foo", "bar"});
  EXPECT_EQ("foo/bar", str);
  EXPECT_EQ(7, str.size());

  str = w_string::pathCat({"foo", "bar", ""});
  EXPECT_EQ("foo/bar", str);
  EXPECT_EQ(7, str.size());

  str = w_string::pathCat({"foo", "", "bar"});
  EXPECT_EQ("foo/bar", str);
  EXPECT_EQ(7, str.size());
}

TEST(String, basename_dirname) {
  auto str = w_string_piece("foo/bar").baseName().asWString();
  EXPECT_EQ(str, "bar");

  str = w_string_piece("foo/bar").dirName().asWString();
  EXPECT_EQ(str, "foo");

  str = w_string_piece("").baseName().asWString();
  EXPECT_EQ(str, "");

  str = w_string_piece("").dirName().asWString();
  EXPECT_EQ(str, "");

  str = w_string_piece("foo").dirName().asWString();
  EXPECT_EQ(str, "");

  str = w_string("f/b/z");
  auto piece = str.piece().dirName();
  auto str2 = piece.baseName().asWString();
  EXPECT_EQ(str2, "b");

  str = w_string_piece("foo/bar/baz").dirName().dirName().asWString();
  EXPECT_EQ(str, "foo");

  str = w_string_piece("foo").baseName().asWString();
  EXPECT_EQ(str, "foo");

  str = w_string_piece("foo\\bar").baseName().asWString();
#ifdef _WIN32
  EXPECT_EQ(str, "bar");
#else
  EXPECT_EQ(str, "foo\\bar");
#endif

  str = w_string_piece("foo\\bar").dirName().asWString();
#ifdef _WIN32
  EXPECT_EQ(str, "foo");
#else
  EXPECT_EQ(str, "");
#endif

#ifdef _WIN32
  w_string_piece winFoo("C:\\foo");

  str = winFoo.baseName().asWString();
  EXPECT_EQ(str, "foo");

  str = winFoo.dirName().asWString();
  EXPECT_EQ(str, "C:\\");

  str = winFoo.dirName().dirName().asWString();
  EXPECT_EQ(str, "C:\\");
#endif

  // This is testing that we don't walk off the end of the string.
  // We had a bug where if the buffer had a slash as the character
  // after the end of the string, baseName and dirName could incorrectly
  // match that position and trigger a string range check.
  // The endSlash string below has 7 characters, with the 8th byte
  // as a slash to trigger this condition.
  w_string_piece endSlash("dir/foo/", 7);
  str = endSlash.baseName().asWString();
  EXPECT_EQ(str, "foo");
  str = endSlash.dirName().asWString();
  EXPECT_EQ(str, "dir");
}

TEST(String, operators) {
  EXPECT_LT(w_string_piece("a"), w_string_piece("b"));
  EXPECT_LT(w_string_piece("a"), w_string_piece("ba"));
  EXPECT_LT(w_string_piece("aa"), w_string_piece("b"));
  EXPECT_TRUE(!(w_string_piece("b") < w_string_piece("a"))) << "b not < a";
  EXPECT_TRUE(!(w_string_piece("a") < w_string_piece("a"))) << "a not < a";
  EXPECT_LT(w_string_piece("A"), w_string_piece("a"));
}

TEST(String, piece_and_string_should_have_same_hash) {
  EXPECT_EQ(w_string{""}.hashValue(), w_string_piece{""}.hashValue());
  EXPECT_EQ(
      w_string{"foobar"}.hashValue(), w_string_piece{"foobar"}.hashValue());
}

TEST(String, split) {
  {
    std::vector<std::string> expected{"a", "b", "c"};
    std::vector<std::string> result;
    w_string_piece("a:b:c").split(result, ':');

    EXPECT_TRUE(expected == result) << "split ok";
  }

  {
    std::vector<w_string> expected{"a", "b", "c"};
    std::vector<w_string> result;
    w_string_piece("a:b:c").split(result, ':');

    EXPECT_TRUE(expected == result) << "split ok (w_string)";
  }

  {
    std::vector<std::string> expected{"a", "b", "c"};
    std::vector<std::string> result;
    w_string_piece("a:b:c:").split(result, ':');

    EXPECT_TRUE(expected == result)
        << "split doesn't create empty last element";
  }

  {
    std::vector<std::string> expected{"a", "b", "", "c"};
    std::vector<std::string> result;
    w_string_piece("a:b::c:").split(result, ':');

    EXPECT_TRUE(expected == result) << "split does create empty element";
  }

  {
    std::vector<std::string> result;
    w_string_piece().split(result, ':');
    EXPECT_TRUE(result.size() == 0)
        << "split as 0 elements, got " << result.size();

    w_string_piece(w_string()).split(result, ':');
    EXPECT_TRUE(result.size() == 0)
        << "split as 0 elements, got " << result.size();
  }
}

TEST(String, path_equal) {
  EXPECT_TRUE(w_string_piece("/foo/bar").pathIsEqual("/foo/bar"));
  EXPECT_TRUE(!w_string_piece("/foo/bar").pathIsEqual("/Foo/bar"));
#ifdef _WIN32
  EXPECT_TRUE(w_string_piece("c:/foo/bar").pathIsEqual("C:/foo/bar"))
      << "allow different case for drive letter only c:/foo/bar";
  EXPECT_TRUE(w_string_piece("c:/foo\\bar").pathIsEqual("C:/foo/bar"))
      << "allow different slashes c:/foo\\bar";
  EXPECT_TRUE(!w_string_piece("c:/Foo/bar").pathIsEqual("C:/foo/bar"))
      << "strict case in the other positions c:/Foo/bar";
#endif
}

TEST(String, truncated_head) {
  char head[8];
  storeTruncatedHead(head, w_string_piece{"foo"});
  EXPECT_EQ(
      w_string_piece{"foo"}, w_string_piece(head, strnlen(head, sizeof(head))));

  storeTruncatedHead(head, w_string_piece{"0123456"});
  EXPECT_EQ(
      w_string_piece{"0123456"},
      w_string_piece(head, strnlen(head, sizeof(head))));

  storeTruncatedHead(head, w_string_piece{"01234567"});
  EXPECT_EQ(
      w_string_piece{"01234567"},
      w_string_piece(head, strnlen(head, sizeof(head))));

  storeTruncatedHead(head, w_string_piece{"012345678"});
  EXPECT_EQ(
      w_string_piece{"01234..."},
      w_string_piece(head, strnlen(head, sizeof(head))));
}

TEST(String, truncated_tail) {
  char head[8];
  storeTruncatedTail(head, w_string_piece{"foo"});
  EXPECT_EQ(
      w_string_piece{"foo"}, w_string_piece(head, strnlen(head, sizeof(head))));

  storeTruncatedTail(head, w_string_piece{"0123456"});
  EXPECT_EQ(
      w_string_piece{"0123456"},
      w_string_piece(head, strnlen(head, sizeof(head))));

  storeTruncatedTail(head, w_string_piece{"01234567"});
  EXPECT_EQ(
      w_string_piece{"01234567"},
      w_string_piece(head, strnlen(head, sizeof(head))));

  storeTruncatedTail(head, w_string_piece{"012345678"});
  EXPECT_EQ(
      w_string_piece{"...45678"},
      w_string_piece(head, strnlen(head, sizeof(head))));
}

TEST(String, contains) {
  w_string data{"watchman"};
  w_string_piece haystack{data};
  EXPECT_TRUE(haystack.contains("atch"));
  EXPECT_FALSE(haystack.contains("maan"));
  EXPECT_TRUE(haystack.contains(""));
  EXPECT_TRUE(haystack.contains("watchman"));
  EXPECT_FALSE(haystack.contains("watchman2"));
}

TEST(String, allocate_many_sizes) {
  // This strange test relies on ASAN to assert that our allocation size math is
  // correct.
  EXPECT_EQ(0, w_string("", 0).size());
  EXPECT_EQ(1, w_string("x", 1).size());
  EXPECT_EQ(2, w_string("xx", 2).size());
  EXPECT_EQ(3, w_string("xxx", 3).size());
  EXPECT_EQ(4, w_string("xxxx", 4).size());
  EXPECT_EQ(5, w_string("xxxxx", 5).size());
  EXPECT_EQ(6, w_string("xxxxxx", 6).size());
  EXPECT_EQ(7, w_string("xxxxxxx", 7).size());
  EXPECT_EQ(8, w_string("xxxxxxxx", 8).size());
}
