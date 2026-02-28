<?hh

class C {
  public bool $foo = true;
}

function bar(C $c): int {
  $x = $c->foo;
  return $x;
}
