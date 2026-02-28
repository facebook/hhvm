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

  $foo = Foo::class as dynamic;

  $baz = 'bar';

  // invoke prop $baz
  hh_show((Foo::$baz)());
  hh_show((Foo::{$baz})());
}
