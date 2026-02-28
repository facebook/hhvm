<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

function test($x) :mixed{
  $x = __hhvm_intrinsics\launder_value($x);
  var_dump(HH\is_list_like($x));
}

<<__EntryPoint>> function test_all(): void {
  test(null);
  test(false);
  test(true);
  test(123);
  test(3.141);
  test('abc');
  test(new stdClass);

  test(dict[]);
  test(vec[]);
  test(dict[]);
  test(vec[]);
  test(dict[]);
  test(keyset[]);

  test(darray(vec[1, 2, 3]));
  test(darray(dict[0 => 'a', 1 => 'b']));
  test(vec['a', 'b', 'c']);
  test(dict[0 => 'a', 1 => 'b']);
  test(vec['a', 'b', 'c']);
  test(dict[0 => 'a', 1 => 'b']);
  test(keyset[0, 1]);

  test(darray(dict['a' => 100, 'b' => 200]));
  test(dict['a' => 100, 'b' => 200]);
  test(dict['a' => 100, 'b' => 200]);
  test(keyset[0, 100]);
}
