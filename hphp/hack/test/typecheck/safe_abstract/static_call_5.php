<?hh

abstract class A {
  public function __construct(): void {
    // in a constructor, we know `static` is concrete
    static::abs(); // ok
  }
  public static abstract function abs(): void;

}
