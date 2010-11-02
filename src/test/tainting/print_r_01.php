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
require_once('setup.inc');

$a = "good\n";
$b = print_r($a, true);

if(fb_get_taint($b) & TAINT_HTML_MASK){
  echo "b is tainted\n";
} else {
  echo "b is not tainted\n";
}

$b = array($a);
$c = print_r($b, true);

if(fb_get_taint($c) & TAINT_HTML_MASK){
  echo "c is tainted\n";
} else {
  echo "c is not tainted\n";
}
