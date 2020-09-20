<?hh // partial

class C {
  public static function test(mixed $x): void {
    if ($x is this) {
      static::expectThis($x);
      self::expectThis($x);
    } else {
      static::expectThis($x);
      self::expectThis($x);
    }
  }

  protected static function expectThis(this $x): void {}
}
