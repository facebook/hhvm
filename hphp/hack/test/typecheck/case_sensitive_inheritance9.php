<?hh
// (c) Facebook, Inc. and its affiliates. Confidential and proprietary.

class A {
  public function FOO(): void {}
}
trait T {
  public static function foO(): void {}
  public function bar(): int {
    return 0;
  }
}
class B extends A {
  use T;
}
