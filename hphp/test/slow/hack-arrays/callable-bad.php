<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

class A {
  public function meth($x) :mixed{ var_dump(__METHOD__); return $x; }
  public static function staticMeth($x) :mixed{ var_dump(__METHOD__); return $x; }
}

function call1($c, $x) :mixed{ return $c($x); }
function call2($c, $x) :mixed{ return call_user_func($c, $x); }
function call3($c, $x) :mixed{ return array_map($c, vec[$x]); }

<<__EntryPoint>>
function main_callable_bad() :mixed{
  $cases = vec[
    42,
    'nonexistent',
    'A::func1',
    vec['B', 'nonexistent'],
    vec[new A],
    vec[new A, 'func2'],
    vec['staticMeth', 'A'],
    dict[0 => 'staticMeth', 1 => A::class],
    dict[1 => 'A', 2 => 'staticMeth'],
    dict[1 => new A, 2 => 'meth'],
    dict[0 => 'A'],
    dict[2 => 'meth', 0 => new A],
    dict[0 => A::class, 1 => 'func3'],
    dict[1 => 'meth', 0 => new A],
  ];
  $tests = vec[];
  foreach (vec['call1', 'call2', 'call3'] as $t) {
    foreach ($cases as $k => $c) {
      $tests[] = tuple($t, $k, $c);
    }
  }
  $key = 'count_callable_bad';
  $count = __hhvm_intrinsics\apc_fetch_no_check($key);
  if ($count === false) $count = 0;
  while ($count < count($tests)) {
    $test = $tests[$count];
    ++$count;
    apc_store($key, $count);
    printf("===== %s/%02d =====\n", $test[0], $test[1]);
    try {
      var_dump($test[0]($test[2], 42));
    } catch (BadMethodCallException $ex) {
      echo "Caught: ".$ex->getMessage()."\n";
      return;
    }
  }
}
