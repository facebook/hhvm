<?hh // strict
<<file: __EnableUnstableFeatures('coeffects_provisional')>>

// ERROR
<<__AtMostRxAsArgs>>
function f((function(): int) $s): void {
}
