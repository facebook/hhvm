<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

function test(): void {
  $x = null;
  invariant($x is vec<_>, "");
  $x[] = 0;
  $x[0];
  $x[0] = 0;
}
