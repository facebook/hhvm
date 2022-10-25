/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <fcntl.h>
#include <fmt/core.h>
#include <folly/logging/xlog.h>
#include <folly/portability/GTest.h>
#include <inttypes.h>
#include <stdio.h>
#include <string.h>
#include "watchman/watchman_system.h"

#include "watchman/thirdparty/libart/src/art.h"

static FILE* open_test_file(const char* name) {
  FILE* f = fopen(name, "r");
  char altname[1024];

  if (f) {
    return f;
  }

#ifdef WATCHMAN_TEST_SRC_DIR
  snprintf(altname, sizeof(altname), "%s/%s", WATCHMAN_TEST_SRC_DIR, name);
  f = fopen(altname, "r");
  if (f) {
    return f;
  }
#endif

  snprintf(altname, sizeof(altname), "watchman/%s", name);
  f = fopen(altname, "r");
  if (f) {
    return f;
  }
  throw std::runtime_error(fmt::format("can't find test data file {}", name));
}

TEST(Art, insert) {
  art_tree<uintptr_t> t;
  int len;
  char buf[512];
  FILE* f = open_test_file("watchman/thirdparty/libart/tests/words.txt");
  uintptr_t line = 1;

  while (fgets(buf, sizeof buf, f)) {
    len = (int)strlen(buf);
    buf[len - 1] = '\0';
    t.insert(buf, line);
    EXPECT_EQ(t.size(), line) << "art_size didn't match current line no";
    line++;
  }
}

TEST(Art, insert_verylong) {
  art_tree<void*> t;

  unsigned char key1[300] = {
      16,  0,   0,   0,   7,   10,  0,   0,   0,   2,   17,  10,  0,   0,
      0,   120, 10,  0,   0,   0,   120, 10,  0,   0,   0,   216, 10,  0,
      0,   0,   202, 10,  0,   0,   0,   194, 10,  0,   0,   0,   224, 10,
      0,   0,   0,   230, 10,  0,   0,   0,   210, 10,  0,   0,   0,   206,
      10,  0,   0,   0,   208, 10,  0,   0,   0,   232, 10,  0,   0,   0,
      124, 10,  0,   0,   0,   124, 2,   16,  0,   0,   0,   2,   12,  185,
      89,  44,  213, 251, 173, 202, 211, 95,  185, 89,  110, 118, 251, 173,
      202, 199, 101, 0,   8,   18,  182, 92,  236, 147, 171, 101, 150, 195,
      112, 185, 218, 108, 246, 139, 164, 234, 195, 58,  177, 0,   8,   16,
      0,   0,   0,   2,   12,  185, 89,  44,  213, 251, 173, 202, 211, 95,
      185, 89,  110, 118, 251, 173, 202, 199, 101, 0,   8,   18,  180, 93,
      46,  151, 9,   212, 190, 95,  102, 178, 217, 44,  178, 235, 29,  190,
      218, 8,   16,  0,   0,   0,   2,   12,  185, 89,  44,  213, 251, 173,
      202, 211, 95,  185, 89,  110, 118, 251, 173, 202, 199, 101, 0,   8,
      18,  180, 93,  46,  151, 9,   212, 190, 95,  102, 183, 219, 229, 214,
      59,  125, 182, 71,  108, 180, 220, 238, 150, 91,  117, 150, 201, 84,
      183, 128, 8,   16,  0,   0,   0,   2,   12,  185, 89,  44,  213, 251,
      173, 202, 211, 95,  185, 89,  110, 118, 251, 173, 202, 199, 101, 0,
      8,   18,  180, 93,  46,  151, 9,   212, 190, 95,  108, 176, 217, 47,
      50,  219, 61,  134, 207, 97,  151, 88,  237, 246, 208, 8,   18,  255,
      255, 255, 219, 191, 198, 134, 5,   223, 212, 72,  44,  208, 250, 180,
      14,  1,   0,   0,   8,   '\0'};
  unsigned char key2[303] = {
      16,  0,   0,   0,   7,   10,  0,   0,   0,   2,   17,  10,  0,   0,   0,
      120, 10,  0,   0,   0,   120, 10,  0,   0,   0,   216, 10,  0,   0,   0,
      202, 10,  0,   0,   0,   194, 10,  0,   0,   0,   224, 10,  0,   0,   0,
      230, 10,  0,   0,   0,   210, 10,  0,   0,   0,   206, 10,  0,   0,   0,
      208, 10,  0,   0,   0,   232, 10,  0,   0,   0,   124, 10,  0,   0,   0,
      124, 2,   16,  0,   0,   0,   2,   12,  185, 89,  44,  213, 251, 173, 202,
      211, 95,  185, 89,  110, 118, 251, 173, 202, 199, 101, 0,   8,   18,  182,
      92,  236, 147, 171, 101, 150, 195, 112, 185, 218, 108, 246, 139, 164, 234,
      195, 58,  177, 0,   8,   16,  0,   0,   0,   2,   12,  185, 89,  44,  213,
      251, 173, 202, 211, 95,  185, 89,  110, 118, 251, 173, 202, 199, 101, 0,
      8,   18,  180, 93,  46,  151, 9,   212, 190, 95,  102, 178, 217, 44,  178,
      235, 29,  190, 218, 8,   16,  0,   0,   0,   2,   12,  185, 89,  44,  213,
      251, 173, 202, 211, 95,  185, 89,  110, 118, 251, 173, 202, 199, 101, 0,
      8,   18,  180, 93,  46,  151, 9,   212, 190, 95,  102, 183, 219, 229, 214,
      59,  125, 182, 71,  108, 180, 220, 238, 150, 91,  117, 150, 201, 84,  183,
      128, 8,   16,  0,   0,   0,   3,   12,  185, 89,  44,  213, 251, 133, 178,
      195, 105, 183, 87,  237, 150, 155, 165, 150, 229, 97,  182, 0,   8,   18,
      161, 91,  239, 50,  10,  61,  150, 223, 114, 179, 217, 64,  8,   12,  186,
      219, 172, 150, 91,  53,  166, 221, 101, 178, 0,   8,   18,  255, 255, 255,
      219, 191, 198, 134, 5,   208, 212, 72,  44,  208, 250, 180, 14,  1,   0,
      0,   8,   '\0'};

  t.insert(std::string(reinterpret_cast<char*>(key1), 299), (void*)key1);
  t.insert(std::string(reinterpret_cast<char*>(key2), 302), (void*)key2);
  t.insert(std::string(reinterpret_cast<char*>(key2), 302), (void*)key2);
  EXPECT_TRUE(t.size() == 2);
}

