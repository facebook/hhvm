<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

class B {
  public function foo(): int {
    return 2;
  }
}
class C {
  public function __construct(
    private B $b = new B(),
    private int $x = $this->b->foo(),
  ) {}
}
