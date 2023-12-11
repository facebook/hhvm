<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

class A {
  <<__DynamicallyCallable>> public function func1($x) :mixed{ return $x; }
  <<__DynamicallyCallable>> public static function func2($x) :mixed{ return $x; }
}

function call1($c, $x) :mixed{ return $c($x); }
function call2($c, $x) :mixed{ return call_user_func($c, $x); }
function call3($c, $x) :mixed{ return array_map($c, $x); }

function make_tests($tests, $c1, $c2) :mixed{
  if ($c1) $tests[] = vec[call1<>, $c1, count($tests)+1];
  if ($c2) $tests[] = vec[call1<>, $c2, count($tests)+1];
  if ($c1) $tests[] = vec[call2<>, $c1, count($tests)+1];
  if ($c2) $tests[] = vec[call2<>, $c2, count($tests)+1];
  if ($c1) $tests[] = vec[call3<>, $c1, vec[count($tests)+1]];
  if ($c2) $tests[] = vec[call3<>, $c2, vec[count($tests)+1]];
  return $tests;
}

<<__EntryPoint>>
function main_callable() :mixed{
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

  $count = __hhvm_intrinsics\apc_fetch_no_check('count');
  if ($count === false) $count = 0;
  while ($count < count($tests)) {
    $test = $tests[$count];
    ++$count;
    apc_store('count', $count);
    echo "====================================================\n";
    var_dump($test[0]($test[1], $test[2]));
  }
}
