<?hh // strict

// OK
<<__Rx, __OnlyRxIfArgs>>
function f(<<__OnlyRxIfRxFunc>>(function(): int) $s): void {
}