TEST(Art, insert_search) {
  art_tree<uintptr_t> t;
  int len;
  char buf[512];
  FILE* f = open_test_file("watchman/thirdparty/libart/tests/words.txt");
  uintptr_t line = 1;

  while (fgets(buf, sizeof buf, f)) {
    len = (int)strlen(buf);
    buf[len - 1] = '\0';
    t.insert(std::string(buf), line);
    line++;
  }

  // Seek back to the start
  fseek(f, 0, SEEK_SET);

  // Search for each line
  line = 1;
  while (fgets(buf, sizeof buf, f)) {
    len = (int)strlen(buf);
    buf[len - 1] = '\0';

    {
      uintptr_t val = *t.search(buf);
      EXPECT_EQ(line, val) << "Line: " << line << " Val: " << val
                           << " Str: " << buf;
    }
    line++;
  }

  // Check the minimum
  auto l = t.minimum();
  EXPECT_TRUE(l && l->key == "A");

  // Check the maximum
  l = t.maximum();
  EXPECT_TRUE(l && l->key == "zythum");
}

TEST(Art, insert_delete) {
  art_tree<uintptr_t> t;
  int len;
  char buf[512];
  FILE* f = open_test_file("watchman/thirdparty/libart/tests/words.txt");

  uintptr_t line = 1, nlines;
  while (fgets(buf, sizeof buf, f)) {
    len = (int)strlen(buf);
    buf[len - 1] = '\0';
    t.insert(buf, line);
    line++;
  }

  nlines = line - 1;

  // Seek back to the start
  fseek(f, 0, SEEK_SET);

  // Search for each line
  line = 1;
  while (fgets(buf, sizeof buf, f)) {
    uintptr_t val;
    len = (int)strlen(buf);
    buf[len - 1] = '\0';

    // Search first, ensure all entries still
    // visible
    val = *t.search(buf);
    EXPECT_EQ(line, val) << "Line: " << line << " Val: " << val
                         << " Str: " << buf;

    // Delete, should get lineno back
    EXPECT_TRUE(t.erase(buf))
        << "failed to erase line " << line << " str: " << buf;

    // Check the size
    EXPECT_EQ(t.size(), nlines - line) << "bad size after delete";
    line++;
  }

  // Check the minimum and maximum
  EXPECT_TRUE(!t.minimum());
  EXPECT_TRUE(!t.maximum());
}

