<?hh // partial

class A {}

<<__Rx, __MutableReturn>>
function g(): A {
  return new A();
}

<<__Rx>>
function f(): void {
  // ERROR
  $a = g() ?? \HH\Rx\mutable(new A());
}
