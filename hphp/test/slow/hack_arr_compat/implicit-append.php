<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

function test($x, $k) {
  $x = __hhvm_intrinsics\launder_value($x);
  $x[__hhvm_intrinsics\launder_value($k)] = 123;
  return $x;
}

function test_literal() {
  $x = varray[];
  $x[0] = 123;
  $x[1] = 456;
  $x[2] = 789;
  var_dump(is_varray($x));
}

function test_promotion() {
  $x = varray[];
  $x['foo'] = 'bar';
}


<<__EntryPoint>>
function main_implicit_append() {
  $x = test(varray[], 0) |> test($$, 1) |> test($$, 2);
  var_dump(is_varray($x));
  test_literal();
  test_promotion();
}
