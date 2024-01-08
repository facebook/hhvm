<?hh

module MLT_A;

internal trait T {
  internal int $myprop = 42;

  internal function foo(): void {
    echo "I am foo in T\n";
  }

  internal static function bar(): void {
    echo "I am bar in T\n";
  }
}

class C {
  use T;
}

internal class D {
  use T;
}

<<__EntryPoint>>
function bar(): void {
  include 'module_level_traits_module_a.inc';

  $c = new C();
  $c->foo();
  C::bar();
  echo "myprop in C is ".$c->myprop."\n";

  $d = new D();
  $d->foo();
  D::bar();
  echo "myprop in D is ".$d->myprop."\n";
}
