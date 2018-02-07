<?hh // strict

class A {}
class B {}

function f(bool $x): nonnull {
  if ($x) {
    return new A();
  } else {
    return new B();
  }
}

function g(int $i): nonnull {
  $v = Vector {};
  $v[0] = new A();
  $v[1] = new B();
  return $v[$i];
}
