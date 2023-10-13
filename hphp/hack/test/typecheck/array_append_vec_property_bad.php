<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

class C {
  public vec<int> $vs = vec[];
}

function test(C $c): void {
  $c->vs[] = 'foo';
}
