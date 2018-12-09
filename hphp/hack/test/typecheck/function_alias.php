<?hh // strict

type Predicate<T> = (function(T): bool);

<<__Rx, __AtMostRxAsArgs>>
function f(<<__AtMostRxAsFunc>>Predicate<int> $f): void {
}
