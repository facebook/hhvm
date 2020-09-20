<?hh // strict

<<__Rx>>
function f1<T as Traversable<int>>(T $a): void {
  // ERROR: non rx-traversable
  foreach ($a as $c) {
  }
}
