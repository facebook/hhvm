<?hh // strict
<<file: __EnableUnstableFeatures('coeffects_provisional')>>

// ERROR, missing <<__AtMostRxAsArgs>>
<<__Rx>>
function f(<<__AtMostRxAsFunc>>(function(): void) $a): void {
}
