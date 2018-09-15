<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

function test(vec<vec<int>> $v): void {
  $v[][] = 42;
}
