<?hh

abstract class AbstractClass {
  public static abstract function abstractMethod(): void;
  public static function concreteMethod(): void {}
}

class ConcreteClass extends AbstractClass {
  public static function abstractMethod(): void {}
  public static function concreteMethod(): void {}
}


<<__EntryPoint>>
function main(): void {
  $abstract_cls = AbstractClass::class;
  $abstract_cls::abstractMethod(); // error
  $abstract_cls::concreteMethod(); // ok

  $concrete_class = ConcreteClass::class;
  $concrete_class::abstractMethod(); // ok
  $concrete_class::concreteMethod(); // ok
}
