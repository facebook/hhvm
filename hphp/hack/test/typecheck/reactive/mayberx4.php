<?hh // strict
// ERROR, missing <<__AtMostRxAsArgs>>

function f(<<__AtMostRxAsFunc>>(function(): void) $a): void {
}
