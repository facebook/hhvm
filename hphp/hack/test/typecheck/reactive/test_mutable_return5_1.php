<?hh // strict
<<file: __EnableUnstableFeatures('coeffects_provisional')>>

class C {}

<<__Rx, __MutableReturn>>
function make(): C {
  // OK
  return new C();
}
