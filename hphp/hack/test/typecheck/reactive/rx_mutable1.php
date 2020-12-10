<?hh // strict
<<file: __EnableUnstableFeatures('coeffects_provisional')>>

<<__Rx>>
function f(int $a): void {
  // ERROR
  $b = \HH\Rx\mutable($a);
}
