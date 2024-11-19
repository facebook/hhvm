<?hh

class C {
  public function foo(): bool {
    return true;
  }
}

function bar(C $c): int {
  $x = $c->foo();
  return $x;
}
