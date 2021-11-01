<?hh

trait X {
  public function f(): bool {
    return true;
  }
}

function test(mixed $x): bool {
  return $x is X;
}
