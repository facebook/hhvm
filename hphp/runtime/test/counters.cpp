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

#include "hphp/runtime/base/array-init.h"
#include "hphp/runtime/base/preg.h"
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
    const char* key = "admin.static-strings";
    int ss = getVal(key);
    EXPECT_EQ(ss, makeStaticStringCount());

    makeStaticString("bananas");
    ++ss;
    EXPECT_EQ(ss, getVal(key));
    EXPECT_EQ(ss, makeStaticStringCount());

    refineStaticStringTableSize();
    EXPECT_EQ(ss, getVal(key));
    EXPECT_EQ(ss, makeStaticStringCount());
}

TEST(COUNTERS, pcre_cache) {
    const char* key = "admin.pcre-cache";
    int ss = getVal(key);
    EXPECT_EQ(ss, preg_pcre_cache_size());

    preg_grep(String{"/.ba./"}, make_vec_array(String{"ababu"}, String{"bananas"}));
    ++ss;
    EXPECT_EQ(ss, getVal(key));
    EXPECT_EQ(ss, preg_pcre_cache_size());

    // confirm we're correctly skipping when hitting the cache
    preg_grep(String{"/.ba./"}, make_vec_array(String{"ababu"}, String{"bananas"}));
    EXPECT_EQ(ss, getVal(key));
    EXPECT_EQ(ss, preg_pcre_cache_size());

    // kill table
    pcre_reinit();
    EXPECT_EQ(0, getVal(key));
    EXPECT_EQ(0, preg_pcre_cache_size());

}

TEST(COUNTERS, named_entities) {
  const char* key = "admin.named-entities";
    int ss = getVal(key);
    EXPECT_EQ(ss, namedEntityTableSize());

    const auto str = String{"MyNewHilariousType"};
    NamedType::get(str.get());
    ++ss;
    EXPECT_EQ(ss, getVal(key));
    EXPECT_EQ(ss, namedEntityTableSize());

    const auto str2 = String{"MyNewHilariousFunc"};
    NamedFunc::get(str2.get());
    ++ss;
    EXPECT_EQ(ss, getVal(key));
    EXPECT_EQ(ss, namedEntityTableSize());

}

}
