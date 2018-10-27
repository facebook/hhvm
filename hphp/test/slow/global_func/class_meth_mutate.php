<?hh
class C {
  static public function func1() {
    return 1;
  }
}

class D {
  static public function func1() {
    return 10;
  }

  static public function func2() {
    return 20;
  }
}

function testMutateClsMethArray() {
  $m = hh\class_meth(C::class, 'func1');
  var_dump($m is vec);
  var_dump($m());

  $m[0] = D::class;
  $m[1] = 'func1';
  var_dump($m is vec);
  var_dump($m()); // succeed
}

function testMutatePartialClsMethArray() {
  $m = hh\class_meth(C::class, 'func1');
  var_dump($m());

  $m[0] = D::class;
  var_dump($m()); // fail E_ERROR
}

<<__EntryPoint>>
function main() {
  // Right now these 2 tests will pass.
  // After migrating to new ClaMethKind, we can kill these tests.
  testMutateClsMethArray();
  testMutatePartialClsMethArray();
}
