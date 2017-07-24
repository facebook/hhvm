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
#include "hphp/runtime/base/exceptions.h"
#include "hphp/runtime/base/memory-manager.h"
#include "hphp/runtime/base/program-functions.h"
#include "hphp/runtime/base/req-containers.h"
#include "hphp/runtime/base/surprise-flags.h"
#include "hphp/runtime/ext/json/JSON_parser.h"
#include "hphp/runtime/ext/json/ext_json.h"

namespace HPHP {

void test_json(const char* json) {
  auto old_limit = MM().getMemoryLimit();
  MM().setMemoryLimit(MM().getStatsCopy().usage() + 0x10000);
  auto caught = false;
  req::vector<Variant> buf;
  ASSERT_LE(MM().getStatsCopy().usage(), MM().getMemoryLimit());
  ASSERT_FALSE(getSurpriseFlag(MemExceededFlag));
  auto len = strlen(json);
  try {
    for (int i = 0; i < 100000; i++) {
      // call into SimpleParser
      Variant z;
      auto ok = JSON_parser(z, json, len, true, 0xffff,
                            k_JSON_FB_LOOSE);
      ASSERT_TRUE(ok);
      ASSERT_FALSE(getSurpriseFlag(MemExceededFlag));
      buf.push_back(z);
    }
  } catch (RequestMemoryExceededException& e) {
    caught = true;
  }
  EXPECT_TRUE(caught);
  MM().setMemoryLimit(old_limit);
  MM().resetCouldOOM();
}

TEST(JSON, simple_packed_oom) {
  test_json("[\"a\",1,true,false,null]");
}

TEST(JSON, simple_mixed_oom) {
  test_json("{\"a\":1,\"b\":2.3,\"3\":\"test\"}");
}

}
