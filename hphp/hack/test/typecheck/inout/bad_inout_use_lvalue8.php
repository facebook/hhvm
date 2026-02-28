<?hh

function f(inout int $i): void {}

function launder(): bool {
  return false;
}

class C {
  public int $x = 42;
}

function test(): void {
  $c = launder() ? new C() : null;
  f(inout $c?->x);
}
