<?hh

class Foo {
  public static function doStuff(int $x): void {}

  public static function callIt(): void {
    static::doStuff(42);
    //              ^ hover-at-caret
  }
}
