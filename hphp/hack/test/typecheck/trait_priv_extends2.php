<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

trait T {
  abstract public function f(): int;
}

class C {
  use T;

  public function g(): void {
    $this->f();
  }

  private function f(): string {
    return '';
  }
}
