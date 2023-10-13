<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

function test(Pair<int, vec<string>> $p): void {
  $p[1][] = 'foo';
}
