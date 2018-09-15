<?hh // strict

class C {
  public function foo(): string { return 's'; }
}

class D {
  public function foo(): int { return 1; }
}

function test(): void {
  $f = $x ==> {
    $y = $x->foo();
    return $y;
  };
  $f(new C());
  $f(new D());
}
