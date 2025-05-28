<?hh

class Foo {

  static function bar() :mixed{
    var_dump(10);
  }


  static function rbar<reify T>(T $x) :mixed{
    var_dump($x);
  }

  <<__DynamicallyCallable>>
  static function baz() :mixed{
    var_dump(20);
  }
}

<<__EntryPoint>>
function main() :mixed{
  $x = Foo::class;
  $x::bar(); // no warning
  $x::rbar<int>(10); // no warning
  $x = 'Foo';
  $x::bar(); // warning
  $x::rbar<int>(10); // warning
  $s = 'bar';
  HH\dynamic_class_meth(Foo::class, $s)(); // warning
  $s = 'baz';
  HH\dynamic_class_meth(Foo::class, $s)(); // no warning

}
