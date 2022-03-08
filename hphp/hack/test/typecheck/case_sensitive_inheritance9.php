<?hh
// (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

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
