<?hh
trait T {
  abstract static function f():mixed;
}
abstract class Base {
  use T;
}
abstract class Foo extends Base {
  abstract static function f():mixed;
}
class Bar extends Foo {
  static function f() :mixed{
    echo "Foo\n";
  }
}
<<__EntryPoint>> function main(): void {
Bar::f();
}
