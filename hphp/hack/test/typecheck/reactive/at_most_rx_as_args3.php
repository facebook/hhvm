<?hh // strict
<<file: __EnableUnstableFeatures('coeffects_provisional')>>

// OK
<<__Rx, __AtMostRxAsArgs>>
function f(<<__AtMostRxAsFunc>>(function(): int) $s): void {
}
