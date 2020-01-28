<?hh
// Copyright 2004-present Facebook. All Rights Reserved.


class A {
  public function __call($name, $arguments) {
    echo "================== A::__call =====================\n";
    var_dump($arguments);
  }
}

function test($x) {
  $a = new A();
  $a->foobaz(1, 2, ...$x);
  $a->foobaz(...$x);
  call_user_func_array(varray[$a, 'foobaz'], $x);
}

<<__EntryPoint>>
function main_magic() {
test(varray[3, 4, 5]);
test(vec[3, 4, 5]);
test(darray[10 => 3, 20 => 4, 30 => 5]);
test(dict[10 => 3, 20 => 4, 30 => 5]);
}
