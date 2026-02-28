<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

function test($a) :mixed{
  var_dump(shuffle(inout $a));
  var_dump($a);
}


<<__EntryPoint>>
function main_shuffle() :mixed{
srand(1234);

test(vec[]);
test(vec[1, 2, 3, 4, 5]);
test(dict[1 => 'a', 2 => 'b', 3 => 'c']);

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
test(fopen(__FILE__, 'r'));
test(new stdClass);
test(Vector{1, 2, 3, 4});
}
