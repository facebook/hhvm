<?hh // strict

// ERROR, missing <<__OnlyRxIfArgs>>
<<__Rx>>
function f(<<__MaybeRx>>(function(): void) $a): void {
}
