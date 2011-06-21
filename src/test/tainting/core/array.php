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
 * Sanity checks for tainting array entries.
 */

$arrg1 = array($good1);
$arrb1 = array($bad1);
echo "Testing array() on strings:\n";
assert_not_tainted($arrg1[0]);
assert_tainted($arrb1[0]);

$arr = array(
  'good1' => $good1,
  'bad1'  => $bad1,
  42      => array(
    'good2' => $good2,
    'bad2'  => $bad2,
  ),
);
echo "\n";
echo "Testing array containing mixed taints:\n";
assert_not_tainted($arr);
assert_tainted(print_r($arr, true));

echo "\n";
echo "Testing taint independence among array entries:\n";
assert_not_tainted($arr['good1']);
assert_tainted($arr['bad1']);
assert_not_tainted($arr[42]);
assert_tainted(print_r($arr[42], true));
assert_not_tainted($arr[42]['good2']);
assert_tainted($arr[42]['bad2']);
