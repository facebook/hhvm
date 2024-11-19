<?hh

class C {
  public static function foo(): bool {
    return true;
  }
}

function bar(): int {
  $x = C::foo();
  return $x;
}
