<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

function test_cast_to_array($obj_factory) {
  var_dump(varray($obj_factory()));
  var_dump(darray($obj_factory()));
  var_dump(vec($obj_factory()));
  var_dump(dict($obj_factory()));
  var_dump(keyset($obj_factory()));
}

function empty_generator() {
  yield break;
}

function non_empty_generator() {
  yield 1;
  yield 2;
  yield 3;
}

class NonIterableObj {
};

<<__EntryPoint>>
function main(): void {
  test_cast_to_array(() ==> Vector{});
  test_cast_to_array(() ==> Vector{1, 2, 3});
  test_cast_to_array(() ==> ImmVector{});
  test_cast_to_array(() ==> ImmVector{1, 2, 3});
  test_cast_to_array(() ==> Set{});
  test_cast_to_array(() ==> Set{1, 2, 3});
  test_cast_to_array(() ==> ImmSet{});
  test_cast_to_array(() ==> ImmSet{1, 2, 3});
  test_cast_to_array(() ==> Map{});
  test_cast_to_array(() ==> Map{1 => 1, 2 => 2, 3 => 3});
  test_cast_to_array(() ==> ImmMap{});
  test_cast_to_array(() ==> ImmMap{1 => 1, 2 => 2, 3 => 3});
  test_cast_to_array(() ==> Pair{1, 2});
  test_cast_to_array(() ==> empty_generator());
  test_cast_to_array(() ==> non_empty_generator());
  try {
    test_cast_to_array(() ==> new NonIterableObj());
    echo "fail";
  } catch (Exception $e) {
  }
}
