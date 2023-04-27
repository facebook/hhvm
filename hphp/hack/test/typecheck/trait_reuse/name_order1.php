<?hh
// (c) Meta Platforms, Inc. and affiliates. Confidential and proprietary.

interface I {
  protected function foo(): void;
}

trait E {
  protected function foo(): void {}
}

trait E1 {
  use E;
}

final class C implements I  {
  use E;
  use E1;
}
