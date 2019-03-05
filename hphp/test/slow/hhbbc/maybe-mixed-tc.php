<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

if (__hhvm_intrinsics\launder_value(true)) {
  require 'maybe-mixed-tc1.inc';
} else {
  require 'maybe-mixed-tc2.inc';
}

function test(Disable $c) {
  var_dump($c);
}
test(null);
