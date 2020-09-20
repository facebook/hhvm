<?hh // strict

class C {}

<<__Rx, __MutableReturn>>
function make(): C {
  // OK
  return new C();
}
