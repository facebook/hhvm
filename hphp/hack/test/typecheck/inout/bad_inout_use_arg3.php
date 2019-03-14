<?hh // partial

function f(inout int $i): void {}

function launder(mixed $x) {
  return $x;
}

class C {
  public int $x = 42;
}

function test(): void {
  $c = launder(new C());
  f(inout $c->x);
}
