/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <folly/Benchmark.h>
#include <unordered_map>
#include <vector>
#include "squangle/mysql_client/Query.h"

using namespace facebook::common::mysql_client;

using folly::runBenchmarks;
using std::string;

const int kCount = 20000;
const int kNumericCount = 20000;

std::vector<string> keys;
std::string verylongstring; // not that much

void numericDynamic(int numericCount) {
  for (int i = 0; i < numericCount; ++i) {
    folly::dynamic v(0);
    v = int(1);
    v = double(2.0);
    v = short(3);
    v = int(1);
    v = double(2.0);
    v = short(3);
    v = int(1);
    v = double(2.0);
    v = short(3);
    v = int(1);
    v = double(2.0);
    v = short(3);
    v = int(1);
    v = double(2.0);
    v = short(3);
    v = int(1);
    v = double(2.0);
    v = short(3);
    v = int(1);
    v = double(2.0);
    v = short(3);
  }
}

void numericQueryArgument(int numericCount) {
  for (int i = 0; i < numericCount; ++i) {
    QueryArgument v;
    v = int(1);
    v = double(2.0);
    v = short(3);
    v = int(1);
    v = double(2.0);
    v = short(3);
    v = int(1);
    v = double(2.0);
    v = short(3);
    v = int(1);
    v = double(2.0);
    v = short(3);
    v = int(1);
    v = double(2.0);
    v = short(3);
    v = int(1);
    v = double(2.0);
    v = short(3);
    v = int(1);
    v = double(2.0);
    v = short(3);
  }
}

void stringDynamic(int count) {
  for (int i = 0; i < count; ++i) {
    folly::dynamic q(verylongstring);
    folly::dynamic q3("const char string");
  }
}

void stringQueryArgument(int count) {
  for (int i = 0; i < count; ++i) {
    QueryArgument q(verylongstring);
    QueryArgument q3("const char string");
  }
}

void vectorDynamic(int count) {
  for (int i = 0; i < count; ++i) {
    folly::dynamic q((folly::dynamic::array(
        "String value",
        10,
        2.0,
        "String value",
        10,
        2.0,
        folly::dynamic::object("key", 10),
        folly::dynamic::object("key", 10))));
  }
}

void vectorQueryArgument(int count) {
  for (int i = 0; i < count; ++i) {
    QueryArgument q(
        {"String value",
         10,
         2.0,
         "String value",
         10,
         2.0,
         QueryArgument("key", 10),
         QueryArgument("key", 10)});
  }
}

void keyPairDynamic(int count) {
  for (int i = 0; i < count; ++i) {
    folly::dynamic dyn =
        folly::dynamic::object("key1", verylongstring)(verylongstring, 10);
  }
}

void keyPairQueryArgument(int count) {
  for (int i = 0; i < count; ++i) {
    QueryArgument q = QueryArgument("key1", verylongstring)(verylongstring, 10);
  }
}

void simpleConversionDynamic(int /*count*/) {
  folly::dynamic q_int(10), q_double(2.0), q_string("hey, I'm Pusheen"),
      q_pair(folly::dynamic::object("col1", "Leaving la vila loca"));

  for (int i = 0; i < kCount; ++i) {
    uint64_t ii = q_int.isInt() ? q_int.getInt() : 0;
    double d = q_double.isDouble() ? q_double.getDouble() : 0;
    folly::fbstring s = q_string.isString() ? q_string.getString() : nullptr;
    folly::doNotOptimizeAway(ii);
    folly::doNotOptimizeAway(d);
    auto b = q_pair.isObject();
    folly::doNotOptimizeAway(b);

    auto s1 = q_int.asString();
    auto s2 = q_double.asString();
    auto s3 = q_string.asString();

    int hack = s.length() + s1.length() + s2.length() + s3.length();
    folly::doNotOptimizeAway(hack);
  }
}

void simpleConversionQueryArgument(int /*count*/) {
  QueryArgument q_int(10), q_double(2.0), q_string("hey, I'm Pusheen"),
      q_pair("col1", "Leaving la vila loca");

  for (int i = 0; i < kCount; ++i) {
    uint64_t ii = q_int.isInt() ? q_int.getInt() : 0;
    double d = q_double.isDouble() ? q_double.getDouble() : 0;
    folly::fbstring s = q_string.isString() ? q_string.getString() : nullptr;
    folly::doNotOptimizeAway(ii);
    folly::doNotOptimizeAway(d);
    auto b = q_pair.isPairList();
    folly::doNotOptimizeAway(b);

    auto s1 = q_int.asString();
    auto s2 = q_double.asString();
    auto s3 = q_string.asString();

    int hack = s.length() + s1.length() + s2.length() + s3.length();
    folly::doNotOptimizeAway(hack);
  }
}

