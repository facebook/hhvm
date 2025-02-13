<?hh

<<__ConsistentConstruct>>
abstract class AbstractClass {
  public static abstract function abstractMethod(): void;
  public static function concreteMethod(): void {}
  public static function example(): void {
    static::abstractMethod(); // error
    static::concreteMethod(); // ok
  }
}
