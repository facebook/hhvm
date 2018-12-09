<?hh // strict

<<__Rx, __AtMostRxAsArgs>>
function test<T>(<<__AtMostRxAsFunc>>Predicate<int> $predicate): void {
  // OK
  $predicate(0);
}

type Predicate<T> = (function(T): bool);
