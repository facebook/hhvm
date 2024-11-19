<?hh

class C {
  public static bool $foo = true;
}

function bar(C $c): int {
  $x = C::$foo;
  return $x;
}
