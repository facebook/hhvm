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
 * Check that various forms of concatenations output the right general taint
 * information.  Concatenation tests for staticity can be found in ./static/
 */

$a = $good1 . $good2;
assert_not_tainted($a);

$a = $good1 . $bad1;
assert_tainted($a);

$a = $good1;
$a .= $good2;
assert_not_tainted($a);

$a = $good1;
$a .= $bad1;
assert_tainted($a);

$a = "$good1 $good2";
assert_not_tainted($a);

$a = "$good1 $bad1";
assert_tainted($a);
