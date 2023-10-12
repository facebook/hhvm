<?hh //strict

class A {
  public int $x = 0;
}

function f(): void {}

function g(): void {
  $a = null;
  try {
    f();
    $a = new A();
  } finally {
    $a?->x; // no error here
  }
}
