<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

function func1() { return array_diff([1, '2'], ['1']); }
function func2($a, $b) { return array_diff($a, $b); }

var_dump(func1());
var_dump(
  func2(
    __hhvm_intrinsics\launder_value([1, '2']),
    __hhvm_intrinsics\launder_value(['1'])
  )
);
