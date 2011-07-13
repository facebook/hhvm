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
 * Taint tests for call_user_func*() family of functions.
 */

// Checks that call_user_func* family correctly propagate taint (i.e., that
// taints are passed to parameters even without a TaintObserver declared in
// the scope of call_user_func*)
function callback_check_propagation($a, $a_taint, $b, $b_taint) {
  $a_taint ? assert_tainted($a) : assert_not_tainted($a);
  $b_taint ? assert_tainted($b) : assert_not_tainted($b);
}

// Helper function for callback_check_independence to make a new string without
// scoping a new TaintObserver
// TODO figure out away to avoid re-scoping a TaintObserver (currently this is
// a placeholder)
function make_string($a) {
  return $a . 'quux';
}

// Checks that call_user_func* family does not create a TaintObserver which
// would transmit taint between independent strings
function callback_check_independence($a) {
  // taint $a
  fb_set_taint($a, TAINT_HTML);

  // access $a without scoping new TaintObserver
  if ($a === 'foobarbaz') {
    // create new string $b without scoping new TaintObserver
    $b = make_string('');
    assert_not_tainted($b);
  }
}

$arr_good  = array($good1, false, $good2, false);
$arr_mixed = array($good1, false, $bad1, true);
$arr_bad   = array($bad1, true, $bad2, true);

echo "Testing call_user_func taint propagation:\n";
call_user_func('callback_check_propagation', $good1, false, $good2, false);
call_user_func('callback_check_propagation', $good1, false, $bad1, true);
call_user_func('callback_check_propagation', $bad1, true, $bad2, true);

echo "\n";
echo "Testing call_user_func_array taint propagation:\n";
call_user_func_array('callback_check_propagation', $arr_good);
call_user_func_array('callback_check_propagation', $arr_mixed);
call_user_func_array('callback_check_propagation', $arr_bad);

echo "\n";
echo "Testing taint independence:\n";
call_user_func('callback_check_independence', 'foobarbaz');
call_user_func_array('callback_check_independence', array('foobarbaz'));