TEST(Art, insert_iter) {
  art_tree<uintptr_t> t;

  int len;
  char buf[512];
  FILE* f = open_test_file("watchman/thirdparty/libart/tests/words.txt");

  uint64_t xor_mask = 0;
  uintptr_t lineno = 1, nlines;
  while (fgets(buf, sizeof buf, f)) {
    len = (int)strlen(buf);
    buf[len - 1] = '\0';
    t.insert(buf, lineno);

    xor_mask ^= (lineno * (buf[0] + len - 1));
    lineno++;
  }
  nlines = lineno - 1;

  {
    uint64_t out[] = {0, 0};
    EXPECT_TRUE(t.iter([&out](const std::string& key, uintptr_t& line) {
      uint64_t mask = (line * (key[0] + key.size()));
      out[0]++;
      out[1] ^= mask;
      return 0;
    }) == 0);

    EXPECT_TRUE(out[0] == nlines);
    EXPECT_TRUE(out[1] == xor_mask);
  }
}

template <typename T>
struct prefix_data {
  int count;
  int max_count;
  const char** expected;

  int operator()(const std::string& k, T&) {
    EXPECT_TRUE(count < max_count);
    XLOG(ERR) << "Key: " << k << " Expect: " << expected[count];
    EXPECT_TRUE(memcmp(k.data(), expected[count], k.size()) == 0);
    count++;
    return 0;
  }
};

TEST(Art, iter_prefix) {
  art_tree<void*> t;
  const char* expected2[] = {
      "abc.123.456",
      "api",
      "api.foe.fum",
      "api.foo",
      "api.foo.bar",
      "api.foo.baz"};

  t.insert("api.foo.bar", nullptr);
  t.insert("api.foo.baz", nullptr);
  t.insert("api.foe.fum", nullptr);
  t.insert("abc.123.456", nullptr);
  t.insert("api.foo", nullptr);
  t.insert("api", nullptr);

  {
    // Iterate over api
    const char* expected[] = {
        "api", "api.foe.fum", "api.foo", "api.foo.bar", "api.foo.baz"};
    prefix_data<void*> p = {0, 5, expected};
    EXPECT_TRUE(!t.iterPrefix((unsigned char*)"api", 3, p));
    XLOG(ERR) << "Count: " << p.count << " Max: " << p.max_count;
    EXPECT_TRUE(p.count == p.max_count);
  }

  {
    // Iterate over 'a'
    prefix_data<void*> p2 = {0, 6, expected2};
    EXPECT_TRUE(!t.iterPrefix((unsigned char*)"a", 1, p2));
    EXPECT_TRUE(p2.count == p2.max_count);
  }

  {
    // Check a failed iteration
    prefix_data<void*> p3 = {0, 0, nullptr};
    EXPECT_TRUE(!t.iterPrefix((unsigned char*)"b", 1, p3));
    EXPECT_TRUE(p3.count == 0);
  }

  {
    // Iterate over api.
    const char* expected4[] = {
        "api.foe.fum", "api.foo", "api.foo.bar", "api.foo.baz"};
    prefix_data<void*> p4 = {0, 4, expected4};
    EXPECT_TRUE(!t.iterPrefix((unsigned char*)"api.", 4, p4));
    XLOG(ERR) << "Count: " << p4.count << " Max: " << p4.max_count;
    EXPECT_TRUE(p4.count == p4.max_count);
  }

  {
    // Iterate over api.foo.ba
    const char* expected5[] = {"api.foo.bar"};
    prefix_data<void*> p5 = {0, 1, expected5};
    EXPECT_TRUE(!t.iterPrefix((unsigned char*)"api.foo.bar", 11, p5));
    XLOG(ERR) << "Count: " << p5.count << " Max: " << p5.max_count;
    EXPECT_TRUE(p5.count == p5.max_count);
  }

  // Check a failed iteration on api.end
  {
    prefix_data<void*> p6 = {0, 0, nullptr};
    EXPECT_TRUE(!t.iterPrefix((unsigned char*)"api.end", 7, p6));
    EXPECT_TRUE(p6.count == 0);
  }

  // Iterate over empty prefix
  {
    prefix_data<void*> p7 = {0, 6, expected2};
    EXPECT_TRUE(!t.iterPrefix((unsigned char*)"", 0, p7));
    EXPECT_TRUE(p7.count == p7.max_count);
  }
}

