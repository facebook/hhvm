<?hh

function foo(): int {
  return 1;
}

function getStr() :mixed{
  return __hhvm_intrinsics\launder_value("foo");
}

function getFunc() :mixed{
  return __hhvm_intrinsics\launder_value(foo<>);
}

function wrap($fun) :mixed{
  try {
    $fun();
  } catch (Exception $e) {
    print $e->getMessage()."\n";
  }
}

function comp($x, $y) :mixed{
  wrap(() ==> var_dump($x === $y));
  wrap(() ==> var_dump(HH\Lib\Legacy_FIXME\eq($x, $y)));
  wrap(() ==> var_dump(HH\Lib\Legacy_FIXME\lt($x, $y)));
  wrap(() ==> var_dump(HH\Lib\Legacy_FIXME\lte($x, $y)));
  wrap(() ==> var_dump(HH\Lib\Legacy_FIXME\gt($x, $y)));
  wrap(() ==> var_dump(HH\Lib\Legacy_FIXME\gte($x, $y)));
  print("\n");
}

function getTestcase(int $num) :mixed{
  $test_cases = vec[
    true, false, 0, 1, 0.0, 1.0, "foo",
    darray(vec['foo']),
    vec['foo'],
    vec['foo'],
    dict[0 => 'foo'],
    dict[0 => 'foo'],
    keyset['foo'],
  ];
  return __hhvm_intrinsics\launder_value($test_cases[$num]);
}

function comp_test($x) :mixed{
  for ($i = 0; $i < 13; $i++) {
    print("Test ".$i."\n");
    comp($x, getTestcase($i));
    comp(getTestcase($i), $x);
  }
}

<<__EntryPoint>>
function main(): void {
  comp_test(getStr());
  print("--- test func --- \n");
  comp_test(getFunc());
}
