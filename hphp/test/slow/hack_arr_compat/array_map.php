<?hh

// Copyright 2004-present Facebook. All Rights Reserved.

<<__EntryPoint>>
function main_array_map() {
array_map($x ==> $x, __hhvm_intrinsics\dummy_cast_to_kindofarray(dict['x' => 3]));
array_filter(__hhvm_intrinsics\dummy_cast_to_kindofarray(dict['x' => 3]), $x ==> true);
echo "DONE\n";
}
