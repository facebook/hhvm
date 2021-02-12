<?hh // strict
class A {
}


function g(): A {
  return new A();
}

function f(bool $x): void {
  // ERROR
  $b = \HH\Rx\mutable(g());
}
