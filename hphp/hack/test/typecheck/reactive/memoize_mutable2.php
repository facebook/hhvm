<?hh // partial

class A {}

<<__Rx, __Memoize, __MutableReturn>>
function f(): A {
  return new A();
}
