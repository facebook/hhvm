<?hh // strict

<<__Rx>>
function f1(Traversable<int> $a): void {
  // ERROR: non rx-traversable
  foreach ($a as $c) {
  }
}
