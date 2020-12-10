<?hh
<<file: __EnableUnstableFeatures('coeffects_provisional')>>

class A {}

<<__Rx>>
function f(<<__OwnedMutable>> A $a): void {
}

<<__Rx, __MutableReturn>>
function getA(): A {
  return new A();
}

<<__Rx>>
function g(): void {
  // OK
  f(\HH\Rx\mutable(new A()));
  f(\HH\Rx\mutable(getA()));
}
