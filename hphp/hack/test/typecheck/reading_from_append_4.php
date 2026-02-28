<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

function test(dict<string, int> $d, vec<string> $v): void {
  $d[$v[]] = 42;
}
