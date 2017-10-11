<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

class A {
  public function func1($x) { return $x; }
  public static function func2($x) { return $x; }
}

function call1($c, $x) { return $c($x); }
function call2($c, $x) { return call_user_func($c, $x); }
function call3($c, $x) { return forward_static_call($c, $x); }
function call4($c, $x) { return fb_call_user_func_safe($c, $x); }
function call5($c, $x) { return array_map($c, $x); }
function call6($c, $x) { return array_walk($x, $c); }

function make_tests($tests, $c1, $c2) {
  if ($c1) $tests[] = vec['call1', $c1, count($tests)+1];
  if ($c2) $tests[] = vec['call1', $c2, count($tests)+1];
  if ($c1) $tests[] = vec['call2', $c1, count($tests)+1];
  if ($c2) $tests[] = vec['call2', $c2, count($tests)+1];
  if ($c1) $tests[] = vec['call3', $c1, count($tests)+1];
  if ($c2) $tests[] = vec['call3', $c2, count($tests)+1];
  if ($c1) $tests[] = vec['call4', $c1, count($tests)+1];
  if ($c2) $tests[] = vec['call4', $c2, count($tests)+1];
  if ($c1) $tests[] = vec['call5', $c1, [count($tests)+1]];
  if ($c2) $tests[] = vec['call5', $c2, [count($tests)+1]];
  if ($c1) $tests[] = vec['call6', $c1, [count($tests)+1]];
  if ($c2) $tests[] = vec['call6', $c2, [count($tests)+1]];
  return $tests;
}

$tests = vec[]
  |> make_tests(
      $$,
      vec[new A, 'func1'],
      vec['A', 'func2']
     )
  |> make_tests(
      $$,
      dict[0 => new A, 1 => 'func1'],
      dict[0 => 'A', 1 => 'func2']
     )
  |> make_tests($$, null, keyset['A', 'func2']);

$count = apc_fetch('count');
if ($count === false) $count = 0;
while ($count < count($tests)) {
  $test = $tests[$count];
  ++$count;
  apc_store('count', $count);
  echo "====================================================\n";
  var_dump($test[0]($test[1], $test[2]));
}
