<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

class C {
  public vec<string> $vs = vec[];
}

function test(C $c): void {
  $c->vs[0] = 'foo';
}
