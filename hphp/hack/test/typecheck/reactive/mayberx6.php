<?hh // strict

// ERROR
<<__Rx>>
function f<T as (function(): int)>(<<__OnlyRxIfRxFunc>>T $a, T $b): void {
}
