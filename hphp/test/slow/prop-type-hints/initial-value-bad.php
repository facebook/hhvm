<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

enum Enum1 : string {
  VAL1 = 'val1';
  VAL2 = 'val2';
  VAL3 = 'val3';
}

enum Enum2 : int {
  VAL1 = 1;
  VAL2 = 2;
  VAL3 = 3;
}

enum Enum3 : arraykey {
  VAL1 = 1;
  VAL2 = 'val2';
  VAL3 = 2;
}

<<__EntryPoint>>
function entrypoint_initialvaluebad(): void {

  if (__hhvm_intrinsics\launder_value(true)) {
    include 'redefine1.inc';
  } else {
    include 'redefine2.inc';
  }
  include 'initial-value-bad.inc';

  A::test();
  A::test();
  C::test();
  C::test();
  echo "DONE\n";
}
