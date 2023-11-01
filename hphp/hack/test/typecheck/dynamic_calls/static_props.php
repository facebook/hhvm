<?hh

class Foo {
  const int bar = 42;
  public static string $bar = 'bar';
  public static (function(): float) $baz = Foo::g<>;
  public static function bar(): bool {
    return true;
  }
  private static function g(): float {
    return 3.14159;
  }
}

function h(): void {
  // const
  hh_show(Foo::{"bar"});

  // prop $bar
  hh_show(Foo::$bar);
  hh_show(Foo::{$bar});
  hh_show(Foo::{'ba'.'r'});

  $foo = Foo::class as dynamic;

  // $baz undefined
  hh_show($foo::{$baz}());

  $baz = 'bar';

  // invoke prop $baz
  hh_show((Foo::$baz)());
  hh_show((Foo::{$baz})());

  // invoke static meth named in $baz
  hh_show($foo::{$baz}());
}
