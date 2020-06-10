<?hh // strict

class C {}
class D extends C {}

function launder(): bool {
  return false;
}

function f(inout D $s): void {
  if (launder()) {
    $s = new C();
  } else {
    $s = new D();
  }
}
