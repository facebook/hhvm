<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

function test($c) :mixed{
  echo "====================================================\n";
  var_dump($c->toKeysArray());
  var_dump($c->toValuesArray());
}


<<__EntryPoint>>
function main_collections_to_keys_and_values() :mixed{
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
