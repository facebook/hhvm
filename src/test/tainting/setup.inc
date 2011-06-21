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

define('TAINT_HTML',    0x1);
define('TAINT_SQL',     0x2);
define('TAINT_MUTATED', 0x4);
define('TAINT_ALL',     0x7);

$good1 = "heLlO\nwoRld\ntoto\ntiti\ntata";
$good2 = "world";
$good3 = "toto";
$serialized_good = 's:5:"hello";';

$bad1 = "eViL\nsTring\nare\tfun\narent\tthey?";
$bad2 = "some\nthing\ntoto\nbad!";
$serialized_bad = 's:11:"evil string";';

fb_set_taint($bad1, TAINT_ALL);
fb_set_taint($bad2, TAINT_ALL);
fb_set_taint($serialized_bad, TAINT_ALL);

function assert_positive($v, $taint) {
  if (ob_get_level()) {
    die("Error: may not call assert_negative() inside ob_start()\n");
  }

  if (!fb_get_taint($v, $taint)) {
    echo chr(27)."[0;32m";
    echo "OK\n";
    echo chr(27)."[0m";
  } else {
    echo chr(27)."[0;31m";
    echo "FAIL\n";
    echo chr(27)."[0m";
  }
}

function assert_negative($v, $taint) {
  if (ob_get_level()) {
    die("Error: may not call assert_positive() inside ob_start()\n");
  }

  if (fb_get_taint($v, $taint)) {
    echo chr(27)."[0;32m";
    echo "OK\n";
    echo chr(27)."[0m";
  } else {
    echo chr(27)."[0;31m";
    echo "FAIL\n";
    echo chr(27)."[0m";
  }
}

function assert_not_tainted($v) {
  return assert_positive($v, TAINT_ALL);
}
function assert_html_safe($v) {
  return assert_positive($v, TAINT_HTML);
}
function assert_sql_safe($v) {
  return assert_positive($v, TAINT_SQL);
}
function assert_static($v) {
  return assert_positive($v, TAINT_MUTATED);
}

function assert_tainted($v) {
  return assert_negative($v, TAINT_ALL);
}
function assert_html_unsafe($v) {
  return assert_negative($v, TAINT_HTML);
}
function assert_sql_unsafe($v) {
  return assert_negative($v, TAINT_SQL);
}
function assert_not_static($v) {
  return assert_negative($v, TAINT_MUTATED);
}
