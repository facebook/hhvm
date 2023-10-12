<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

class C {
  public int $x = 0;
}

function test(vec<C> $v): void {
  $v[]->x = 42;
}
