<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

function test(array $a) { return varray($a); }

$a = ['a' => 1, 'b' => 2, 'c' => 3];
var_dump(test(__hhvm_intrinsics\launder_value($a)));
