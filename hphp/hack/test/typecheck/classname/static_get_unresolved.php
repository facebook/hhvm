<?hh // strict

class Foo {
  public static function f(): arraykey {
    // UNSAFE_BLOCK
  }
}
class Bar extends Foo {
  public static function f(): int {
    // UNSAFE_BLOCK
  }
}
class Baz extends Foo {
  public static function f(): string {
    // UNSAFE_BLOCK
  }
}

function f(mixed $x): arraykey {
  $classes = Vector {Bar::class, Baz::class};
  if ($x instanceof $classes[0]) {
    $ret = $x::f();
    hh_show($ret);
    return $ret;
  }
  invariant_violation('');
}
