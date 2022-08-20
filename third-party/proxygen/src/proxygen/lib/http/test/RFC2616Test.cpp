/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <proxygen/lib/http/RFC2616.h>

#include <folly/portability/GTest.h>

using namespace proxygen;

using RFC2616::parseByteRangeSpec;
using std::string;

TEST(QvalueTest, Basic) {

  RFC2616::TokenPairVec output;

  {
    string test("iso-8859-5, unicode-1-1;q=0.8");
    EXPECT_TRUE(RFC2616::parseQvalues(test, output));
    EXPECT_EQ(output.size(), 2);
    EXPECT_EQ(output[0].first.compare(folly::StringPiece("iso-8859-5")), 0);
    EXPECT_DOUBLE_EQ(output[0].second, 1);
    EXPECT_EQ(output[1].first.compare(folly::StringPiece("unicode-1-1")), 0);
    EXPECT_DOUBLE_EQ(output[1].second, 0.8);
    output.clear();
  }

  {
    string test("compress, gzip");
    EXPECT_TRUE(RFC2616::parseQvalues(test, output));
    EXPECT_EQ(output.size(), 2);
    EXPECT_EQ(output[0].first.compare(folly::StringPiece("compress")), 0);
    EXPECT_DOUBLE_EQ(output[0].second, 1);
    EXPECT_EQ(output[1].first.compare(folly::StringPiece("gzip")), 0);
    EXPECT_DOUBLE_EQ(output[1].second, 1);
    output.clear();
  }

  {
    string test("");
    // The spec says a blank one is ok but empty headers are disallowed in SPDY?
    EXPECT_FALSE(RFC2616::parseQvalues(test, output));
    EXPECT_EQ(output.size(), 0);
  }

  {
    string test(" ");
    // The spec says a blank one is ok but empty headers are disallowed in SPDY?
    EXPECT_FALSE(RFC2616::parseQvalues(test, output));
    EXPECT_EQ(output.size(), 0);
  }

  {
    string test("compress;q=0.5, gzip;q=1.0");
    EXPECT_TRUE(RFC2616::parseQvalues(test, output));
    EXPECT_EQ(output.size(), 2);
    EXPECT_EQ(output[0].first.compare(folly::StringPiece("compress")), 0);
    EXPECT_DOUBLE_EQ(output[0].second, 0.5);
    EXPECT_EQ(output[1].first.compare(folly::StringPiece("gzip")), 0);
    EXPECT_DOUBLE_EQ(output[1].second, 1.0);
    output.clear();
  }

  {
    string test("gzip;q=1.0, identity; q=0.5, *;q=0");
    EXPECT_TRUE(RFC2616::parseQvalues(test, output));
    EXPECT_EQ(output.size(), 3);
    EXPECT_EQ(output[0].first.compare(folly::StringPiece("gzip")), 0);
    EXPECT_DOUBLE_EQ(output[0].second, 1);
    EXPECT_EQ(output[1].first.compare(folly::StringPiece("identity")), 0);
    EXPECT_DOUBLE_EQ(output[1].second, 0.5);
    EXPECT_EQ(output[2].first.compare(folly::StringPiece("*")), 0);
    EXPECT_DOUBLE_EQ(output[2].second, 0);
    output.clear();
  }

  {
    string test("da, en-gb;q=0.8, en;q=0.7");
    EXPECT_TRUE(RFC2616::parseQvalues(test, output));
    EXPECT_EQ(output.size(), 3);
    EXPECT_EQ(output[0].first.compare(folly::StringPiece("da")), 0);
    EXPECT_DOUBLE_EQ(output[0].second, 1);
    EXPECT_EQ(output[1].first.compare(folly::StringPiece("en-gb")), 0);
    EXPECT_DOUBLE_EQ(output[1].second, 0.8);
    EXPECT_EQ(output[2].first.compare(folly::StringPiece("en")), 0);
    EXPECT_DOUBLE_EQ(output[2].second, 0.7);
    output.clear();
  }
}

TEST(QvalueTest, Extras) {

  RFC2616::TokenPairVec output;

  string test("gzip");
  EXPECT_TRUE(RFC2616::parseQvalues(test, output));
  EXPECT_EQ(output.size(), 1);
  EXPECT_EQ(output[0].first.compare(folly::StringPiece("gzip")), 0);
  EXPECT_DOUBLE_EQ(output[0].second, 1);
  output.clear();
}

