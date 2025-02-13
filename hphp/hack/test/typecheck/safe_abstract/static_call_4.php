<?hh

abstract class A {
  public function example(): void {
    // in a nonstatic function, we know `static` is concrete
    static::abs(); // ok
  }
  public static abstract function abs(): void;

}
