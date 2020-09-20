<?hh

abstract class Foo {
abstract public static function who();
}

<<__EntryPoint>> function main(): void {
  Foo::who();
}
