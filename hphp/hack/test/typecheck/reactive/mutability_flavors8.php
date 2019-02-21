<?hh // partial

class A {
  <<__Rx>>
  public function f(): void {
  }
}

<<__Rx>>
function f(bool $x): void {
  if ($x) {
    // OK
    $a = \HH\Rx\mutable(new A());
  }
}
