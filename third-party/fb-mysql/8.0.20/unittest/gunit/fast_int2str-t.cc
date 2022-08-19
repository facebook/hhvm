#include <gtest/gtest.h>
#include <stddef.h>
#include <stdint.h>
#include <limits>

extern char *u32toa_jeaiii(uint32_t u, char *b);
extern char *i32toa_jeaiii(int32_t i, char *b);
extern char *u64toa_jeaiii(uint64_t n, char *b);
extern char *i64toa_jeaiii(int64_t i, char *b);
extern size_t u32toa_jeaiii_n(uint32_t u, char *b, const size_t buflen);
extern size_t i32toa_jeaiii_n(int32_t i, char *b, const size_t buflen);
extern size_t u64toa_jeaiii_n(uint64_t n, char *b, const size_t buflen);
extern size_t i64toa_jeaiii_n(int64_t i, char *b, const size_t buflen);

char *u32toa_sprintf(uint32_t u, char *b) {
  sprintf(b, "%u", u);
  return b;
}
char *i32toa_sprintf(int32_t i, char *b) {
  sprintf(b, "%d", i);
  return b;
}
char *u64toa_sprintf(uint64_t n, char *b) {
  sprintf(b, "%llu", (unsigned long long)n);
  return b;
}
char *i64toa_sprintf(int64_t i, char *b) {
  sprintf(b, "%lld", (long long)i);
  return b;
}
size_t u32toa_sprintf_n(uint32_t u, char *b, const size_t buflen) {
  u32toa_sprintf(u, b);
  b[buflen] = 0;
  return strlen(b);
}
size_t i32toa_sprintf_n(int32_t i, char *b, const size_t buflen) {
  i32toa_sprintf(i, b);
  b[buflen] = 0;
  return strlen(b);
}
size_t u64toa_sprintf_n(uint64_t n, char *b, const size_t buflen) {
  u64toa_sprintf(n, b);
  b[buflen] = 0;
  return strlen(b);
}
size_t i64toa_sprintf_n(int64_t i, char *b, const size_t buflen) {
  i64toa_sprintf(i, b);
  b[buflen] = 0;
  return strlen(b);
}

struct TestWithParam {
  char buf1[32];
  char buf2[32];
  TestWithParam() {
    memset(buf1, 0, sizeof(buf1));
    memset(buf2, 0, sizeof(buf2));
  }
};

struct TestWithParamUInt64 : public TestWithParam,
                             ::testing::TestWithParam<uint64_t> {};
struct TestWithParamInt64 : public TestWithParam,
                            ::testing::TestWithParam<int64_t> {};
struct TestWithParamUInt32 : public TestWithParam,
                             ::testing::TestWithParam<uint32_t> {};
struct TestWithParamInt32 : public TestWithParam,
                            ::testing::TestWithParam<int32_t> {};
struct TestWithParamLen : public TestWithParam,
                          ::testing::TestWithParam<size_t> {};
struct TestNoParam : public TestWithParam, ::testing::Test {};

TEST_P(TestWithParamUInt64, TestUInt64) {
  u64toa_sprintf((uint64_t)GetParam(), buf1);
  u64toa_jeaiii((uint64_t)GetParam(), buf2);
  ASSERT_STREQ(buf1, buf2);
}
TEST_P(TestWithParamInt64, TestInt64) {
  i64toa_sprintf((int64_t)GetParam(), buf1);
  i64toa_jeaiii((int64_t)GetParam(), buf2);
  ASSERT_STREQ(buf1, buf2);
}
TEST_P(TestWithParamUInt32, TestUInt32) {
  u32toa_sprintf((uint32_t)GetParam(), buf1);
  u32toa_jeaiii((uint32_t)GetParam(), buf2);
  ASSERT_STREQ(buf1, buf2);
}
TEST_P(TestWithParamInt32, TestInt32) {
  i32toa_sprintf((int32_t)GetParam(), buf1);
  i32toa_jeaiii((int32_t)GetParam(), buf2);
  ASSERT_STREQ(buf1, buf2);
}
TEST_P(TestWithParamUInt64, TestUInt64N) {
  u64toa_sprintf_n((uint64_t)GetParam(), buf1, sizeof(buf1) - 1);
  u64toa_jeaiii_n((uint64_t)GetParam(), buf2, sizeof(buf2) - 1);
  ASSERT_STREQ(buf1, buf2);
}
TEST_P(TestWithParamInt64, TestInt64N) {
  i64toa_sprintf_n((int64_t)GetParam(), buf1, sizeof(buf1) - 1);
  i64toa_jeaiii_n((int64_t)GetParam(), buf2, sizeof(buf2) - 1);
  ASSERT_STREQ(buf1, buf2);
}
TEST_P(TestWithParamUInt32, TestUInt32N) {
  u32toa_sprintf_n((uint32_t)GetParam(), buf1, sizeof(buf1) - 1);
  u32toa_jeaiii_n((uint32_t)GetParam(), buf2, sizeof(buf2) - 1);
  ASSERT_STREQ(buf1, buf2);
}
TEST_P(TestWithParamInt32, TestInt32N) {
  i32toa_sprintf_n((int32_t)GetParam(), buf1, sizeof(buf1) - 1);
  i32toa_jeaiii_n((int32_t)GetParam(), buf2, sizeof(buf2) - 1);
  ASSERT_STREQ(buf1, buf2);
}

