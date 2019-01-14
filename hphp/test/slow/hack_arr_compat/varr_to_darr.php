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

<<__EntryPoint>>
function main_varr_to_darr() {
  test(varray[]);
  test(darray[]);
  test([]);
  test(dict[]);

  test(varray[1, 2, 3, 4]);
  test(darray['a' => 10, 'b' => 20]);
  test(['a' => 10, 'b' => 20]);
  test(dict['a' => 10, 'b' => 20]);

  test(varray[1, 2, 3, 4, 5, 6, 7, 8]);

  literals();

  test_implicit_append();
}
