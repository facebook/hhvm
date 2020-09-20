<?hh // strict

class A {
}

<<__Rx>>
function g(): A {
  return new A();
}

<<__Rx>>
function f(): void {
  // ERROR
  $b = \HH\Rx\mutable(g());
}
