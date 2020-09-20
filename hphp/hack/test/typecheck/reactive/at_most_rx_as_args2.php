<?hh // strict

// ERROR
<<__Rx, __AtMostRxAsArgs>>
function f((function(): int) $s): void {
}
