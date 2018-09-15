<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

class C1 {
  public function foo(): int {
    return 5;
  }
}

class C2 {
  public function foo(): string {
    return 's';
  }
}

function test(C1 $c1, C2 $c2, bool $cond): arraykey {
  $x = $cond ? $c1 : $c2;
  return $x->foo();
}