BENCHMARK_NAMED_PARAM(numericDynamic, kNumericCount);
BENCHMARK_RELATIVE_NAMED_PARAM(numericQueryArgument, kNumericCount);
BENCHMARK_DRAW_LINE();

BENCHMARK_NAMED_PARAM(stringDynamic, kCount);
BENCHMARK_RELATIVE_NAMED_PARAM(stringQueryArgument, kCount);
BENCHMARK_DRAW_LINE();

BENCHMARK_NAMED_PARAM(vectorDynamic, kCount);
BENCHMARK_RELATIVE_NAMED_PARAM(vectorQueryArgument, kCount);
BENCHMARK_DRAW_LINE();

BENCHMARK_NAMED_PARAM(keyPairDynamic, kCount);
BENCHMARK_RELATIVE_NAMED_PARAM(keyPairQueryArgument, kCount);
BENCHMARK_DRAW_LINE();

BENCHMARK_NAMED_PARAM(simpleConversionDynamic, kCount);
BENCHMARK_RELATIVE_NAMED_PARAM(simpleConversionQueryArgument, kCount);
BENCHMARK_DRAW_LINE();

BENCHMARK(OverallDynamic) {
  numericDynamic(kNumericCount);
  stringDynamic(kCount);
  vectorDynamic(kCount);
  keyPairDynamic(kCount);
  simpleConversionDynamic(kCount);
}

BENCHMARK_RELATIVE(OverallQueryArgument) {
  numericQueryArgument(kNumericCount);
  stringQueryArgument(kCount);
  vectorQueryArgument(kCount);
  keyPairQueryArgument(kCount);
  simpleConversionDynamic(kCount);
}

BENCHMARK_DRAW_LINE();

#define QUERY(all_args)          \
  {                              \
    Query q all_args;            \
    auto s = q.renderInsecure(); \
  }

#define testArg QueryArgument
#define testObject QueryArgument

BENCHMARK(createQueryObject) {
  for (int i = 0; i < kCount; ++i) {
    QUERY(("SELECT * FROM table1 WHERE column3 = %d", 7));
    QUERY(
        ("SELECT * FROM table1 WHERE column3 = %d AND column5 = %s",
         14,
         "table1"));
    QUERY(("SELECT * FROM table1 WHERE %C = %f", "column1", 2.2));
    QUERY(
        ("SELECT * FROM table1 WHERE %W",
         testObject("column3", 17)("column5", "99")));
    QUERY(("SELECT * FROM %T WHERE column3 = %d", "table2", 7));
    QUERY(("SELECT %Ld FROM table1", testArg{1, 2, 3, 4, 5}));
    QUERY(
        ("SELECT COUNT(*) FROM table1 WHERE %LA",
         testObject("col1", "red")("col2", "blue")));
    QUERY(
        ("SELECT COUNT(*) FROM table1 WHERE %LO",
         testObject("col1", "red")("col2", "blue")));
    QUERY(
        ("SELECT COUNT(*) FROM table1 WHERE %LA",
         testObject("col1", "red")("col2", nullptr)));
    QUERY(
        ("SELECT COUNT(*) FROM table1 WHERE %LA AND %LO",
         testObject("color", "yellow")("species", "cat"),
         testObject("color", "brown")("species", "dog")));
    QUERY(
        ("UPDATE %T SET %U WHERE 1",
         "table2",
         testObject("column1", "red")("column2", "small")));
    QUERY(
        ("UPDATE %T SET col1 = value1 WHERE %W",
         "table2",
         testObject("column3", 7)("column4", 14)));
    QUERY(("UPDATE %T SET col1 = %s, col2 = %d", "table2", "twelve", nullptr));
    QUERY(("UPDATE %T SET col1 = %s, col2 = %d", "table2", "twelve", nullptr));
    QUERY(("SELECT %LC FROM %T", testArg{"col1", "col2"}, "table"));
    QUERY(("SELECT %K col1 FROM table1", "my /* magic */ comment"));

    {
      testArg vlist = {testArg{1, "foo", 0.2, 2}, testArg{2, "bar", 0.4, 5}};
      QUERY(
          ("INSERT INTO %T (%C, %C, %C) VALUES %V",
           "table1",
           "a",
           "b",
           "c",
           std::move(vlist)));
    }
  }
}

int main(int /*argc*/, char** /*argv*/) {
  int keys_count = 1000;
  for (int i = 0; i < keys_count; ++i) {
    keys.push_back(
        "this is a key and we are having fun " + folly::to<std::string>(i));
  }

  verylongstring = std::string(1000, '*');

  runBenchmarks();
  return 0;
}
