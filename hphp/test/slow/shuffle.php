<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

function test($a) {
  var_dump(shuffle($a));
  var_dump($a);
}

srand(1234);

test([]);
test([1, 2, 3, 4, 5]);
test([1 => 'a', 2 => 'b', 3 => 'c']);

test(vec[]);
test(vec[1, 2, 3, 4, 5]);
test(vec['a', 'b', 'c']);

test(dict[]);
test(dict[1 => 'a', 2 => 'b', 3 => 'c']);
test(dict['a' => 100, 'b' => 200, 'c' => 300]);

test(keyset[]);
test(keyset[1, 2, 3, 4, 5]);
test(keyset['a', 'b', 'c']);

test(null);
test(true);
test(false);
test(123);
test('abc');
test(3.14);
test(STDIN);
test(new stdclass);
test(Vector{1, 2, 3, 4});