TEST(QvalueTest, Invalids) {

  RFC2616::TokenPairVec output;

  string test1(",,,");
  EXPECT_FALSE(RFC2616::parseQvalues(test1, output));
  EXPECT_EQ(output.size(), 0);
  output.clear();

  string test2("  ; q=0.1");
  EXPECT_FALSE(RFC2616::parseQvalues(test2, output));
  EXPECT_EQ(output.size(), 0);
  output.clear();

  string test3("gzip; q=uietplease");
  EXPECT_FALSE(RFC2616::parseQvalues(test3, output));
  EXPECT_EQ(output.size(), 1);
  EXPECT_EQ(output[0].first.compare(folly::StringPiece("gzip")), 0);
  EXPECT_DOUBLE_EQ(output[0].second, 1);
  output.clear();
}

TEST(ParseEncodingTest, Simple) {
  string test("zstd;q=1.0;wl=20,gzip;q=0.0");
  auto encodings = RFC2616::parseEncoding(test);
  EXPECT_FALSE(encodings.hasException());
  EXPECT_EQ(encodings.value(),
            (RFC2616::EncodingList{{"zstd", {{"q", "1.0"}, {"wl", "20"}}},
                                   {"gzip", {{"q", "0.0"}}}}));
}

TEST(ParseEncodingTest, Whitespace) {
  string test("zstd ; q =\t1.0 ; wl = 20 , gzip ;\t q = 0.0");
  auto encodings = RFC2616::parseEncoding(test);
  EXPECT_FALSE(encodings.hasException());
  EXPECT_EQ(encodings.value(),
            (RFC2616::EncodingList{{"zstd", {{"q", "1.0"}, {"wl", "20"}}},
                                   {"gzip", {{"q", "0.0"}}}}));
}

TEST(ParseEncodingTest, Invalid) {
  EXPECT_TRUE(RFC2616::parseEncoding("").hasException());
  EXPECT_TRUE(RFC2616::parseEncoding(" ").hasException());
  EXPECT_TRUE(RFC2616::parseEncoding(",").hasException());
  EXPECT_TRUE(RFC2616::parseEncoding(" ,").hasException());
  EXPECT_TRUE(RFC2616::parseEncoding(", ").hasException());
  EXPECT_TRUE(RFC2616::parseEncoding(" , ").hasException());
  EXPECT_TRUE(RFC2616::parseEncoding(" , , ").hasException());
  EXPECT_TRUE(RFC2616::parseEncoding(";").hasException());
  EXPECT_TRUE(RFC2616::parseEncoding(" ;").hasException());
  EXPECT_TRUE(RFC2616::parseEncoding("; ").hasException());
  EXPECT_TRUE(RFC2616::parseEncoding(" ; ").hasException());
  EXPECT_TRUE(RFC2616::parseEncoding(" ; ; ").hasException());
  EXPECT_TRUE(RFC2616::parseEncoding(" ; ; , ; ;").hasException());

  EXPECT_TRUE(RFC2616::parseEncoding("zstd;=,gzip").hasException());
  EXPECT_TRUE(RFC2616::parseEncoding("zstd;=wat,gzip").hasException());
}

TEST(AcceptEncodingTest, Simple) {
  EXPECT_TRUE(RFC2616::acceptsEncoding("zstd", "zstd"));
  EXPECT_TRUE(RFC2616::acceptsEncoding("zstd, gzip", "zstd"));
  EXPECT_FALSE(RFC2616::acceptsEncoding("gzip", "zstd"));
  EXPECT_FALSE(RFC2616::acceptsEncoding("*", "zstd"));
}

TEST(AcceptEncodingTest, QValues) {
  EXPECT_TRUE(RFC2616::acceptsEncoding("zstd", "zstd"));
  EXPECT_TRUE(RFC2616::acceptsEncoding("zstd;q=1.0", "zstd"));
  EXPECT_TRUE(RFC2616::acceptsEncoding("zstd;q=wat", "zstd"));
  EXPECT_FALSE(RFC2616::acceptsEncoding("zstd;q=0.0", "zstd"));
}