TEST(Art, long_prefix) {
  art_tree<uintptr_t> t;

  t.insert("this:key:has:a:long:prefix:3", 3);
  t.insert("this:key:has:a:long:common:prefix:2", 2);
  t.insert("this:key:has:a:long:common:prefix:1", 1);

  // Search for the keys
  EXPECT_TRUE(1 == *t.search("this:key:has:a:long:common:prefix:1"));
  EXPECT_TRUE(2 == *t.search("this:key:has:a:long:common:prefix:2"));
  EXPECT_TRUE(3 == *t.search("this:key:has:a:long:prefix:3"));

  {
    const char* expected[] = {
        "this:key:has:a:long:common:prefix:1",
        "this:key:has:a:long:common:prefix:2",
        "this:key:has:a:long:prefix:3",
    };
    prefix_data<uintptr_t> p = {0, 3, expected};
    EXPECT_TRUE(!t.iterPrefix((unsigned char*)"this:key:has", 12, p));
    XLOG(ERR) << "Count: " << p.count << " Max: " << p.max_count;
    EXPECT_TRUE(p.count == p.max_count);
  }
}

TEST(Art, prefix) {
  art_tree<void*> t;
  void* v;

  t.insert("food", (void*)"food");
  t.insert("foo", (void*)"foo");
  XLOG(ERR) << "size is now " << t.size();
  EXPECT_TRUE(t.size() == 2);
  EXPECT_TRUE((v = *t.search("food")) != nullptr);
  XLOG(ERR) << "food lookup yields " << v;
  EXPECT_TRUE(v && strcmp((char*)v, "food") == 0);

  t.iter([](const std::string& key, void*& value) {
    XLOG(ERR) << "iter leaf: key_len=" << key.size() << " " << key
              << " value=" << value;
    return 0;
  });

  EXPECT_TRUE((v = *t.search("foo")) != nullptr);
  XLOG(ERR) << "foo lookup yields " << v;
  EXPECT_TRUE(v && strcmp((char*)v, "foo") == 0);
}

TEST(Art, insert_search_uuid) {
  art_tree<uintptr_t> t;
  int len;
  char buf[512];
  FILE* f = open_test_file("watchman/thirdparty/libart/tests/uuid.txt");
  uintptr_t line = 1;

  while (fgets(buf, sizeof buf, f)) {
    len = (int)strlen(buf);
    buf[len - 1] = '\0';
    t.insert(buf, line);
    line++;
  }

  // Seek back to the start
  fseek(f, 0, SEEK_SET);

  // Search for each line
  line = 1;
  while (fgets(buf, sizeof buf, f)) {
    uintptr_t val;
    len = (int)strlen(buf);
    buf[len - 1] = '\0';

    val = *t.search(buf);
    EXPECT_EQ(line, val) << "Line: " << line << " Val: " << val
                         << " Str: " << buf;
    line++;
  }

  // Check the minimum
  auto l = t.minimum();
  XLOG(ERR) << "minimum is " << l->key;
  EXPECT_TRUE(l && l->key == "00026bda-e0ea-4cda-8245-522764e9f325");

  // Check the maximum
  l = t.maximum();
  XLOG(ERR) << "maximum is " << l->key;
  EXPECT_TRUE(l && l->key == "ffffcb46-a92e-4822-82af-a7190f9c1ec5");
}
