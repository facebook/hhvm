<?hh
// (c) Facebook, Inc. and its affiliates.
//
// This source code is licensed under the MIT license found in the
// LICENSE file in the "hack" directory of this source tree.

trait MyTrait {

  private function createBuildID(): string {
    return "foo";
  }

  final public function foo(): string {
    return $this->createBuildID();
  }
}

final class MyA {
  use MyTrait;

  private static function createBuildId(): string {
    return "FOO";
  }
}
