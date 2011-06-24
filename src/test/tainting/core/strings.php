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
 * Taint tests for functions defined in string.idl.php
 *
 * Please make sure to test both cases, where a tainted result is expected
 * as well as where an untainted result is expected.
 */

$common_suite_funcs = array(
  "stripcslashes",
  "addslashes",
  "stripslashes",
  "bin2hex",
//  "hex2bin",
  "nl2br",
  "quotemeta",
  "str_shuffle",
  "strrev",
  "strtolower",
  "strtoupper",
  "ucfirst",
  "ucwords",
  "strip_tags",
  "trim",
  "ltrim",
  "chop",
  "html_entity_decode",
  "htmlentities",
  "htmlspecialchars_decode",
  "htmlspecialchars",
  "quoted_printable_encode",
  "quoted_printable_decode",
  "convert_uudecode",
  "convert_uuencode",
  "str_rot13",
  "crypt",
  "md5",
  "sha1"
);

function test_common_suite($func) {
  global $good1, $bad1;
  echo "\n";
  echo "Testing $func:\n";
  assert_html_safe($func($good1));
  assert_html_unsafe($func($bad1));
  assert_not_static($func($good1));
  assert_not_static($func($bad1));
}



echo "Testing addcslashes:\n";
assert_html_safe(addcslashes($good1, $good2));
assert_html_unsafe(addcslashes($bad1, $bad1));
assert_html_unsafe(addcslashes($good1, $bad1));
assert_not_static(addcslashes($good1, $good2));
assert_not_static(addcslashes($bad1, $bad2));

echo "\n";
echo "Testing rtrim:\n";
assert_html_safe(rtrim($good1));
assert_html_unsafe(rtrim($bad1));
assert_static(rtrim($good1));
assert_not_static(rtrim($good1, 5));
assert_not_static(rtrim($bad1));

foreach ($common_suite_funcs as $func) {
  test_common_suite($func);
}

echo "\n";
echo "Testing explode:\n";
$arr = explode("\n", $good1);
assert_html_safe($arr[0]);
assert_html_safe($arr[1]);
assert_not_static($arr[0]);
assert_not_static($arr[1]);
$arr = explode("\n", $bad1);
assert_html_unsafe($arr[0]);
assert_html_unsafe($arr[1]);
assert_not_static($arr[0]);
assert_not_static($arr[1]);

echo "\n";
echo "Testing implode:\n";
$arr = array();
$arr[] = $good1;
$arr[] = $good2;
assert_html_safe(implode("\t", $arr));
assert_static(implode("\t", $arr));
assert_not_static(implode($bad2, $arr));
$arr[] = $bad1;
assert_html_unsafe(implode("\t", $arr));
assert_not_static(implode("\t", $arr));

echo "\n";
echo "Testing join:\n";
$arr = array();
$arr[] = $good1;
$arr[] = $good2;
assert_html_safe(join("\t", $arr));
assert_not_static(join("\t", $arr));
$arr[] = $bad1;
assert_html_unsafe(join("\t", $arr));
assert_not_static(join("\t", $arr));

echo "\n";
echo "Testing str_split:\n";
$arr = str_split($good1, 2);
assert_html_safe($arr[0]);
assert_html_safe($arr[1]);
assert_not_static($arr[0]);
assert_not_static($arr[1]);
$arr = str_split($bad1, 2);
assert_html_unsafe($arr[0]);
assert_html_unsafe($arr[1]);
assert_not_static($arr[0]);
assert_not_static($arr[1]);

echo "\n";
echo "Testing chunk_split:\n";
assert_html_safe(chunk_split($good1, 3, $good2));
assert_html_unsafe(chunk_split($bad1, 3, $good1));
assert_html_unsafe(chunk_split($good1, 3, $bad1));
assert_not_static(chunk_split($good1, 3, $good2));
assert_not_static(chunk_split($bad1, 3, $good1));
assert_not_static(chunk_split($good1, 3, $bad1));

echo "\n";
echo "Testing strtok:\n";
assert_html_safe(strtok($good1, "\n"));
assert_html_safe(strtok(":\n"));
assert_not_static(strtok($good1, "\n"));
assert_not_static(strtok(":\n"));

assert_html_unsafe(strtok($bad1, "\n"));
assert_html_unsafe(strtok(":\n"));
assert_not_static(strtok($bad1, "\n"));
assert_not_static(strtok(":\n"));

echo "\n";
echo "Testing str_replace:\n";
assert_html_safe(str_replace($good3, $good2, $good1));
$arr = array($good1, $good2);
$a = str_replace($good3, $good2, $arr);
assert_html_safe($a[0]);
assert_html_safe($a[1]);

