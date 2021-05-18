<?hh // strict

class A {
  public function f(): void {}
}

function cond(): bool {
  return false;
}

function test(): void {
  $x = null;
  if (cond()) {
    $x = new A();
  }

  if ($x !== null) {
    $x?->f();
  }
}
