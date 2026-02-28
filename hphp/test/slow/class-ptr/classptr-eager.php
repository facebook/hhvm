<?hh

class Foo {
  public int $x = 42;
  public static int $y = 100;
  const A = 10;

  public static function cfun() :mixed{
    var_dump('hello1\n');
  }
}

class Bar {
  public int $x = 24;
}

class Baz {
  public static @HH\classname $f = Foo::class;
}

type T = null;
type N = HH\classname;

function foo(@HH\classname $x) : @N {
  var_dump($x);
  return Bar::class;
}

<<__EntryPoint>>
  function main() :mixed{
  $ca = vec[Foo::class, Bar::class];
  var_dump($ca);
  $c = $ca[0];
  var_dump($c::$y);
  var_dump($c::A);
  $o = new $ca[1];
  var_dump($o->x);
  var_dump(foo(Foo::class));
  Baz::$f = Bar::class;
  var_dump(Baz::$f);
  $t = T::class; // T is not a class
  var_dump($t);
}
