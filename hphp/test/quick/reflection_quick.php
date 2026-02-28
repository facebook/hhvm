<?hh
class Foo {

  private static $barX;
  public static function bar() :mixed{
  }
}
<<__EntryPoint>> function main(): void {
var_dump((new ReflectionClass('Foo'))->isInstantiable());
}
