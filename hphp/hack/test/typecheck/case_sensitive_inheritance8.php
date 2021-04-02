<?hh
// (c) Facebook, Inc. and its affiliates. Confidential and proprietary.

trait MyTrait {

  final private function createBuildID(): string {
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
