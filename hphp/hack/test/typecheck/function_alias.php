<?hh // strict

type Predicate<T> = (function(T): bool);

<<__Rx, __OnlyRxIfArgs>>
function f(<<__OnlyRxIfRxFunc>>Predicate<int> $f): void {
}
