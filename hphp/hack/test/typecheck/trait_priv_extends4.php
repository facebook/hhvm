<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

class C1 {}
class C2 {}

trait T {
  private function f(string $x): void {}
}

class C {
  use T;

  // ok to "override" a trait private function with a visibility expanding
  // *consistent* implementation
  protected function f(string $x): void {}
}