TEST(AcceptEncodingTest, Whitespace) {
  EXPECT_TRUE(RFC2616::acceptsEncoding(" zstd ", "zstd"));
  EXPECT_TRUE(RFC2616::acceptsEncoding(" zstd ,", "zstd"));
  EXPECT_TRUE(RFC2616::acceptsEncoding("gzip, br \t, \tzstd ", "zstd"));
  EXPECT_FALSE(RFC2616::acceptsEncoding("gzip, \tdeflate, br \t ", "zstd"));
  EXPECT_FALSE(RFC2616::acceptsEncoding("zstd;q=0.0", "zstd"));
  EXPECT_FALSE(RFC2616::acceptsEncoding("zstd; q = 0.0 ", "zstd"));
}

TEST(ByteRangeSpecTest, Valids) {
  unsigned long firstByte = ULONG_MAX;
  unsigned long lastByte = ULONG_MAX;
  unsigned long instanceLength = ULONG_MAX;

  ASSERT_TRUE(parseByteRangeSpec(
      "bytes 0-10/100", firstByte, lastByte, instanceLength));
  EXPECT_EQ(0, firstByte);
  EXPECT_EQ(10, lastByte);
  EXPECT_EQ(100, instanceLength);

  ASSERT_TRUE(
      parseByteRangeSpec("bytes */100", firstByte, lastByte, instanceLength));
  EXPECT_EQ(0, firstByte);
  EXPECT_EQ(ULONG_MAX, lastByte);
  EXPECT_EQ(100, instanceLength);

  ASSERT_TRUE(
      parseByteRangeSpec("bytes 0-10/*", firstByte, lastByte, instanceLength));
  EXPECT_EQ(0, firstByte);
  EXPECT_EQ(10, lastByte);
  EXPECT_EQ(ULONG_MAX, instanceLength);
}

TEST(ByteRangeSpecTest, Invalids) {
  unsigned long dummy;

  EXPECT_FALSE(parseByteRangeSpec("0-10/100", dummy, dummy, dummy))
      << "Spec must start with 'bytes '";
  EXPECT_FALSE(parseByteRangeSpec("bytes 10/100", dummy, dummy, dummy))
      << "Spec missing initial range";
  EXPECT_FALSE(parseByteRangeSpec("bytes 10-/100", dummy, dummy, dummy))
      << "Spec missing last byte in initial range";
  EXPECT_FALSE(parseByteRangeSpec("bytes 0-10 100", dummy, dummy, dummy))
      << "Spec missing '/' separator";
  EXPECT_FALSE(parseByteRangeSpec("bytes 0-10/100Q", dummy, dummy, dummy))
      << "Spec has trailing garbage";
  EXPECT_FALSE(parseByteRangeSpec("bytes 10-1/100", dummy, dummy, dummy))
      << "Spec initial range is invalid";
  EXPECT_FALSE(parseByteRangeSpec("bytes 10-90/50", dummy, dummy, dummy))
      << "Spec initial range is invalid too large";
  EXPECT_FALSE(parseByteRangeSpec("bytes x/100", dummy, dummy, dummy))
      << "Spec initial range has invalid first byte";
  EXPECT_FALSE(parseByteRangeSpec("bytes 0-x/100", dummy, dummy, dummy))
      << "Spec initial range has invalid last bytek";
  EXPECT_FALSE(parseByteRangeSpec("bytes *-10/100", dummy, dummy, dummy))
      << "Spec cannot contain wildcard in initial range";
  EXPECT_FALSE(parseByteRangeSpec("bytes 0-*/100", dummy, dummy, dummy))
      << "Spec cannot contain wildcard in initial range";

  folly::StringPiece sp("bytes 0-10/100");
  sp.subtract(3);
  EXPECT_FALSE(parseByteRangeSpec(sp, dummy, dummy, dummy))
      << "Spec StringPiece ends before instance length";
  sp.subtract(1);
  EXPECT_FALSE(parseByteRangeSpec(sp, dummy, dummy, dummy))
      << "Spec StringPiece ends before '/' character";
  sp.subtract(2);
  EXPECT_FALSE(parseByteRangeSpec(sp, dummy, dummy, dummy))
      << "Spec StringPiece ends before last byte in initial byte range";
  sp.subtract(1);
  EXPECT_FALSE(parseByteRangeSpec(sp, dummy, dummy, dummy))
      << "Spec StringPiece ends before '-' in initial byte range";
  sp.subtract(2);
  EXPECT_FALSE(parseByteRangeSpec(sp, dummy, dummy, dummy))
      << "Spec StringPiece ends before first byte in initial byte range";
}
