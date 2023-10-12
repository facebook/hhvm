<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

function test(Pair<int, Map<string, int>> $p): void {
  $p[1]['foo'] = 42;
}
