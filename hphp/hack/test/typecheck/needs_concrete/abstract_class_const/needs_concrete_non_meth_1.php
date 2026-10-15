<?hh


<<__ConsistentConstruct>>
abstract class A {
  abstract const int X;
  public static abstract function abs(): void;

  public static function error(): void {
    static::X; // error
    static::abs<>; // error
    new static(); // error
  }
}
