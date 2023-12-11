<?hh

class A { public static function meth() :mixed{} }

function LV($x) :mixed{ return __hhvm_intrinsics\launder_value($x); }

function wrap($fun) :mixed{
  try {
    $fun();
  } catch (Exception $e) {
    echo "Caught: {$e->getMessage()}\n";
  }
}

function getTestcase(int $num) :mixed{
  $test_cases = vec[
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
  $x = LV(A::meth<>);

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
