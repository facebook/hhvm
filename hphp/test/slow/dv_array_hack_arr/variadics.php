<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

class A {
  public function __call($name, $arguments) {
    echo "================== A::__call =====================\n";
    var_dump($arguments);
    var_dump(is_varray($arguments));
  }

}

function variadic($a, ...$b) {
  echo "================== variadic ======================\n";
  var_dump($b);
  var_dump(is_varray($b));
}

function test($x) {
  variadic(1, 2, 3, 4);
  variadic(1, 2, ...$x);
  if (count($x) > 0) variadic(...$x);
  if (count($x) > 0) call_user_func_array(fun('variadic'), $x);

  $a = new A();
  if (count($x) > 0) $a->foobaz(...$x);
  $a->foobaz(1, 2, ...$x);
  if (count($x) > 0) call_user_func_array(__hhvm_intrinsics\dummy_cast_to_kindofarray(vec[$a, 'foobaz']), $x);
}

<<__EntryPoint>>
function main_variadics() {
test(__hhvm_intrinsics\dummy_cast_to_kindofarray(vec[]));
test(__hhvm_intrinsics\dummy_cast_to_kindofarray(vec[3, 4, 5]));
test(vec[]);
test(vec[3, 4, 5]);
test(dict['a' => 3, 'b' => 4, 'c' => 5]);
test(varray[3, 4, 5]);
test(Vector{3, 4, 5});
}
