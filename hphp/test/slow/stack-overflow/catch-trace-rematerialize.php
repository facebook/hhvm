<?php
/*
 * This test is intended to ensure that we restore %rsp correctly as we unwind
 * through TC frames.
 *
 * The function that hits the stack overflow (either choose() or quux()) should
 * have an inlined function as a parent.  All the inlined functions are simple
 * wrappers, so they should have their frames partially elided, and the
 * DefInlineFP should be sunk into the catch trace for the call to quux().
 *
 * If we don't restore %rsp correctly before unwinding to that catch trace, it
 * will scribble over some VM stack memory, and will cause crashes (or
 * assertion failures, in a debug build) somewhere in the unwinder---either in
 * tc_unwind_resume() or discardStackTemps(), or a callee of one of those.
 */

function foo($a) {
  return bar($a);
}

function bar($a) {
  return baz($a, 'garbanzo');
}

function baz($a, $b) {
  return quux($b, $a);
}

function chwrap($x) {
  return choose($x);
}

function choose($a) {
  $arr = array('banana', $a, 'apple');
  $dict = array(
    'banana' => 'banana',
    $a => $a,
    'apple' => 'apple',
  );

  $out = array_map(
    function ($val) { return strrev($val); },
    $arr
  );

  foreach ($out as &$thing) {
    $thing = $thing . 'boop';
  }

  return $arr[0] === 'bananaboop';
}

function quux($b, $a) {
  $arr = array($b, 'banana', $a, 'apple');
  $dict = array(
    $b => $b,
    'banana' => 'banana',
    $a => $a,
    'apple' => 'apple',
  );

  $out = array_map(
    function ($val) {
      return chwrap($val) ? $val : strrev($val);
    },
    $arr
  );

  foreach ($out as &$thing) {
    $thing = chwrap($thing) ? foo($thing) : bar($thing);
  }
  return $out;
}

function main() {
  foo('tofu');
}

<<__EntryPoint>>
function main_catch_trace_rematerialize() {
main();
}
