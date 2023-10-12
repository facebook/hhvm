<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

class C {
  public function __construct(private bool $x) {}
  public function foo(): bool {
    return true;
  }
  public function test(): void {
    $this->x = $this->x ?: $this->foo();
  }
  public function test2(): void {
    // Note space!
    $this->x = $this->x ? : $this->foo();
  }
}
