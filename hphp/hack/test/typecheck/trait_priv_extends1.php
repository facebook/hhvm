<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

trait T {
  private function f(): int {
    return 10;
  }
}

class C {
  use T;

  public function g(): void {
    $this->f();
  }

  private function f(): string { // should conflict
    return 'foo';
  }

}
