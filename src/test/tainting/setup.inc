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

define('TAINT_HTML_MASK', 0x1);

$good1 = "heLlO\nwoRld\ntoto\ntiti\ntata";
$good2 = "world";
$good3 = "toto";

$serialized_good = 's:5:"hello";';

$bad1 = "eViL\nsTring\nare\tfun\narent\tthey?";
$bad2 = "some\nthing\ntoto\nbad!";

$serialized_bad = 's:11:"evil string";';
fb_set_taint($bad1, TAINT_HTML_MASK);
fb_set_taint($bad2, TAINT_HTML_MASK);
fb_set_taint($serialized_bad, TAINT_HTML_MASK);

function not_tainted($v) {
  if (ob_get_level()) {
    die("HEY YOU CAN'T CALL not_tainted INSIDE ob_start()");
  }

  $r = fb_get_taint($v);
  if ($r) {
    echo chr(27)."[0;31m";
    echo "FAIL (tainted something which shouldn't be tainted)\n";
    echo chr(27)."[0m";
  } else {
    echo chr(27)."[0;32m";
    echo "Ok\n";
    echo chr(27)."[0m";
  }
}

function tainted($v) {
  if (ob_get_level()) {
    die("HEY YOU CAN'T CALL tainted INSIDE ob_start()");
  }

  $r = fb_get_taint($v);
  if (!$r) {
    echo chr(27)."[0;31m";
    echo "FAIL (did not taint something which should have been)\n";
    echo chr(27)."[0m";
  } else {
    echo chr(27)."[0;32m";
    echo "Ok\n";
    echo chr(27)."[0m";
  }
}
