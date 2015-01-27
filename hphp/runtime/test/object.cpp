/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2014 Facebook, Inc. (http://www.facebook.com)     |
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

#include <gtest/gtest.h>

#include "hphp/runtime/base/comparisons.h"
#include "hphp/runtime/base/type-object.h"
#include "hphp/runtime/ext/string/ext_string.h"

namespace HPHP {

TEST(Object, Serialization) {
  String s = "O:1:\"B\":1:{s:3:\"obj\";O:1:\"A\":1:{s:1:\"a\";i:10;}}";
  Variant v = unserialize_from_string(s);
  EXPECT_TRUE(v.isObject());
  auto o = v.toObject();
  EXPECT_TRUE(
    !o->getClassName().asString().compare("__PHP_Incomplete_Class")
  );
  auto os = HHVM_FN(serialize)(o);
  EXPECT_TRUE(
    !os.compare( "O:1:\"B\":1:{s:3:\"obj\";O:1:\"A\":1:{s:1:\"a\";i:10;}}")
  );
}

}
