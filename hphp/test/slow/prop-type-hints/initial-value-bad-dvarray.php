<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

<<__EntryPoint>>
function entrypoint_initialvaluebaddvarray(): void {
  include 'redefine1.inc';

  include 'initial-value-bad-dvarray.inc';

  B::test();
  B::test();
  echo "DONE\n";
}
