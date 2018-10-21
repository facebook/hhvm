<?hh // strict

class A {
}

<<__Rx, __MutableReturn>>
function g(): A {
  return new A();
}

<<__Rx>>
function f(bool $x): void {
  // OK
  $b = \HH\Rx\mutable(g());
  // OK
  $b = \HH\Rx\mutable(new A());
}
