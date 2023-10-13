<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

function test(vec<dict<string, int>> $v): void {
  $v[]['foo'] = 42;
}
