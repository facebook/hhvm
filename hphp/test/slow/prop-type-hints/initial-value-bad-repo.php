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
function main(): void {
  require_once __DIR__.'/redefine1.inc';
  require_once __DIR__.'/initial-value-bad-repo-classes.inc';

  A::test();
  A::test();
  C::test();
  C::test();
  echo "DONE\n";
}
