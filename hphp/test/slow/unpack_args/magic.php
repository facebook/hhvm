<?hh
// Copyright 2004-present Facebook. All Rights Reserved.


class A {
  public function __call($name, $arguments) {
    echo "================== A::__call =====================\n";
    var_dump($arguments);
  }

  public static function __callStatic($name, $arguments) {
    echo "================== A::__callStatic ===============\n";
    var_dump($arguments);
  }
}

function test($x) {
  $a = new A();
  $a->foobaz(1, 2, ...$x);
  $a->foobaz(...$x);
  A::foobaz(1, 2, ...$x);
  A::foobaz(...$x);
  call_user_func_array([$a, 'foobaz'], $x);
  call_user_func_array('A::foobaz', $x);
}
test([3, 4, 5]);
test(vec[3, 4, 5]);
test([10 => 3, 20 => 4, 30 => 5]);
test(dict[10 => 3, 20 => 4, 30 => 5]);
