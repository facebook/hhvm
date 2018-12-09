<?hh // strict

// ERROR
<<__AtMostRxAsArgs>>
function f((function(): int) $s): void {
}
