<?hh // strict

interface I {}

<<__Rx>>
function f(<<__MaybeMutable>>I $i): void {
  $j = $i;
}
