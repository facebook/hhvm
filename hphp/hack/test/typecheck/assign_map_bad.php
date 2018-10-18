<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

function test(Map<string, int> $v): Map<string, num> {
  $v['foo'] = 3.14;
  return $v;
}
