<?hh // strict

// ERROR
<<__Rx>>
function f<T as (function(): int)>(<<__MaybeRx>>T $a, T $b): void {
}
