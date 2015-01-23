<?hh

trait X {
  public function f(): bool {
    return true;
  }
}

function test($x): bool {
  return $x instanceof X;
}
