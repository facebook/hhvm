<?php
/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010 Facebook, Inc. (http://www.facebook.com)          |
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
require_once('../setup.inc');

/**
 * Check that fb_is_static_string works correctly on various static values
 * cast to strings.  Currently:
 *   - all numeric values (ints, doubles) are by default non-static (see
 *     constructors in runtime/base/type_string.cpp)
 *   - bools and NULL are static (N.B.: this is done because when implicitly
 *     typecast in HPHP, bools and NULL are converted to global string
 *     constants, and tainting these constants could have undesired side-
 *     effects; we would like it if they could be marked non-static, hence
 *     the test failure)
 *   - HPHP casts all arrays to the string "Array"; this is considered
 *     static
 *   - values in an array are static or non-static (or, in general, tainted
 *     or not tainted) independent of other values
 *   - whether an object is cast to a static string depends on whether its
 *     __toString() method returns a static string
 */

echo "Testing string literal: ";
if (fb_get_taint("str", TAINT_MUTATED)) {
  echo chr(27)."[0;31m";
  echo "FAIL\n";
  echo chr(27)."[0m";
} else {
  echo chr(27)."[0;32m";
  echo "OK\n";
  echo chr(27)."[0m";
}

$str = "str";
echo "Testing static string variable: ";
assert_static($str);

echo "Testing NULL: ";
assert_not_static(NULL);

echo "Testing true: ";
assert_not_static(true);
echo "Testing false: ";
assert_not_static(false);

echo "Testing int: ";
assert_not_static(42);

echo "Testing double: ";
assert_not_static(3.1415926535897);

$array = array(
  'str' => 42,
  42 => 'str',
  1 => true,
);
echo "Testing array...\n";
echo "  int value: ";
assert_not_static($array['str']);
echo "  string value: ";
assert_static($array[42]);
echo "  bool value: ";
assert_not_static($array[1]);
echo "  typecast as string: ";
assert_static($array);

class ObjToStringLiteral {
  function __toString() { return "obj"; }
}
class ObjToStringNonliteral {
  function __toString() {
    global $bad1;
    return "obj" . $bad1;
  }
}
echo "Testing object...\n";
$obj1 = new ObjToStringLiteral();
echo "  cast as literal: ";
assert_static($obj1);
$obj2 = new ObjToStringNonliteral();
echo "  cast as nonliteral: ";
assert_not_static($obj2);
