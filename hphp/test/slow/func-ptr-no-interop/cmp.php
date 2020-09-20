<?hh

function foo(): int {
  return 1;
}

function getStr() {
  return __hhvm_intrinsics\launder_value("foo");
}

function getFunc() {
  return __hhvm_intrinsics\launder_value(HH\fun("foo"));
}

function wrap($fun) {
  try {
    $fun();
  } catch (Exception $e) {
    print $e->getMessage()."\n";
  }
}

function comp($x, $y) {
  wrap(() ==> var_dump($x === $y));
  wrap(() ==> var_dump($x == $y));
  wrap(() ==> var_dump($x < $y));
  wrap(() ==> var_dump($x <= $y));
  wrap(() ==> var_dump($x > $y));
  wrap(() ==> var_dump($x >= $y));
  print("\n");
}

function getTestcase(int $num) {
  $test_cases = varray[
    true, false, 0, 1, 0.0, 1.0, "foo",
    __hhvm_intrinsics\dummy_cast_to_kindofarray(vec['foo']),
    varray['foo'],
    vec['foo'],
    darray[0 => 'foo'],
    dict[0 => 'foo'],
    keyset['foo'],
  ];
  return __hhvm_intrinsics\launder_value($test_cases[$num]);
}

function comp_test($x) {
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
