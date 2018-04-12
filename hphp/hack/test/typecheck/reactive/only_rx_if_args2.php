<?hh // strict

// ERROR
<<__Rx, __OnlyRxIfArgs>>
function f((function(): int) $s): void {
}
