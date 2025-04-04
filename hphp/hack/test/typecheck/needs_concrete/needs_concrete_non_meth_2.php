<?hh


<<__ConsistentConstruct>>
abstract class A {
  abstract const int X;
  public static abstract function abs(): void;

  <<__NeedsConcrete>>
  public static function ok(): void {
    static::X; // ok
    static::abs<>; // ok
    new static(); // ok
  }
}
