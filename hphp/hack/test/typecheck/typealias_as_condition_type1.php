<?hh // strict

<<__Rx, __OnlyRxIfArgs>>
function test<T>(<<__OnlyRxIfRxFunc>>Predicate<int> $predicate): void {
  // OK
  $predicate(0);
}

type Predicate<T> = (function(T): bool);