assert_html_unsafe(str_replace($good3, $bad1, $good1));
$arr = array($bad1, $bad2);
$a = str_replace($good3, $good2, $arr);
assert_html_unsafe($a[0]);
assert_html_unsafe($a[1]);

echo "\n";
echo "Testing str_ireplace:\n";
assert_html_safe(str_ireplace($good3, $good2, $good1));
assert_not_static(str_ireplace($good3, $good2, $good1));
$arr = array($good1, $good2);
$a = str_ireplace($good3, $good2, $arr);
assert_html_safe($a[0]);
assert_html_safe($a[1]);
assert_not_static($a[0]);
// Note: if no replacement occurs, static strings in array remain static.
assert_static($a[1]);

assert_html_unsafe(str_ireplace($good3, $bad1, $good1));
assert_not_static(str_ireplace($good3, $bad1, $good1));
$arr = array($bad1, $bad2);
$a = str_ireplace($good3, $good2, $arr);
assert_html_unsafe($a[0]);
assert_html_unsafe($a[1]);
assert_not_static($a[0]);
assert_not_static($a[1]);

echo "\n";
echo "Testing substr_replace:\n";
assert_html_safe(substr_replace($good1, $good2, 0, 3));
assert_html_safe(substr_replace($good1, $good2, 0));
assert_html_unsafe(substr_replace($bad1, $good1, 0, 3));
assert_html_unsafe(substr_replace($bad1, $good1, 0));
assert_html_unsafe(substr_replace($good1, $bad1, 0, 3));
assert_html_unsafe(substr_replace($good1, $bad1, 0));
assert_html_unsafe(substr_replace($bad1, $bad2, 0, 3));
assert_html_unsafe(substr_replace($bad1, $bad2, 0));

assert_not_static(substr_replace($good1, $good2, 0, 3));
assert_not_static(substr_replace($good1, $good2, 0));
assert_not_static(substr_replace($bad1, $good1, 0, 3));
assert_not_static(substr_replace($bad1, $good1, 0));
assert_not_static(substr_replace($good1, $bad1, 0, 3));
assert_not_static(substr_replace($good1, $bad1, 0));
assert_not_static(substr_replace($bad1, $bad2, 0, 3));
assert_not_static(substr_replace($bad1, $bad2, 0));

echo "\n";
echo "Testing substr:\n";
assert_html_safe(substr($good1, 5, 5));
assert_html_unsafe(substr($bad1, 5, 5));
assert_not_static(substr($good1, 5, 5));
assert_not_static(substr($bad1, 5, 5));

echo "\n";
echo "Testing str_pad:\n";
assert_html_safe(str_pad($good1, 5, $good2));
assert_html_unsafe(str_pad($bad1, 5, $good1));
assert_html_unsafe(str_pad($good1, 5, $bad1));
assert_html_unsafe(str_pad($bad1, 5, $bad2));
assert_not_static(str_pad($good1, 5, $good2));
assert_not_static(str_pad($bad1, 5, $good1));
assert_not_static(str_pad($good1, 5, $bad1));
assert_not_static(str_pad($bad1, 5, $bad2));

echo "\n";
echo "Testing str_repeat:\n";
assert_html_safe(str_repeat($good1, 5));
assert_html_unsafe(str_repeat($bad1, 13));
assert_static(str_repeat($good1, 5));
assert_not_static(str_repeat($bad1, 13));

echo "\n";
echo "Testing wordwrap:\n";
assert_html_safe(wordwrap($good1, 5, $good2, true));
assert_html_unsafe(wordwrap($bad1, 5, $good1, true));
assert_html_unsafe(wordwrap($good1, 5, $bad1, true));
assert_html_unsafe(wordwrap($bad1, 5, $bad2, true));
assert_not_static(wordwrap($good1, 5, $good2, true));
assert_not_static(wordwrap($bad1, 5, $good1, true));
assert_not_static(wordwrap($good1, 5, $bad1, true));
assert_not_static(wordwrap($bad1, 5, $bad2, true));

echo "\n";
echo "Testing crc32:\n";
assert_html_safe(crc32($good1));

echo "\n";
echo "Testing print_r:\n";
$arr = array($good1, $good2);
$x = print_r($arr, true);
assert_html_safe($x);
assert_not_static($x);
$arr = array($good1, $bad1);
$x = print_r($arr, true);
assert_html_unsafe($x);
assert_not_static($x);
