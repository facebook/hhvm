<?hh
class Foo {

  private static $barX;
  public static function bar() {
  }
}
var_dump((new ReflectionClass('Foo'))->isInstantiable());
