<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

function test(arraylike $a) { return varray($a); }


<<__EntryPoint>>
function main_arr_to_varray_cast() {
$a = __hhvm_intrinsics\dummy_cast_to_kindofarray(dict['a' => 1, 'b' => 2, 'c' => 3]);
var_dump(test(__hhvm_intrinsics\launder_value($a)));
}
