<?hh

function main(?array $a) {
  return foo($a[100][200]);
}

// Copyright 2004-present Facebook. All Rights Reserved.

<<__EntryPoint>>
function main_unknown_reffiness_array_param() {
if (__hhvm_intrinsics\launder_value(false)) {
  include 'unknown-reffiness-array-param1.inc';
} else {
  include 'unknown-reffiness-array-param2.inc';
}

$a = [100 => [200 => 'abc']];
var_dump(main(__hhvm_intrinsics\launder_value($a)));
}