TEST_P(TestWithParamLen, TestShortBufferInt32) {
  const size_t buflen = GetParam();
  i32toa_sprintf_n(1234567890, buf1, buflen);
  i32toa_jeaiii_n(1234567890, buf2, buflen);
  ASSERT_STREQ(buf1, buf2);
}
TEST_P(TestWithParamLen, TestShortBufferInt32Neg) {
  const size_t buflen = GetParam();
  i32toa_sprintf_n(-1234567890, buf1, buflen);
  i32toa_jeaiii_n(-1234567890, buf2, buflen);
  ASSERT_STREQ(buf1, buf2);
}
TEST_P(TestWithParamLen, TestShortBufferUInt32) {
  const size_t buflen = GetParam();
  u32toa_sprintf_n(1234567890, buf1, buflen);
  u32toa_jeaiii_n(1234567890, buf2, buflen);
  ASSERT_STREQ(buf1, buf2);
}
TEST_P(TestWithParamLen, TestShortBufferInt64) {
  const size_t buflen = GetParam();
  i64toa_sprintf_n(1234567890123456789LL, buf1, buflen);
  i64toa_jeaiii_n(1234567890123456789LL, buf2, buflen);
  ASSERT_STREQ(buf1, buf2);
}
TEST_P(TestWithParamLen, TestShortBufferInt64Neg) {
  const size_t buflen = GetParam();
  i64toa_sprintf_n(-1234567890123456789LL, buf1, buflen);
  i64toa_jeaiii_n(-1234567890123456789LL, buf2, buflen);
  ASSERT_STREQ(buf1, buf2);
}
TEST_P(TestWithParamLen, TestShortBufferUInt64) {
  const size_t buflen = GetParam();
  u64toa_sprintf_n(1234567890123456789ULL, buf1, buflen);
  u64toa_jeaiii_n(1234567890123456789ULL, buf2, buflen);
  ASSERT_STREQ(buf1, buf2);
}

TEST_F(TestNoParam, TestAllInt32) {
  const int32_t min = std::numeric_limits<int32_t>::min();
  const int32_t max = std::numeric_limits<int32_t>::max();

  const int k = 100000000;
  int32_t x = min;
  int32_t p = min + k;
  while (true) {
    i32toa_sprintf(x, buf1);
    i32toa_jeaiii(x, buf2);
    ASSERT_STREQ(buf1, buf2);
    if (x == max) break;
    if (x < (max - 100))
      x += 100;
    else
      x++;
    if (x == p) {
      // printf("%u\n", (unsigned)(p - min));
      p += k;
    }
  }
}

INSTANTIATE_TEST_CASE_P(
    TestLimitsUInt64, TestWithParamUInt64,
    ::testing::Values(0, 1, 10, 100, 1000, 10000, 100000, 1000000, 10000000,
                      100000000, 1000000000, 10000000000, 100000000000,
                      1000000000000, std::numeric_limits<uint64_t>::min(),
                      std::numeric_limits<uint64_t>::max()));

INSTANTIATE_TEST_CASE_P(TestLimitsInt64, TestWithParamInt64,
                        ::testing::Values(0, 1, 10, 100, 1000, 10000, 100000,
                                          1000000, 10000000, 100000000,
                                          1000000000, 10000000000, 100000000000,
                                          1000000000000, -1, -10, -100, -1000,
                                          -10000, -100000, -1000000, -10000000,
                                          -100000000, -1000000000, -10000000000,
                                          -100000000000, -1000000000000,
                                          std::numeric_limits<int64_t>::min(),
                                          std::numeric_limits<int64_t>::max()));

INSTANTIATE_TEST_CASE_P(
    TestLimitsUInt32, TestWithParamUInt32,
    ::testing::Values(0, 1, 10, 100, 1000, 10000, 100000, 1000000, 10000000,
                      100000000, 1000000000,
                      std::numeric_limits<uint32_t>::min(),
                      std::numeric_limits<uint32_t>::max()));

INSTANTIATE_TEST_CASE_P(TestLimitsInt32, TestWithParamInt32,
                        ::testing::Values(0, 1, 10, 100, 1000, 10000, 100000,
                                          1000000, 10000000, 100000000,
                                          1000000000, -1, -10, -100, -1000,
                                          -10000, -100000, -1000000, -10000000,
                                          -100000000, -1000000000,
                                          std::numeric_limits<int32_t>::min(),
                                          std::numeric_limits<int32_t>::max()));

INSTANTIATE_TEST_CASE_P(TestShortBufLen, TestWithParamLen,
                        ::testing::Range(size_t(0), size_t(23)));
