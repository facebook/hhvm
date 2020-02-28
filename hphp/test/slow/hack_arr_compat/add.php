<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

function do_add($a, $b) {
  var_dump($a + $b);
}

function main() {
  do_add(__hhvm_intrinsics\dummy_cast_to_kindofarray(vec[]), __hhvm_intrinsics\dummy_cast_to_kindofarray(vec[]));
  do_add(__hhvm_intrinsics\dummy_cast_to_kindofarray(vec[1, 2, 3]), __hhvm_intrinsics\dummy_cast_to_kindofarray(vec[]));
  do_add(__hhvm_intrinsics\dummy_cast_to_kindofarray(vec[]), __hhvm_intrinsics\dummy_cast_to_kindofarray(vec[1, 2, 3]));
  do_add(__hhvm_intrinsics\dummy_cast_to_kindofarray(vec[1, 2, 3]), __hhvm_intrinsics\dummy_cast_to_kindofarray(vec[4, 5, 6]));
}

<<__EntryPoint>>
function main_add() {
main();
}
