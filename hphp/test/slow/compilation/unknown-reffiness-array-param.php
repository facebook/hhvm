<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

if (__hhvm_intrinsics\launder_value(false)) {
  function foo(&$a) { return $a; }
} else {
  function foo($a) { return $a; }
}

function main(?array $a) {
  return foo($a[100][200]);
}

$a = [100 => [200 => 'abc']];
var_dump(main(__hhvm_intrinsics\launder_value($a)));
