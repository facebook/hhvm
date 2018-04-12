<?hh // strict

// ERROR, __MaybeRx can only appear on parameters of
// conditionally reactive functions
function f(<<__MaybeRx>>(function(): void) $a): void {
}
