<?hh
// Copyright (c) Facebook, Inc. and its affiliates. All Rights Reserved.

function testit(mixed $m):void {
  invariant($m is Vector<_>, 'Invalid Type');
  foreach ($m as $y) {
    $x = $y['a'];
  }
}
