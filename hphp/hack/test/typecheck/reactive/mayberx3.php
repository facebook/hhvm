<?hh // strict

// ERROR, __OnlyRxIfRxFunc can only appear on parameters of
// conditionally reactive functions
function f(<<__OnlyRxIfRxFunc>>(function(): void) $a): void {
}
