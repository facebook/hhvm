<?hh

<<__ConsistentConstruct>>
abstract class AbstractClass {
  public static abstract function abstractMethod(): void;
  public static function concreteMethod(): void {}
  public function example(): void {
    self::abstractMethod(); // should be only one error (no duplicate warning here)
  }
}
