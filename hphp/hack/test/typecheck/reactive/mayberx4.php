<?hh // strict

// ERROR, missing <<__OnlyRxIfArgs>>
<<__Rx>>
function f(<<__OnlyRxIfRxFunc>>(function(): void) $a): void {
}
