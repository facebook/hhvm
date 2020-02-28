<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

function test($x) {
  $copy = $x;
  $x['abc'] = 123;
  $copy[5] = 'abc';
}

function literals() {
  $x = varray[];
  $x[1] = 123;

  $x = varray[1, 2, 3, 4, 5];
  $x[8] = 'abc';

  $x = varray[1, 2, 3, 4, 5];
  $x['abc'] = 10;
}


function test_implicit_append() {
  $x = varray[];
  $x[0] = 1;
}

function test_sort() {
  $x = varray[1, 2, 3];
  asort(inout $x);
}

function test_unset() {
  $x = varray[1, 2];
  unset($x[1]);
  $x = varray[1, 2];
  unset($x[0]);
  $x = varray[1, 2];
  unset($x[2]);
}

function test_serialization($x) {
  $str = fb_serialize($x);
  $success = false;
  $_ = fb_unserialize($str, inout $success);
  invariant($success, 'unable to round-trip %s', var_export($x, true));
}

function test_misc() {
  $x = varray[1, 2, 3, 4];
  unset($x[0]);
  $x = varray[1, 2, 3, 4];
  $x['foo'] = 1;
  $x = varray[1, 2, 3, 4];
  $x[42] = 1;
}

<<__EntryPoint>>
function main_varr_to_darr() {
  test(varray[]);
  test(darray[]);
  test(__hhvm_intrinsics\dummy_array_builtin(varray[]));
  test(dict[]);

  test(varray[1, 2, 3, 4]);
  test(darray['a' => 10, 'b' => 20]);
  test(__hhvm_intrinsics\dummy_cast_to_kindofarray(dict['a' => 10, 'b' => 20]));
  test(dict['a' => 10, 'b' => 20]);

  test(varray[1, 2, 3, 4, 5, 6, 7, 8]);

  literals();

  test_implicit_append();

  test_sort();

  test_unset();

  test_serialization(varray[]);
  test_serialization(darray[]);
  test_serialization(varray[1, 2, 3]);
  test_serialization(darray[1 => 2, 3 => 4]);

  test_misc();

}
