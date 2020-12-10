<?hh // strict
<<file: __EnableUnstableFeatures('coeffects_provisional')>>

interface I {}

<<__Rx>>
function f(<<__MaybeMutable>>I $i): void {
  $j = $i;
}
