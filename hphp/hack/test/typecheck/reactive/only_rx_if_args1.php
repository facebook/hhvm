<?hh // strict

// ERROR
<<__OnlyRxIfArgs>>
function f((function(): int) $s): void {
}
