<?hh

abstract class Foo {
abstract public static function who():mixed;
}

<<__EntryPoint>> function main(): void {
  Foo::who();
}
