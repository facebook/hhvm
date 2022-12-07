<?hh
// (c) Meta Platforms, Inc. and affiliates.

abstract class BaseClass {
  public int $x = 0;
  public static function func(): int {
    return 0;
  }
  protected function lol(): void {}
}

trait TTrait {
  protected static int $x = 0;
  protected function func(): int {
    return 0;
  }
  public static function lol(): void {}
}

class SubClass extends BaseClass {
  use TTrait;
}
