<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

function test($x) {
  $x = __hhvm_intrinsics\launder_value($x);
  var_dump(HH\is_list_like($x));
}

function test_all() {
  test(null);
  test(false);
  test(true);
  test(123);
  test(3.141);
  test('abc');
  test(new stdclass);

  test([]);
  test(varray[]);
  test(darray[]);
  test(vec[]);
  test(dict[]);
  test(keyset[]);

  test([1, 2, 3]);
  test([0 => 'a', 1 => 'b']);
  test(varray['a', 'b', 'c']);
  test(darray[0 => 'a', 1 => 'b']);
  test(vec['a', 'b', 'c']);
  test(dict[0 => 'a', 1 => 'b']);
  test(keyset[0, 1]);

  test(['a' => 100, 'b' => 200]);
  test(darray['a' => 100, 'b' => 200]);
  test(dict['a' => 100, 'b' => 200]);
  test(keyset[0, 100]);
}
test_all();
