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
#include <folly/portability/GTest.h>

#include "hphp/runtime/base/type-string.h"
#include "hphp/runtime/base/static-string-table.h"
#include "hphp/util/service-data.h"

namespace HPHP {

namespace {
int getVal(const char* key) {
    std::map<std::string, int64_t> values;
    ServiceData::exportAll(values);
    return values[key];
}
}

TEST(COUNTERS, static_string) {
    int ss = getVal("admin.static-strings");
    EXPECT_EQ(ss, makeStaticStringCount());

    makeStaticString("bananas");
    ++ss;
    EXPECT_EQ(ss, getVal("admin.static-strings"));
    EXPECT_EQ(ss, makeStaticStringCount());

    refineStaticStringTableSize();
    EXPECT_EQ(ss, getVal("admin.static-strings"));
    EXPECT_EQ(ss, makeStaticStringCount());
}

}
