<?hh

<<__ConsistentConstruct>>
abstract class AbstractClass {
  public static abstract function abstractMethod(): void;
  public static function concreteMethod(): void {}
}

abstract class AbstractClass2 extends AbstractClass {
  public static function concreteMethod(): void {
    new static();
  }
}
