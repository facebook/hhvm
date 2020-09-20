<?hh // strict

// ERROR
<<__Rx>>
function f<T as (function(): int)>(<<__AtMostRxAsFunc>>T $a, T $b): void {
}
