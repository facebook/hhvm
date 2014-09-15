<?hh // strict

class X {
  private static int $prop = 10;

  public function test(): int {
    return static::$prop;
  }
}
