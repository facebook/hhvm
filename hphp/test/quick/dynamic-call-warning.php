<?hh

class Foo {

  static function bar() {
    var_dump(10);
  }


  static function rbar<reify T>(T $x) {
    var_dump($x);
  }

  <<__DynamicallyCallable>>
  static function baz() {
    var_dump(20);
  }
}

<<__EntryPoint>>
function main() {
  $x = Foo::class;
  $x::bar(); // no warning
  $x::rbar<int>(10); // no warning
  $x = 'Foo';
  $x::bar(); // warning
  $x::rbar<int>(10); // warning
  $s = 'bar';
  Foo::$s(); // warning
  $s = 'baz';
  Foo::$s(); // no warning

}
