<?hh // strict
<<file: __EnableUnstableFeatures('coeffects_provisional')>>

class C {}

<<__Rx, __MutableReturn>>
function make(C $c): C {
  // not OK - returns parameter
  return $c;
}
