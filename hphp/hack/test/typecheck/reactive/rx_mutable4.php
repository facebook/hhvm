<?hh // strict
<<file: __EnableUnstableFeatures('coeffects_provisional')>>

class A {
}

<<__Rx, __MutableReturn>>
function g(): A {
  return new A();
}

function f(bool $x): void {
  // ERROR
  $b = \HH\Rx\mutable(g());
}
