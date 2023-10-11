<?hh

final class X {
  private static int $prop = 10;

  private static function meth(): void {}

  public function test(): int {
    static::meth();
    return static::$prop;
  }
}
