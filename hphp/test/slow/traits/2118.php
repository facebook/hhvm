<?hh

trait T {
  abstract static function f():mixed;
}
abstract class Foo {
  abstract static function f():mixed;
}
class Bar extends Foo {
  use T;
  static function f() :mixed{
    echo "Foo\n";
  }
}

<<__EntryPoint>>
function main_2118() :mixed{
Bar::f();
}
