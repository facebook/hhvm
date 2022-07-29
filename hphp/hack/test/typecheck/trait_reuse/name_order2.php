<?hh
// (c) Meta Platforms, Inc. and affiliates. Confidential and proprietary.

interface I {
  protected function foo(): void;
}

trait T {
  protected function foo(): void {}
}

trait T1 {
  use T;
}

final class C implements I  {
  use T;
  use T1;
}
