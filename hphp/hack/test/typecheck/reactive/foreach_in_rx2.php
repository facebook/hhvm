<?hh // strict
<<file: __EnableUnstableFeatures('coeffects_provisional')>>

<<__Rx>>
function f1(Traversable<int> $a): void {
  // ERROR: non rx-traversable
  foreach ($a as $c) {
  }
}
