<?hh
class Foo {
  static function f() :mixed{
    echo "Foo\n";
  }
}
trait T {
  function f() :mixed{
    echo "Bar\n";
  }
}
class Bar extends Foo {
  use T;
}
<<__EntryPoint>> function main(): void {
Bar::f();
}
