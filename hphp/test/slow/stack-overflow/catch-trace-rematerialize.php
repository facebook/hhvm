<?hh
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

function foo($a) :mixed{
  return bar($a);
}

function bar($a) :mixed{
  return baz($a, 'garbanzo');
}

function baz($a, $b) :mixed{
  return quux($b, $a);
}

function chwrap($x) :mixed{
  return choose($x);
}

function choose($a) :mixed{
  $arr = vec['banana', $a, 'apple'];
  $dict = dict[
    'banana' => 'banana',
    $a => $a,
    'apple' => 'apple',
  ];

  $out = array_map(
    function ($val) { return strrev($val); },
    $arr
  );

  foreach ($out as $idx => $thing) {
    $out[$idx] = $thing . 'boop';
  }

  return $arr[0] === 'bananaboop';
}

function quux($b, $a) :mixed{
  $arr = vec[$b, 'banana', $a, 'apple'];
  $dict = dict[
    $b => $b,
    'banana' => 'banana',
    $a => $a,
    'apple' => 'apple',
  ];

  $out = array_map(
    function ($val) {
      return chwrap($val) ? $val : strrev($val);
    },
    $arr
  );

  foreach ($out as $idx => $thing) {
    $out[$idx] = chwrap($thing) ? foo($thing) : bar($thing);
  }
  return $out;
}

function main() :mixed{
  foo('tofu');
}

<<__EntryPoint>>
function main_catch_trace_rematerialize() :mixed{
main();
}
