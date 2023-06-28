<?hh

final class Foo {
  public static function doTestClassName(
    classname $foo
  ) :mixed{
    var_dump($foo);
    var_dump($foo == Foo::class);
    var_dump($foo == "Foo");
  }

  public static function doTestString(
    string $foo
  ) :mixed{
    var_dump($foo);
    var_dump($foo == Foo::class);
    var_dump($foo == "Foo");
  }
}

<<__EntryPoint>>
function main() :mixed{
  $foo = Foo::class;
  var_dump($foo);
  var_dump($foo == Foo::class);
  var_dump($foo == "Foo");
  var_dump(Foo::class == "Foo");
  Foo::doTestClassName(Foo::class);
  Foo::doTestString(Foo::class);

  $fn = $foo ==> {
    var_dump($foo);
    var_dump($foo == Foo::class);
    var_dump($foo == "Foo");
  };
  $fn(Foo::class);

  $fn = (classname $foo) ==> {
    var_dump($foo);
    var_dump($foo == Foo::class);
    var_dump($foo == "Foo");
  };
  $fn(Foo::class);

  $fn = (string $foo) ==> {
    var_dump($foo);
    var_dump($foo == Foo::class);
    var_dump($foo == "Foo");
  };
  $fn(Foo::class);
}
