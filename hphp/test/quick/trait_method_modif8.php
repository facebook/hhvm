<?hh
class Foo {
  function f() :mixed{
    echo "Foo\n";
  }
}
trait T {
  static function f() :mixed{
    echo "Bar\n";
  }
}
class Bar extends Foo {
  use T;
}
<<__EntryPoint>> function main(): void {
Bar::f();
}
