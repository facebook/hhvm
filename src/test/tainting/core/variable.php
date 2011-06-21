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
 * Taint tests for functions defined in variable.idl.php
 *
 * - Please keep these tests in the same order as the function declarations in
 *   the idl files.
 * - Please make sure to test both cases, where a tainted result is expected
 *   as well as where an untainted result is expected.
 */

echo "Testing strval:\n";
assert_html_safe(strval($good1));
assert_html_unsafe(strval($bad1));

echo "\n";
echo "Testing var_export:\n";
$a = var_export($good1, true);
assert_html_safe($a);

ob_start();
var_export($good1);
$a = ob_get_clean();
assert_html_safe($a);

$a = var_export($bad1, true);
assert_html_unsafe($a);

ob_start();
var_export($bad1);
$a = ob_get_clean();
assert_html_unsafe($a);

echo "\n";
echo "Testing var_dump:\n";
ob_start();
var_dump($good1);
$a = ob_get_clean();
assert_html_safe($a);

ob_start();
var_dump($bad1);
$a = ob_get_clean();
assert_html_unsafe($a);

echo "\n";
echo "Testing debug_zval_dump:\n";
ob_start();
debug_zval_dump($good1);
$a = ob_get_clean();
assert_html_safe($a);

ob_start();
debug_zval_dump($bad1);
$a = ob_get_clean();
assert_html_unsafe($a);

echo "\n";
echo "Testing serialize:\n";
assert_html_safe(serialize($good1));
assert_html_unsafe(serialize($bad1));

echo "\n";
echo "Testing unserialize:\n";
assert_html_safe(unserialize($serialized_good));
assert_html_unsafe(unserialize($serialized_bad));

echo "\n";
echo "Testing get_defined_vars:\n";
$arr = get_defined_vars();
assert_html_safe($arr['good1']);
assert_html_unsafe($arr['bad1']);

// Note: import_request_variables is not supported in hphp

echo "\n";
echo "Testing extract:\n";
$arr = array(
  'good1' => $good1,
  'bad1' => $bad1,
);
extract($arr, EXTR_PREFIX_ALL, 'extract');
assert_html_safe($extract_good1);
assert_html_unsafe($extract_bad1);
