<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

function test_cmp() {
  var_dump(__hhvm_intrinsics\dummy_cast_to_kindofarray(vec[1, 2, 3]) === vec[1, 2, 3]);
  var_dump(__hhvm_intrinsics\dummy_cast_to_kindofarray(vec[1, 2, 3]) !== vec[1, 2, 3]);
  var_dump(__hhvm_intrinsics\dummy_cast_to_kindofarray(vec[1, 2, 3]) == vec[1, 2, 3]);
  var_dump(__hhvm_intrinsics\dummy_cast_to_kindofarray(vec[1, 2, 3]) != vec[1, 2, 3]);
  wrap(() ==> __hhvm_intrinsics\dummy_cast_to_kindofarray(vec[1, 2, 3]) < true, __LINE__);
  wrap(() ==> __hhvm_intrinsics\dummy_cast_to_kindofarray(vec[1, 2, 3]) <= true, __LINE__);
  wrap(() ==> __hhvm_intrinsics\dummy_cast_to_kindofarray(vec[1, 2, 3]) > true, __LINE__);
  wrap(() ==> __hhvm_intrinsics\dummy_cast_to_kindofarray(vec[1, 2, 3]) >= true, __LINE__);
  wrap(() ==> __hhvm_intrinsics\dummy_cast_to_kindofarray(vec[1, 2, 3]) <=> true, __LINE__);

  var_dump(vec[1, 2, 3] === __hhvm_intrinsics\dummy_cast_to_kindofarray(vec[1, 2, 3]));
  var_dump(vec[1, 2, 3] !== __hhvm_intrinsics\dummy_cast_to_kindofarray(vec[1, 2, 3]));
  var_dump(vec[1, 2, 3] == __hhvm_intrinsics\dummy_cast_to_kindofarray(vec[1, 2, 3]));
  var_dump(vec[1, 2, 3] != __hhvm_intrinsics\dummy_cast_to_kindofarray(vec[1, 2, 3]));
  wrap(() ==> true < __hhvm_intrinsics\dummy_cast_to_kindofarray(vec[1, 2, 3]), __LINE__);
  wrap(() ==> true <= __hhvm_intrinsics\dummy_cast_to_kindofarray(vec[1, 2, 3]), __LINE__);
  wrap(() ==> true > __hhvm_intrinsics\dummy_cast_to_kindofarray(vec[1, 2, 3]), __LINE__);
  wrap(() ==> true >= __hhvm_intrinsics\dummy_cast_to_kindofarray(vec[1, 2, 3]), __LINE__);
  wrap(() ==> true <=> __hhvm_intrinsics\dummy_cast_to_kindofarray(vec[1, 2, 3]), __LINE__);
}

function test_add() {
  var_dump(__hhvm_intrinsics\dummy_cast_to_kindofarray(vec[1, 2, 3]) + __hhvm_intrinsics\dummy_cast_to_kindofarray(vec[1, 2, 3, 4, 5]));
  var_dump(__hhvm_intrinsics\dummy_cast_to_kindofarray(vec[1, 2, 3, 4, 5]) + __hhvm_intrinsics\dummy_cast_to_kindofarray(vec[1, 2, 3]));
}

function test_intlike_keys() {
  var_dump(__hhvm_intrinsics\dummy_cast_to_kindofarray(dict['0' => 1]));
  var_dump(__hhvm_intrinsics\dummy_cast_to_kindofarray(dict['0' => 1, 1 => 2, 2 => 3, 3 => 4]));
  var_dump(__hhvm_intrinsics\dummy_cast_to_kindofarray(dict[0 => 1, 1 => 2, '2' => 3, 3 => 4]));
  var_dump(__hhvm_intrinsics\dummy_cast_to_kindofarray(dict[0 => 1, 1 => 2, 2 => 3, '3' => 4]));

  var_dump(__hhvm_intrinsics\dummy_cast_to_kindofarray(dict['10' => 10]));
  var_dump(__hhvm_intrinsics\dummy_cast_to_kindofarray(dict['10' => 10, 20 => 20, 30 => 30]));
  var_dump(__hhvm_intrinsics\dummy_cast_to_kindofarray(dict[10 => 10, '20' => 20, 30 => 30]));
  var_dump(__hhvm_intrinsics\dummy_cast_to_kindofarray(dict[10 => 10, 20 => 20, '30' => 30]));
}

function wrap($cmp, $lineno) {
  echo "\n";
  try {
    $ret = $cmp();
  } catch (InvalidOperationException $e) {
    echo "Caught: ".$e->getMessage()." on line $lineno\n";
    return;
  }
  echo "FAIL ".var_export($ret, true)."\n";
}

<<__EntryPoint>>
function main_scalars() {
  test_cmp();
  test_add();
  test_intlike_keys();
}
