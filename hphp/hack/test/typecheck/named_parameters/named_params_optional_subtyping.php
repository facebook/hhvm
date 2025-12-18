<?hh

function my_fun((function(int, optional named bool $b): void) $f): (function(int): void) {
  return $f; // OK
}

class C1 {
  public function m(int $_): void {}
}

class C2 extends C1 {
  public function m(int $_, named bool $b = true): void {} // OK
}
