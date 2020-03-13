<?hh

class A { static public function func1() { return 1; } }

function getVArr() {
  return __hhvm_intrinsics\launder_value(varray[A::class, 'func1']);
}

function getClsMeth() {
  return __hhvm_intrinsics\launder_value(HH\class_meth(A::class, 'func1'));
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
    null,
    false, true,
    0, 1,
    0.0, 1.0,
    "Array",
    HH\fun("wrap"),
    __hhvm_intrinsics\dummy_cast_to_kindofarray(vec['A', 'func1']),
    Vector {'A', 'func1'},
    varray['A', 'func1'],
    vec['A', 'func1'],
    darray[0 => 'A', 1 => 'func1'],
    dict[0 => 'A', 1 => 'func1'],
    keyset['A', 'func1'],
  ];
  return __hhvm_intrinsics\launder_value($test_cases[$num]);
}

function comp_test($x) {
  for ($i = 0; $i < 16; $i++) {
    print("Test ".$i."\n");
    comp($x, getTestcase($i));
    comp(getTestcase($i), $x);
  }
}

<<__EntryPoint>>
function main(): void {
  comp_test(getVArr());
  print("--- test class_meth ---\n");
  comp_test(getClsMeth());
}
