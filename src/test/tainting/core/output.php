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
 * Taint tests for functions defined in output.idl.php
 */

echo "Testing ob_start:\n";
ob_start();
ob_start();
echo $good1;
$a = ob_get_clean();
echo $a;
$b = ob_get_clean();
assert_html_safe($a);
assert_html_safe($b);

ob_start();
ob_start();
echo $bad1;
$a = ob_get_clean();
echo $a;
$b = ob_get_clean();
assert_html_unsafe($a);
assert_html_unsafe($b);

echo "\n";
echo "Testing ob_clean:\n";
ob_start();
echo $good1;
ob_clean();
echo $good2;
assert_html_safe(ob_get_clean());

ob_start();
echo $bad1;
ob_clean();
echo $good1;
assert_html_safe(ob_get_clean());

ob_start();
echo $good1;
ob_clean();
echo $bad1;
assert_html_unsafe(ob_get_clean());

ob_start();
echo $bad1;
ob_clean();
echo $bad2;
assert_html_unsafe(ob_get_clean());

echo "\n";
echo "Testing ob_flush:\n";
ob_start();
ob_start();
echo $good1;
ob_flush();
ob_end_clean();
$a = ob_get_clean();
assert_html_safe($a);

ob_start();
ob_start();
echo $bad1;
ob_flush();
ob_end_clean();
$a = ob_get_clean();
assert_html_unsafe($a);

echo "\n";
echo "Testing ob_end_clean:\n";
ob_start();
ob_start();
echo $good1;
ob_end_clean();
echo $good2;
assert_html_safe(ob_get_clean());

ob_start();
ob_start();
echo $bad1;
ob_end_clean();
echo $good1;
assert_html_safe(ob_get_clean());

ob_start();
ob_start();
echo $good1;
ob_end_clean();
echo $bad1;
assert_html_unsafe(ob_get_clean());

ob_start();
ob_start();
echo $bad1;
ob_end_clean();
echo $bad2;
assert_html_unsafe(ob_get_clean());

echo "\n";
echo "Testing ob_end_flush:\n";
ob_start();
ob_start();
echo $good1;
ob_end_flush();
assert_html_safe(ob_get_clean());

ob_start();
ob_start();
echo $bad1;
ob_end_flush();
assert_html_unsafe(ob_get_clean());

echo "\n";
echo "Testing ob_get_clean:\n";
ob_start();
echo $good1;
assert_html_safe(ob_get_clean());

ob_start();
echo $bad1;
assert_html_unsafe(ob_get_clean());

echo "\n";
echo "Testing ob_get_contents:\n";
ob_start();
echo $good1;
$a = ob_get_contents();
ob_end_clean();
assert_html_safe($a);

ob_start();
echo $bad1;
$a = ob_get_contents();
ob_end_clean();
assert_html_unsafe($a);

echo "\n";
echo "Testing ob_get_flush:\n";
ob_start();
ob_start();
echo $good1;
$a = ob_get_flush();
ob_end_clean();
$b = ob_get_clean();
assert_html_safe($a);
assert_html_safe($b);

ob_start();
ob_start();
echo $bad1;
$a = ob_get_flush();
ob_end_clean();
$b = ob_get_clean();
assert_html_unsafe($a);
assert_html_unsafe($b);


