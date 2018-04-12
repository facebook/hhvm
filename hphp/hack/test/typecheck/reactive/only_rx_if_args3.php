<?hh // strict

// OK
<<__Rx, __OnlyRxIfArgs>>
function f(<<__MaybeRx>>(function(): int) $s): void {
}
