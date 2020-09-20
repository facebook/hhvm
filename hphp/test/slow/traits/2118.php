<?hh

trait T {
  abstract static function f();
}
abstract class Foo {
  abstract static function f();
}
class Bar extends Foo {
  use T;
  static function f() {
    echo "Foo\n";
  }
}

<<__EntryPoint>>
function main_2118() {
Bar::f();
}
