<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

<<__EntryPoint>>
function entrypoint_initialvaluebaddvarray(): void {
  if (__hhvm_intrinsics\launder_value(true)) {
    include 'redefine1.inc';
  } else {
    include 'redefine2.inc';
  }

  include 'initial-value-bad-dvarray.inc';

  B::test();
  B::test();
  echo "DONE\n";
}
