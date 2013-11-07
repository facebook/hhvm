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

#include "hphp/test/ext/test_ext_apc.h"
#include "hphp/runtime/ext/ext_apc.h"

///////////////////////////////////////////////////////////////////////////////

bool TestExtApc::RunTests(const std::string &which) {
  bool ret = true;
  RUN_TEST(test_apc_reserialize);
  return ret;
}

///////////////////////////////////////////////////////////////////////////////

bool TestExtApc::test_apc_reserialize() {
  {
    String str = "V:6:\"Vector\":3:{i:1;i:2;i:3;}";
    String res = apc_reserialize(str);
    VS(res, str);
  }
  {
    String str = "K:3:\"Map\":2:{s:1:\"a\";i:1;s:1:\"b\";i:2;}";
    String res = apc_reserialize(str);
    VS(res, str);
  }
  return Count(true);
}

