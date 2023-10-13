<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

function test(Pair<int, Vector<string>> $p): void {
  $p[1][] = 'foo';
}
