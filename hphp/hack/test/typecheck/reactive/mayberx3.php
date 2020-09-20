<?hh // strict

// ERROR, __AtMostRxAsFunc can only appear on parameters of
// conditionally reactive functions
function f(<<__AtMostRxAsFunc>>(function(): void) $a): void {
}
