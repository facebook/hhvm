<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

class C {
  public vec<int> $vs = vec[];
}

function test(dict<string, C> $d): void {
  $d['foo']->vs[] = 'bar';
}
