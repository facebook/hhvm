<?hh // strict
// ERROR

function f<T as (function(): int)>(<<__AtMostRxAsFunc>>T $a, T $b): void {
}
