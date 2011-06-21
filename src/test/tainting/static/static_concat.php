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
 * Check that basic concatenations deal with static-ness correctly.
 */

echo "Testing literals:\n";
if (fb_get_taint("heLlO\nwoRld\ntoto\ntiti\ntata", TAINT_MUTATED)) {
  echo chr(27)."[0;31m";
  echo "FAIL\n";
  echo chr(27)."[0m";
} else {
  echo chr(27)."[0;32m";
  echo "OK\n";
  echo chr(27)."[0m";
}
if (fb_get_taint("heLlO\nwoRld\ntoto\ntiti\ntata" . "world", TAINT_MUTATED)) {
  echo chr(27)."[0;31m";
  echo "FAIL\n";
  echo chr(27)."[0m";
} else {
  echo chr(27)."[0;32m";
  echo "OK\n";
  echo chr(27)."[0m";
}
if (fb_get_taint("heLlO\nwoRld\ntoto\ntiti\ntata" . "world" . "toto",
                 TAINT_MUTATED)) {
  echo chr(27)."[0;31m";
  echo "FAIL\n";
  echo chr(27)."[0m";
} else {
  echo chr(27)."[0;32m";
  echo "OK\n";
  echo chr(27)."[0m";
}

echo "\n";
echo "Testing passed literals:\n";
assert_static("heLlO\nwoRld\ntoto\ntiti\ntata");
assert_static("heLlO\nwoRld\ntoto\ntiti\ntata" . "world");
assert_static("heLlO\nwoRld\ntoto\ntiti\ntata" . "world" . "toto");

echo "\n";
echo "Testing assigned literals:\n";
assert_static($good1);
assert_static($good1 . $good2);
assert_not_static($bad1);
assert_not_static($bad1 . $good2);

echo "\n";
echo "Testing in-quote concat:\n";
assert_static("$good1");
assert_static("$good1$good2");
assert_not_static("$bad1");
assert_not_static("$bad1$good2");

echo "\n";
echo "Testing assignment:\n";
$a = $good1;
assert_static($a);
$a .= $good2;
assert_static($a);
$a .= $good2 . $good3;
assert_static($a);
$a = $bad1;
assert_not_static($a);
$a .= $good2;
assert_not_static($a);

echo "\n";
echo "Testing global constants:\n";
define('CONSTANT', 'constant');
assert_static(CONSTANT);
assert_static($good1 . CONSTANT);
assert_not_static($bad1 . CONSTANT);

function wrap_concat($a, $b) {
  return $a . $b;
}
function wrap_concat_with_local($b) {
  $a = "local";
  return $a . $b;
}
function wrap_concat_with_bad_local($b) {
  global $bad2;
  $a = $bad2;
  return $a . $b;
}
echo "\n";
echo "Testing function-wrapped concat:\n";
assert_static(wrap_concat("heLlO\nwoRld\ntoto\ntiti\ntata", "world"));
assert_static(wrap_concat($good1, $good2));
assert_static(wrap_concat_with_local($good1));
assert_not_static(wrap_concat_with_bad_local($good1));
assert_not_static(wrap_concat($good1, $bad2));
assert_not_static(wrap_concat_with_local($bad1));

class StaticConcat {
  const SC_CONST = "constant";
  public $sc_prop;

  function __construct() {
    $this->sc_prop = "property";
  }

  public function concatWithConstant($str) {
    return $str . self::SC_CONST;
  }

  public function concatWithProperty($str) {
    return $str . $this->sc_prop;
  }
}
echo "\n";
echo "Testing class-wrapped concat:\n";
assert_static($good1 . StaticConcat::SC_CONST);
$sc = new StaticConcat();
assert_static($sc->concatWithConstant($good1));
assert_static($good1 . $sc->sc_prop);
assert_static($sc->concatWithProperty($good1));

echo "\n";
echo "Testing concat with other types:\n";
assert_static($good1 . NULL);
assert_static($good1 . true);
assert_static($good1 . false);
assert_not_static($good1 . 42);
assert_not_static($good1 . 3.1415926535897);
