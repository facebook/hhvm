<?hh // strict

// ERROR, missing <<__AtMostRxAsArgs>>
<<__Rx>>
function f(<<__AtMostRxAsFunc>>(function(): void) $a): void {
}
