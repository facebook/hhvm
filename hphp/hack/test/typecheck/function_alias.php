<?hh

type Predicate<T> = (function(T): bool);

function f(Predicate<int> $f): void {
}
