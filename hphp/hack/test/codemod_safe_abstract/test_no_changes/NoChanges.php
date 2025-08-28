<?hh

abstract class NoChanges {
  public static abstract function testMethod(): void;

  <<__NeedsConcrete>>
  final public static function caller(): void {
    static::testMethod();
  }
}
