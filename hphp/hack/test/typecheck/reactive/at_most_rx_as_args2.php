<?hh // strict
<<file: __EnableUnstableFeatures('coeffects_provisional')>>

// ERROR
<<__Rx, __AtMostRxAsArgs>>
function f((function(): int) $s): void {
}
