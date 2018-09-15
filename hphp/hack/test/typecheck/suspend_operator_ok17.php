<?hh // strict

class A {
  public coroutine function f(): int {
    return 1;
  }
}

coroutine function f(A $a): int {
  $m = meth_caller("A", "f");
  // OK - calling coroutine typed method via meth_caller
  $v = suspend $m($a);
  return $v + 1;
}
