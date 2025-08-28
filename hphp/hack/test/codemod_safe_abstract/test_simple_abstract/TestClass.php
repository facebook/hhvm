<?hh

abstract class TestClass {
  public static abstract function testMethod(): void;

  final public static function caller(): void {
    static::testMethod();
  }
}
