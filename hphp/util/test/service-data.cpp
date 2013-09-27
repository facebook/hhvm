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

#include <atomic>
#include "hphp/util/service-data.h"
#include <gtest/gtest.h>
namespace HPHP {

TEST(ServiceDataTest, CounterTest) {
  // Simple counter test.
  auto counter = ServiceData::createCounter("c1");
  counter->increment();
  {
    std::map<std::string, int64_t> values;
    ServiceData::exportAll(values);
    EXPECT_EQ(1, values["c1"]);
  }
  counter->increment();
  counter->increment();
  {
    std::map<std::string, int64_t> values;
    ServiceData::exportAll(values);
    EXPECT_EQ(3, values["c1"]);
  }
  counter->setValue(0);
  {
    std::map<std::string, int64_t> values;
    ServiceData::exportAll(values);
    EXPECT_EQ(0, values["c1"]);
  }

  // Multiple counters.
  auto counter1 = ServiceData::createCounter("c2");
  counter->increment();
  counter1->setValue(5);
  {
    std::map<std::string, int64_t> values;
    ServiceData::exportAll(values);
    EXPECT_EQ(1, values["c1"]);
    EXPECT_EQ(5, values["c2"]);
  }

  // Multiple counter object to the same underlying counter.
  auto counter2 = ServiceData::createCounter("c2");
  counter1->setValue(5);
  counter2->increment();
  counter1->increment();
  ServiceData::createCounter("c2")->increment();
  {
    std::map<std::string, int64_t> values;
    ServiceData::exportAll(values);
    EXPECT_EQ(8, values["c2"]);
  }
}

TEST(ServiceDataTest, TimeSeriesTest) {
  auto statsType = {
    ServiceData::StatsType::AVG,
    ServiceData::StatsType::SUM,
    ServiceData::StatsType::COUNT,
    ServiceData::StatsType::RATE
  };

  auto ts = ServiceData::createTimeseries("foo", statsType);
  ts->addValue(1);
  ts->addValue(1);

  {
    std::map<std::string, int64_t> values;
    ServiceData::exportAll(values);
    EXPECT_EQ(1, values["foo.avg"]);
    EXPECT_EQ(1, values["foo.avg.60"]);
    EXPECT_EQ(1, values["foo.avg.600"]);
    EXPECT_EQ(1, values["foo.avg.3600"]);

    EXPECT_EQ(2, values["foo.sum"]);
    EXPECT_EQ(2, values["foo.sum.60"]);
    EXPECT_EQ(2, values["foo.sum.600"]);
    EXPECT_EQ(2, values["foo.sum.3600"]);

    EXPECT_EQ(2, values["foo.count"]);
    EXPECT_EQ(2, values["foo.count.60"]);
    EXPECT_EQ(2, values["foo.count.600"]);
    EXPECT_EQ(2, values["foo.count.3600"]);

    EXPECT_EQ(2, values["foo.rate"]);
    EXPECT_EQ(2, values["foo.rate.60"]);
    EXPECT_EQ(2, values["foo.rate.600"]);
    EXPECT_EQ(2, values["foo.rate.3600"]);
  }
}

TEST(ServiceDataTest, Histogram) {
  auto hist = ServiceData::createHistogram(
    "foo", 1, 0, 100,
    {0.05, 0.5, 0.75, 0.95});

  for (int i = 0; i < 100; ++i) {
    hist->addValue(i);
  }

  {
    std::map<std::string, int64_t> values;
    ServiceData::exportAll(values);
    EXPECT_EQ(5, values["foo.hist.p5"]);
    EXPECT_EQ(50, values["foo.hist.p50"]);
    EXPECT_EQ(75, values["foo.hist.p75"]);
    EXPECT_EQ(95, values["foo.hist.p95"]);
  }
}

}
