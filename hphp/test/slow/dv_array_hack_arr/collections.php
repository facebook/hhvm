<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

function test($c) :mixed{
  echo "====================================================\n";
  $x1 = $c->toVArray();
  $x2 = $c->toDArray();
  var_dump($x1);
  var_dump($x2);
  var_dump(is_varray($x1));
  var_dump(is_darray($x1));
  var_dump(is_varray($x2));
  var_dump(is_darray($x2));
}


<<__EntryPoint>>
function main_collections() :mixed{
test(Vector{});
test(Map{});
test(Set{});

test(ImmVector{});
test(ImmMap{});
test(ImmSet{});

test(Vector{100, 200, 300});
test(Map{'a' => 100, 'b' => 200});
test(Set{'a', 'b', 'c'});
test(Pair{'abc', 'def'});

test(ImmVector{100, 200, 300});
test(ImmMap{'a' => 100, 'b' => 200});
test(ImmSet{'a', 'b', 'c'});
}
