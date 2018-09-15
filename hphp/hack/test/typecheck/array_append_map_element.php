<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

function test(Map<string, vec<int>> $m): void {
  $m['foo'][] = 'bar';
}
