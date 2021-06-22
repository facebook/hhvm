<?hh

class A { public static function meth() {} }

function LV($x) { return __hhvm_intrinsics\launder_value($x); }

function wrap($fun) {
  try {
    $fun();
  } catch (Exception $e) {
    echo "Caught: {$e->getMessage()}\n";
  }
}

function getTestcase(int $num) {
  $test_cases = varray[
    null,
    false, true,
    0, 1,
    0.0, 1.0,
    "Array",
    wrap<>,
    Vector {'A', 'meth'},
    vec['A', 'meth'],
    dict[0 => 'A', 1 => 'meth'],
    keyset['A', 'meth'],
  ];
  return LV($test_cases[$num]);
}

<<__EntryPoint>>
function main(): void {
  $x = LV(HH\class_meth(A::class, 'meth'));

  for ($i = 0; $i < 13; $i++) {
    $y = getTestcase($i);

    print("Test ".$i."\n");

    wrap(() ==> var_dump($x === $y));
    wrap(() ==> var_dump($x == $y));
    wrap(() ==> var_dump($x < $y));
    wrap(() ==> var_dump($x <= $y));
    wrap(() ==> var_dump($x > $y));
    wrap(() ==> var_dump($x >= $y));
    print("\n");

    wrap(() ==> var_dump($y === $x));
    wrap(() ==> var_dump($y == $x));
    wrap(() ==> var_dump($y < $x));
    wrap(() ==> var_dump($y <= $x));
    wrap(() ==> var_dump($y > $x));
    wrap(() ==> var_dump($y >= $x));
    print("\n");
  }
}
